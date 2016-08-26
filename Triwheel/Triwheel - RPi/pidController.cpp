#include "pidController.h"


float kp[maxN];
float ki[maxN];
float kd[maxN];
float errorInteg[maxN];
float prevError[maxN];

float minPID[maxN];
float maxPID[maxN];
bool isBounded[maxN];


void initPIDController(float kp_, float ki_, float kd_, int i) {
	kp[i] = kp_;
	ki[i] = ki_;
	kd[i] = kd_;
	prevError[i] = 0.0;
	errorInteg[i] = 0.0;
	isBounded[i] = false;
}

float PID(float error, int i) {
	float pid = kp[i] * error + ki[i] * errorInteg[i] + kd[i] * (error - prevError[i]);
	errorInteg[i] += kp[i]*error;
	prevError[i] = error;
	if(isBounded[i]) {
		pid = (pid > maxPID[i]) ? maxPID[i] : (pid < minPID[i]) ? minPID[i] : pid;
	}
	return pid;
}

float setPIDBounds(float min, float max, int i) {
	minPID[i] = min;
	maxPID[i] = max;
	isBounded[i] = true;
}

void clearIntegralError(int i) {
	errorInteg[i] = 0;
}

void resetPIDvar(int i) {
	clearIntegralError(i);
	prevError[i] = 0;
}