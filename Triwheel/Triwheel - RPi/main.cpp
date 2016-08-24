// Compile with -lpthread -lwiringPi
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <wiringPi.h>
#include <wiringSerial.h>
#include <stdlib.h>
#include "pidController.h"
#include "controlmath.h"
#include "minimu9.h"
#include "ps2USB.h"
#include "timerInterrupt.h"
#include "RPS.h"

#define powerOffButton 4
#define headingRefButton 0
#define ps2InputLED 25
#define headingLED 29
#define slowLoopLED 27
#define miscLED 28
#define maxRPM 200
#define maxVELOCITY 132
#define maxCHECKPOINT 5
#define HEADING_TOL 1
#define POSITION_TOL 1

#define PLOT 1

#if PLOT == 1
	FILE *fp,*datum_fp;
	long fileLine = 0;
#endif

bool ps2Ready = false;
bool imuReady = false;
bool powerOffPressed = false;

float desiredHeading = 0.0;
float headingCorrection = 0;
float headingError = 0;
float positionError_x = 0;
float positionError_y = 0;
int rpiPort;

float checkPoint[maxCHECKPOINT][2] = { 	{000.0,160.0},
					{90.0,160.0},
					{175.0,140.0},
					{260.0,110.0},
					{339.0,56.0} };
int index = 0;

struct point zero = {0.0,0.0}, destination = {0.0,0.0}, init = zero;
struct omniDriveState stopState;
enum {headingControl, positionControl};

void powerOff() {
	if(powerOffPressed) return;
	powerOffPressed = true;
	printf("powerOff\n");
	stopIMU();
	stopPS2();
	stopTimer();
	for(int i = 1; i <=3; i++) {
		digitalWrite(ps2InputLED, HIGH);	
		digitalWrite(headingLED, HIGH);	
		digitalWrite(slowLoopLED, HIGH);
		digitalWrite(miscLED, HIGH);
		sleep(1);
		digitalWrite(ps2InputLED, LOW);	
		digitalWrite(headingLED, LOW);	
		digitalWrite(slowLoopLED, LOW);
		digitalWrite(miscLED, LOW);
		sleep(1);
	}
	system("shutdown -h now");
}

void reset() {
	resetRefHeading();
	resetPIDvar(headingControl);
	resetPIDvar(positionControl);
	destination.x = 0;
	destination.y = 0;
	headingCorrection = 0;
	setCurrentPosition(zero);
}

void ps2Activated() {
	printf("ps2 Activated...\n");
	ps2Ready = true;
	digitalWrite(ps2InputLED, HIGH);
}

void ps2Deactivated() {
	printf("PS2 Deactivated...\n");
	ps2Ready = false;
	digitalWrite(ps2InputLED, LOW);
}

void imuActivated() {
	printf("IMU Activated...\n");
	imuReady = true;
	digitalWrite(headingLED, HIGH);
}

void imuDeactivated() {
	printf("IMU Deactivated...\n");
	imuReady = false;
	digitalWrite(headingLED, LOW);
}

void slowTimerHandler() {
	printf("slowLoop\n");
	digitalWrite(slowLoopLED, !digitalRead(slowLoopLED));
}

struct uniCycleState limiter(struct uniCycleState state) {
	if(state.vx > maxVELOCITY) {
            printf("Warning: Trying to send Velocity greater than %f. Limit : %f. Sending %f.\r\n",maxVELOCITY);
            state.vx = maxVELOCITY; //must be even
	} else if(state.vx < -(maxVELOCITY+1)) {
	    printf("Warning: Trying to send Velocity lesser than %f. Limit : %f. Sending %f.\r\n",-(maxVELOCITY+1));
	    state.vx = -(maxVELOCITY+1); // must be odd
	}
	if(state.vy > maxVELOCITY) {
            printf("Warning: Trying to send Velocity greater than %f. Limit : %f. Sending %f.\r\n",maxVELOCITY);
            state.vy = maxVELOCITY;
	} else if(state.vx < -(maxVELOCITY+1)) {
	    printf("Warning: Trying to send Velocity lesser than %f. Limit : %f. Sending %f.\r\n",-(maxVELOCITY+1));
	    state.vy = -(maxVELOCITY+1);
	}
	if(state.w > maxVELOCITY) {
            printf("Warning: Trying to send Velocity greater than %f. Limit : %f. Sending %f.\r\n",maxVELOCITY);
            state.w = maxVELOCITY;
	} else if(state.w < -(maxVELOCITY+1)) {
	    printf("Warning: Trying to send Velocity lesser than %f. Limit : %f. Sending %f.\r\n",-(maxVELOCITY+1));
	    state.w = -(maxVELOCITY+1);
	}
	return state;
}

struct uniCycleState traceLine(struct point curPos, float heading, struct point destinationPoint, struct point linePoint, float desiredHeading_) {
    float lineTheta = radianToDegree(atan2((destinationPoint.y - linePoint.y), (destinationPoint.x - linePoint.x)));
    struct point curPos_ = rotationalTransform(curPos, lineTheta);

    struct point destinationPoint_ = rotationalTransform(destinationPoint, lineTheta);
    struct point translationalState_;
    
    positionError_x = destinationPoint_.x - curPos_.x;
    if(abs(positionError_x) > POSITION_TOL) {
        translationalState_.x = maxVELOCITY * sigmoid(0.1*positionError_x);
    } else {
    	translationalState_.x = 0;
    }    
    positionError_y = destinationPoint_.y - curPos_.y;
    if(abs(positionError_y) > POSITION_TOL) {
        translationalState_.y = PID(positionError_y, positionControl);
    } else {
    	translationalState_.y = 0;
    }    
    headingError = desiredHeading_ - heading;
    if(abs(headingError) > HEADING_TOL) {
    	headingCorrection = PID(headingError, headingControl);
    } else {
	headingCorrection = 0;
    }

    struct point translationalState = rotationalTransform(translationalState_, -lineTheta);
    struct uniCycleState state;
    state.vx = translationalState.x;
    state.vy = translationalState.y;
    state.w = headingCorrection;
    return state;
}

struct uniCycleState getDesiredUniCycleState() {
	struct uniCycleState desiredUniCycleState;
	headingError = desiredHeading - getHeading();
	if(abs(headingError) > HEADING_TOL) {
		headingCorrection = PID(headingError, headingControl);
	} else {
        	headingCorrection = 0;
	}
	desiredUniCycleState.w = headingCorrection;
	desiredUniCycleState.vy = (128 - ps2_getY()) * maxVELOCITY / 128;
	desiredUniCycleState.vx = (ps2_getX() - 128) * maxVELOCITY / 128;
	desiredUniCycleState = limiter(desiredUniCycleState);
	return desiredUniCycleState;
}

char encodeByte(int rpm) {
    if(rpm > 180) {
        printf("Warning: Trying to send Velocity greater than 180. Limit : 180. Sending 180.\r\n");
        rpm = 180; //must be even
    } else if(rpm < -181) {
        printf("Warning: Trying to send Velocity lesser than -181. Limit : -181. Sending -181.\r\n");
        rpm = -181; // must be odd
    }
    if(rpm < 0) {
        rpm = -rpm;
        return (rpm | 0x01);
    } else {
        rpm = (rpm & 0xFE);
        return rpm == 0x0A ? 0x0C : rpm;
    }
}

void transmitOmniState(struct omniDriveState state) {
    printf("%d %d %d %f %f \r\n", state.aRPM, state.bRPM, state.cRPM, getCurrentPosition().x, getCurrentPosition().y);
#if PLOT == 1
    fprintf(fp,"%f %f\n",getCurrentPosition().x, getCurrentPosition().y);
    fileLine++;
#endif
    serialPutchar(rpiPort, 0x0A);
    serialPutchar(rpiPort, encodeByte(state.cRPM));
    serialPutchar(rpiPort, encodeByte(state.aRPM));
    serialPutchar(rpiPort, encodeByte(state.bRPM));
}

void timerHandler() {
	if(!ps2Ready || !imuReady) {
		transmitOmniState(stopState);
	} else {
//		transmitOmniState(transformUniToOmni(getDesiredUniCycleState(), 0));
		transmitOmniState(transformUniToOmni(traceLine(getCurrentPosition(), getHeading(), destination, init, 0), 0));
		digitalWrite(miscLED, !digitalRead(miscLED));
	}
}

void CircleButtonPressed() {
	index++;
	if(index > (maxCHECKPOINT - 1)) {
		index = maxCHECKPOINT - 1;
	}
}

void CircleButtonReleased() {
}

void TriangleButtonPressed() {
	index--;
	if(index < 0) {
		index = 0;
	}
}

void TriangleButtonReleased() {
}

int main(){
#if PLOT == 1
	fp = fopen("../path.dat","w");
	datum_fp = fopen("../datum.dat","w");
	fprintf(datum_fp,"0 0\n%f %f",destination.x,destination.y);
	fclose(datum_fp);
#endif
	rpiPort = serialOpen("/dev/ttyS0",38400);
	stopState.aRPM = 0.0;
	stopState.bRPM = 0.0;
	stopState.cRPM = 0.0;

	struct encoder encoderX;
	encoderX.channelA_pin = 24;//23
	encoderX.channelB_pin = 23;//24
	encoderX. radius = 2.4;
	encoderX.PPR = 360;

	struct encoder encoderY;
	encoderY.channelA_pin = 26;//26
	encoderY.channelB_pin = 22;//22
	encoderY. radius = 2.4;
	encoderY.PPR = 360;	

	if(wiringPiSetup() < 0) {
		printf("Error setting up while setting wiringPi\n");
	}
	
	pinMode(powerOffButton, INPUT);
	pinMode(headingRefButton, INPUT);
	
	pinMode(ps2InputLED, OUTPUT);
	pinMode(headingLED, OUTPUT);
	pinMode(slowLoopLED, OUTPUT);
	pinMode(miscLED, OUTPUT);
	
	if(wiringPiISR(powerOffButton, INT_EDGE_RISING, &powerOff) < 0) {
		printf("Power Off Button interrupt setup error \n");
	}
	if(wiringPiISR(headingRefButton, INT_EDGE_RISING, &reset)) {
		printf("Reset Heading Buttpn interrupt setup error\n");
	}
	
	digitalWrite(ps2InputLED, LOW);	
	digitalWrite(headingLED, LOW);	
	digitalWrite(slowLoopLED, LOW);
	digitalWrite(miscLED, LOW);
	
	initPIDController(0.05, 0.0, 0.5, headingControl);
	initPIDController(5.0,0.0,0.0, positionControl);
	
	enablePS2StatusInterrupt(&ps2Activated, &ps2Deactivated);
	enableIMUStatusInterrupt(&imuActivated, &imuDeactivated);
	enableSlowFuncInterrupt(&slowTimerHandler);
	
	initPS2();
	enableCircleButton(&CircleButtonPressed, &CircleButtonReleased);
	enableTriangleButton(&TriangleButtonPressed, &TriangleButtonReleased);
	
	initIMU();
	initRPS(encoderX,encoderY,0.0,46.24);	
	
	initTimer(25000, &timerHandler);
	while(1) {
		init.x = checkPoint[index][0];
		init.y = checkPoint[index][1];
		destination.x = checkPoint[index][0];
		destination.y = checkPoint[index][1];

		sleep(1);
	}
}
