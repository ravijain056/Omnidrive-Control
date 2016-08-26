#include "LSA08.h"
#include <stdio.h>
#include <wiringPi.h>
#include <math.h>
#include <wiringSerial.h>
#include "controlmath.h"

char getLineSensorData(struct lineSensor ls){
	digitalWrite(ls.UARTPin, LOW);
	char data = serialGetchar(ls.uartPort);
	digitalWrite(ls.UARTPin, HIGH);
	return data;
}

signed int readLineSensor(struct lineSensor ls) {
	signed int out = getLineSensorData(ls);
	(out != 255) ? out = (35-out) : out = 255;
	return out;
}

struct point finePos(struct lineSensor ls, float lineTheta, struct point junctionPoint, struct point curPos, float curHeading) {
	char data = getLineSensorData(ls);
	if(data == 255) {
		return curPos;
	}
	float d = (data - 35.0)/ 35.0 * 5.6;
	float deltaY = (d * sin(degreeToRadian(lineTheta - curHeading - ls.SAngle))) + 
			(ls.distanceFromCenter * sin(degreeToRadian(lineTheta - curHeading - ls.DAngle)));
	junctionPoint = rotationalTransform(junctionPoint, lineTheta);
	float correctY = junctionPoint.y + deltaY;
	return rotationalTransform(rotationalTransform(curPos, lineTheta).x, correctY, -lineTheta); // Please do /*not*/ try to comprehend this.
}

void initLineSensor(struct lineSensor ls) {
	initLineSensor(ls, NULL);
}

void initLineSensor(struct lineSensor ls, void (*junctionInterruptHandler)(void)) {
	pinMode(ls.UARTPin, OUTPUT);
	digitalWrite(ls.UARTPin, HIGH);
	if(wiringPiISR(ls.junctionPin,INT_EDGE_RISING,junctionInterruptHandler)<0) {
		printf("line sensor interrupt setup error \n");
	}
}
