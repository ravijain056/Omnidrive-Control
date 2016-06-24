#include "minimu9.h"
#include "controlmath.h"
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>

static bool isInitialised = false;
pthread_t IMUThread;
float refHeading;
float heading;
bool imuStatusInterruptEnabled = false;

void (*IMUactivated)(void);
void (*IMUdeactivated)(void);

void* headingCalculator(void *param) {
	FILE *minimu9_ = popen("minimu9-ahrs -b /dev/i2c-1 --output euler", "r");
	if(imuStatusInterruptEnabled) {
        	(*IMUactivated)();
        }
        while(1) {
		float matrix[3][3];
		int result = fscanf(minimu9_, "%f %f %f %f %f %f %f %f %f",
			&matrix[0][0], &matrix[1][0], &matrix[2][0],
			&matrix[0][1], &matrix[1][1], &matrix[2][1],
			&matrix[0][2], &matrix[1][2], &matrix[2][2]);
		if(result == 9) {
			heading = matrix[0][0];
		} else {
			printf("IMU Disconnected or reading error!");
			if(imuStatusInterruptEnabled) {
		        	(*IMUdeactivated)();
		        }
		        break;
		}
	}
}

void initIMU() {
	if(isInitialised) return;
	isInitialised = true;
	pthread_create(&IMUThread, NULL, headingCalculator, NULL);
	sleep(5);
	if(imuStatusInterruptEnabled) {
        	(*IMUactivated)();
        }
        refHeading = heading;
}

void stopIMU(){
	pthread_cancel(IMUThread);
	if(imuStatusInterruptEnabled) {
        	(*IMUdeactivated)();
        }
	isInitialised = false;
}

void enableIMUStatusInterrupt(void (*activated)(void), void (*deactivated)(void)) {
	imuStatusInterruptEnabled = true;
	IMUactivated = activated;
	IMUdeactivated = deactivated;
}


void resetRefHeading() {
	refHeading = heading;
}

float getHeading() {
    return normalizeAngle(refHeading - heading);
}

float getAbsoluteHeading() {
	return heading;
}

float getRefHeading() {
	return refHeading;
}
