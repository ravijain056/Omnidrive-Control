#include "pidController.h"

volatile float errorIntegral[maxPIDControllers];
volatile float prevError[maxPIDControllers];

volatile float Kp[maxPIDControllers];
volatile float Ki[maxPIDControllers];
volatile float Kd[maxPIDControllers];

void initPIDController(int i, float Kp_, float Ki_, float Kd_) {
	Kp[i] = Kp_;
	Ki[i] = Ki_;
	Kd[i] = Kd_;
	errorIntegral[i] = 0.0;
	prevError[i] = 0.0;
}

float PID(int i, float error) {
	float pid = (Kp[i] * error) + (Ki[i] * errorIntegral[i]) + (Kd[i] * (error - prevError[i]));
	errorIntegral[i] += error;
	prevError[i] = error;
	return pid;
}
