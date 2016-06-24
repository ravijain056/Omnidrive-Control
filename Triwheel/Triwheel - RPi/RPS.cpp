#include "RPS.h"
#include <stdio.h>
#include <math.h>
#include "minimu9.h"
#include "controlmath.h"
#include <wiringPi.h>

enum {ENCODER_X, ENCODER_Y};
struct encoder encoderX_, encoderY_;
struct point curPos;
float k[2];
float distanceFromCenter;
float offsetAngle;

/* Encoder code */
void encoderInterruptY() {
	if(digitalRead(encoderY_.channelB_pin)) {
		curPos.x += (k[ENCODER_Y] * sin(degreeToRadian(getHeading())));
		curPos.y += (k[ENCODER_Y] * cos(degreeToRadian(getHeading())));
	} else {
		curPos.x -= (k[ENCODER_Y] * sin(degreeToRadian(getHeading())));
		curPos.y -= (k[ENCODER_Y] * cos(degreeToRadian(getHeading())));
	}
}

void encoderInterruptX() {
	if(digitalRead(encoderX_.channelB_pin)) {
		curPos.x += (k[ENCODER_X] * cos(degreeToRadian(getHeading())));
		curPos.y += (k[ENCODER_X] * sin(degreeToRadian(getHeading())));
	} else {
		curPos.x -= (k[ENCODER_X] * cos(degreeToRadian(getHeading())));
		curPos.y -= (k[ENCODER_X] * sin(degreeToRadian(getHeading())));
	}
}

void initRPS(struct encoder encoderX, struct encoder encoderY){
	initRPS(encoderX, encoderY, 0, 0);
}

void initRPS(struct encoder encoderX, struct encoder encoderY, float distanceFromCenter_, float offsetAngle_) {
	struct point initPosition;
	initPosition.x = 0;
	initPosition.y = 0; 
	initRPS(encoderX, encoderY, distanceFromCenter_, offsetAngle_, initPosition);
}

void initRPS(struct encoder encoderX, struct encoder encoderY,  float distanceFromCenter_, float offsetAngle_, struct point initPosition) {
	distanceFromCenter = distanceFromCenter_;
	offsetAngle = offsetAngle_;
	encoderX_.channelB_pin = encoderX.channelB_pin;
	encoderY_.channelB_pin = encoderY.channelB_pin;
	k[ENCODER_X] = (float)(2 * PI * encoderX.radius) / encoderX.PPR;
	k[ENCODER_Y] = (float)(2 * PI * encoderY.radius) / encoderY.PPR;
	
	if(encoderX.channelB_pin!=-1) {
		pinMode(encoderX.channelB_pin, INPUT);
	}
	if(encoderY.channelB_pin!=-1) {
		pinMode(encoderY.channelB_pin, INPUT);
	}
	/*
		Add  initializations for Z phase
	*/
	if(wiringPiISR(encoderX.channelA_pin, INT_EDGE_RISING, &encoderInterruptX) < 0) {
		printf("Interrup setup error \n");
	}
	if(wiringPiISR(encoderY.channelA_pin, INT_EDGE_RISING, &encoderInterruptY) < 0) {
		printf("Interrup setup error \n");
	}
	setCurrentPosition(initPosition);
}

void RPS_setPosition(struct point pos) {
	curPos = pos;
}

struct point RPS_getPosition(void) {
	return curPos;
}

void setCurrentPosition(struct point pos) {
	float theta = degreeToRadian(offsetAngle + getHeading());
	pos.x += (distanceFromCenter * cos(theta));
	pos.y += (distanceFromCenter * sin(theta));
	RPS_setPosition(pos);
}

struct point getCurrentPosition() {
	struct point pos = RPS_getPosition();
	float theta = degreeToRadian(offsetAngle + getHeading());
	pos.x -= (distanceFromCenter * cos(theta));
	pos.y -= (distanceFromCenter * sin(theta));
	return pos; 
}

