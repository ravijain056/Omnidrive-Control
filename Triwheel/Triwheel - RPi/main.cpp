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

#define powerOffButton 4
#define headingRefButton 0
#define ps2InputLED 25
#define headingLED 29
#define slowLoopLED 27
#define miscLED 28
#define maxRPM 200
#define HEADING_TOL 5

bool ps2Ready = false;
bool imuReady = false;
bool powerOffPressed = false;

float desiredHeading = 0.0;
float headingCorrection = 0;
float headingError = 0;
int rpiPort;

int prevPS2_x = 128, prevPS2_y = 128;
float prevX = 0.0, prevY = 0.0;
float vmax = 150;

struct omniDriveState stopState;
enum {headingControl};

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
	headingCorrection = 0;
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

struct uniCycleState getDesiredUniCycleState() {
	struct uniCycleState desiredUniCycleState;
	headingError = desiredHeading - getHeading();
	if(abs(headingError) > HEADING_TOL) {
		headingCorrection += PID(headingError, headingControl);
	} else {
        	headingCorrection = 0;
	}
	desiredUniCycleState.w = headingCorrection;
	desiredUniCycleState.vy = (128 - ps2_getY()) * vmax / 128;
	desiredUniCycleState.vx = (ps2_getX() - 128) * vmax / 128;
	return desiredUniCycleState;
}

char encodeByte(int rpm) {
    if(rpm > maxRPM) {
        printf("Warning: Trying to send Velocity greater than 200. Limit : 200. Sending 200.\r\n");
        rpm = maxRPM; //must be even
    } else if(rpm < -(maxRPM+1)) {
        printf("Warning: Trying to send Velocity lesser than -201. Limit : -201. Sending -201.\r\n");
        rpm = -(maxRPM+1); // must be odd
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
    printf("%d %d %d\r\n", state.aRPM, state.bRPM, state.cRPM);
    serialPutchar(rpiPort, 0x0A);
    serialPutchar(rpiPort, encodeByte(state.aRPM));
    serialPutchar(rpiPort, encodeByte(state.bRPM));
    serialPutchar(rpiPort, encodeByte(state.cRPM));
}

void timerHandler() {
	if(!ps2Ready || !imuReady) {
		transmitOmniState(stopState);
	} else {
		transmitOmniState(transformUniToOmni(getDesiredUniCycleState(), 0));
		digitalWrite(miscLED, !digitalRead(miscLED));
	}
}

int main(){
	rpiPort = serialOpen("/dev/ttyS0",38400);

	stopState.aRPM = 0.0;
	stopState.bRPM = 0.0;
	stopState.cRPM = 0.0;
	
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
		
	initPIDController(0.0005, 0.0, 0.04, headingControl);
//	initPIDController(0.0,0.0,0.0,translationalControl);
	
	enablePS2StatusInterrupt(&ps2Activated, &ps2Deactivated);
	enableIMUStatusInterrupt(&imuActivated, &imuDeactivated);
	enableSlowFuncInterrupt(&slowTimerHandler);
	
	initPS2();
	initIMU();
//	initRPS(encoderX,encoderY,14.0,0.0);	
	
	initTimer(25000, &timerHandler);
	while(1) {
		sleep(1);
	}
}
