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
#include "LSA08.h"

#define powerOffButton 4
#define headingRefButton 0
#define ps2InputLED 25
#define headingLED 29
#define slowLoopLED 27
#define miscLED 28
#define maxRPM 200
#define maxVELOCITY 80
#define maxCHECKPOINT 6
#define HEADING_TOL 1
#define POSITION_TOL 3

#define PLOT 0

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
int backLineSensorError = 0, prev_backLineSensorError = 50;
int leftLineSensorError = 0, prev_leftLineSensorError = 50;

int rpiPort;
enum {headingControl, positionControl, backLineSensorControl, leftLineSensorControl};

struct point zero = {0.0, 0.0}, checkpoint, init = zero, destination = {0.0,0.0};
struct omniDriveState stopState;

struct lineSensor leftLineSensor, backLineSensor;
bool stageComplete = false, stageDetect = false;

int index = 0;
float heading_mem[maxCHECKPOINT] = {	90.0,
			 		0.0,
					-15.0,
					-30.0,
					-45.0,
					-30.0 };
float checkpoint_mem[maxCHECKPOINT][2] = { 	{-270.0,0.0},
			 			{-270.0,115.0},
						{-256,208},
						{-208,292},
						{-132,368},
						{-52,507} };
int lineSensorAdd_mem[maxCHECKPOINT] = { 	backLineSensor.address,
						backLineSensor.address,
						backLineSensor.address,
						backLineSensor.address,
						backLineSensor.address,
						backLineSensor.address		};
 
void leftJunctionDetect() {
}

void backJunctionDetect() {
}

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


void resetLineSensors(void) {
	backLineSensorError = 0;
	leftLineSensorError = 0;
	prev_backLineSensorError = 50;
    	prev_leftLineSensorError = 50;
}


void reset() {
	resetRefHeading();
	resetPIDvar(headingControl);
	resetPIDvar(positionControl);
	resetPIDvar(backLineSensorControl);
	resetPIDvar(leftLineSensorControl);
	resetLineSensors();
	destination.x = 0;
	destination.y = 0;
	headingCorrection = 0;
	index = 0;
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

void automationUpdate(void) {
	if(stageComplete == true) {
		index++;
		init = checkpoint;
		if(index > (maxCHECKPOINT - 1)) {
			index = maxCHECKPOINT - 1;
		}
	}
	desiredHeading = heading_mem[index];
	checkpoint.x = checkpoint_mem[index][0];
	checkpoint.y = checkpoint_mem[index][1];
	destination = checkpoint;
	//keep track of arena
}

void readLineSensors__(void) {
    backLineSensorError = readLineSensor(backLineSensor);
    if(backLineSensorError == 255) {
    	if(prev_backLineSensorError < 0) {
		backLineSensorError = -50;
	} else if(prev_backLineSensorError > 0) {
		backLineSensorError = 50;
	} else {
		backLineSensorError = 0;
	}
    }
    leftLineSensorError = readLineSensor(leftLineSensor);
    if(leftLineSensorError == 255) {
    	if(prev_leftLineSensorError < 0) {
		leftLineSensorError = -50;
	} else if(prev_leftLineSensorError > 0) {
		leftLineSensorError = 50;
	} else {
		leftLineSensorError = 0;
	}
    }
    prev_backLineSensorError = backLineSensorError;
    prev_leftLineSensorError = leftLineSensorError;
}

struct uniCycleState traceLine(struct point curPos, float heading, struct point destinationPoint, struct point linePoint, float desiredHeading_, int ls_pos, int ls_jun) {
    /* Calculate parameters of line to follow and rotate the X axis of coordinate system alonng line.*/
	float lineTheta = radianToDegree(atan2((destinationPoint.x - linePoint.x), (destinationPoint.y - linePoint.y)));
    struct point curPos_ = rotationalTransform(curPos, lineTheta);
    struct point initPos_ = rotationalTransform(linePoint, lineTheta);
    struct point destinationPoint_ = rotationalTransform(destinationPoint, lineTheta);
    float traversePathLength = fabs(destinationPoint_.x - initPos_.x);
    struct point translationalState_;
    
    /* Read line sensors and update variables as per given in parameters or do not update if specified*/
    int n = -1, lineSensorPosError, lineSensorJunError;
	readLineSensors__();
	switch(ls_pos) {
		case 0:
			lineSensorPosError = -666;
			break;
		case 1:
			lineSensorPosError = leftLineSensorError;
			n = leftLineSensorControl;
			break;
		case 2:
			lineSensorPosError = backLineSensorError;
			n = backLineSensorControl;
			break;	
	}
	switch(ls_jun) {
		case 0:
			lineSensorJunError = -666;
			break;
		case 1:
			lineSensorJunError = leftLineSensorError;
			break;
		case 2:
			lineSensorJunError = backLineSensorError;
			break;	
	}	
	
	/* Sigmoid along Y direction */
    positionError_y = destinationPoint_.y - curPos_.y;
    if(abs(positionError_y) > POSITION_TOL) {
		translationalState_.y = maxVELOCITY * sigmoid(0.05*positionError_y);
    } else {
		translationalState_.y = 0;
    }
	
	/* PID on line sensor error or X encoder error */
    positionError_x = destinationPoint_.x - curPos_.x;
    if(lineSensorPosError != -666) {
    	if(abs(lineSensorPosError) > POSITION_TOL) {
			translationalState_.x = PID(lineSensorPosError,n);
    	} else {
			translationalState_.x = 0;
	    }
	} else {
		if(abs(positionError_x) > POSITION_TOL) {
			translationalState_.x = PID(positionError_x,positionControl);
    	} else {
			translationalState_.x = 0;
	    }
	}
	
    headingError = desiredHeading_ - heading;
    
	if(abs(headingError) > HEADING_TOL) {
		headingCorrection = PID(headingError, headingControl);
    } else {
		headingCorrection = 0;
    }

/*
    if(fabs(lineSensorError_y) < 10 && fabs(positionError_x) < (0.5 * traversePathLength) && stageDetect == false) {
	setCurrentPosition(checkpoint);
	stageDetect = true;
	stageComplete = false;
    } else if(fabs(lineSensorError_y) > 36 && fabs(positionError_x) < (0.8 * traversePathLength) && stageComplete == true) {
	stageDetect = false;
    }
*/    
    if(translationalState_.x == 0 && translationalState_.y == 0 && headingCorrection == 0) {
	stageComplete = true;
    } else {
	stageComplete = false;
    }

    struct point translationalState = rotationalTransform(translationalState_, -lineTheta-heading);
    struct uniCycleState state;
    state.vx = -translationalState.x;
    state.vy = translationalState.y;
//    printf("%f %f %f %f :: %f\n",translationalState_.x,translationalState_.y,state.vx,state.vy,positionError_y);
    state.w = headingCorrection;
    automationUpdate();
    return state;
}

char encodeByte(int rpm) {    
    if(rpm > maxRPM) {
        printf("Warning: Trying to send Velocity greater than 180. Limit : 180. Sending 180.\r\n");
        rpm = 200; //must be even
    } else if(rpm < -(maxRPM + 1)) {
        printf("Warning: Trying to send Velocity lesser than -181. Limit : -181. Sending -181.\r\n");
        rpm = -201; // must be odd
    }
    if(rpm < 0) {
        rpm = -rpm;
        return (rpm | 0x01);
    } else {
        rpm = (rpm & 0xFE);
        return rpm == 0x0A ? 0x0C : rpm;
    }
}

struct uniCycleState getDesiredUniCycleState() {
	struct uniCycleState desiredUniCycleState;
	headingError = 0 - getHeading();
	if(abs(headingError) > HEADING_TOL) {
		headingCorrection = PID(headingError, headingControl);
	} else {
        	headingCorrection = 0;
	}
	desiredUniCycleState.w = headingCorrection;
	desiredUniCycleState.vy = (128 - ps2_getY()) * maxVELOCITY / 128;
	desiredUniCycleState.vx = -(128 - ps2_getX()) * maxVELOCITY / 128;
	printf("%f\n",desiredUniCycleState.vy);
	printf("%f\n",desiredUniCycleState.vx);
	return desiredUniCycleState;
}


void transmitOmniState(struct omniDriveState state) {
    state = rpmLimiter(state);
//    printf("%d %d %d, %f %f, %f %f\r\n", state.aRPM, state.bRPM, state.cRPM, getCurrentPosition().x, getCurrentPosition().y,destination.x,destination.y);
    printf("%f %f %f :: %f %f %f\r\n", getHeading(), getCurrentPosition().x, getCurrentPosition().y, desiredHeading, destination.x,destination.y);
#if PLOT == 1
    fprintf(fp,"%f %f\n",zero.x, zero.y);
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
		transmitOmniState(transformUniToOmni(traceLine(getCurrentPosition(), getHeading(), destination, init, desiredHeading, lineSensorAdd_mem[index], 0), 240));
//		transmitOmniState(transformUniToOmni(getDesiredUniCycleState(), 240));
		digitalWrite(miscLED, !digitalRead(miscLED));
	}
}

void CircleButtonPressed() {
}

void CircleButtonReleased() {
}

void TriangleButtonPressed() {
}

void TriangleButtonReleased() {
}

int main(){
	checkpoint.x = checkpoint_mem[index][0];
	checkpoint.y = checkpoint_mem[index][1];
	desiredHeading = heading_mem[index];
	destination = checkpoint;
	init = zero;
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
	
	leftLineSensor.address = 1;
	leftLineSensor.uartPort = rpiPort;
	leftLineSensor.UARTPin = 6;
	leftLineSensor.junctionPin = 13;
	
	backLineSensor.address = 2;
	backLineSensor.uartPort = rpiPort;
	backLineSensor.UARTPin = 12;
	backLineSensor.junctionPin = 5;

	struct encoder encoderX;
	encoderX.channelA_pin = 23;//23
	encoderX.channelB_pin = 24;//24
	encoderX.radius = 2.94;
	encoderX.PPR = 360;

	struct encoder encoderY;
	encoderY.channelA_pin = 26;//26
	encoderY.channelB_pin = 22;//22
	encoderY.radius = 2.94;
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
	initPIDController(4.0,0.0,0.0,positionControl);
	initPIDController(1.0,0.0,0.0, leftLineSensorControl);
	initPIDController(1.0,0.0,0.0, backLineSensorControl);
	
	enablePS2StatusInterrupt(&ps2Activated, &ps2Deactivated);
	enableIMUStatusInterrupt(&imuActivated, &imuDeactivated);
	enableSlowFuncInterrupt(&slowTimerHandler);
	
	initPS2();
	enableCircleButton(&CircleButtonPressed, &CircleButtonReleased);
	enableTriangleButton(&TriangleButtonPressed, &TriangleButtonReleased);
	
	initIMU();
	initRPS(encoderX,encoderY,46,33,zero);	
	initLineSensor(leftLineSensor,leftJunctionDetect);
	initLineSensor(backLineSensor,backJunctionDetect);

	initTimer(25000, &timerHandler);
	while(1) {
		sleep(1);
	}
}
