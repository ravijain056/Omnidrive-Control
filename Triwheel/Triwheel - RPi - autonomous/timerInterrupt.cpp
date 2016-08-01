#include "timerInterrupt.h"
#include "unistd.h"	
#include <sys/time.h>
#include <stdio.h>
#include <pthread.h>

pthread_t timerThread;
float timePeriod;
struct timeval start, stop;
void (*timerfunc)(void);
void (*slowfuncInterrupt)(void);
bool slowFuncInterruptEnabled = false;

void* timerLoop(void* param) {
	gettimeofday(&start, NULL);
	while(1) {
		gettimeofday(&stop, NULL);
		long int timelap = timePeriod -((stop.tv_sec * 1000000 + stop.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec));
		if(timelap < 0) {
			if(slowFuncInterruptEnabled) {
				(*slowfuncInterrupt)();
			}
		} else {
			usleep(timelap);
		}
		gettimeofday(&start, NULL);
		(*timerfunc)();
	}
}

int initTimer(float timePeriod_, void (*timerHandler)(void)) {
	timePeriod = timePeriod_;
	timerfunc = timerHandler;
	return pthread_create(&timerThread, NULL, timerLoop, NULL);
}

void stopTimer(){
	pthread_cancel(timerThread);
}

void enableSlowFuncInterrupt(void (*interruptHandler)(void)) {
	slowfuncInterrupt = interruptHandler;
	slowFuncInterruptEnabled = true;
}