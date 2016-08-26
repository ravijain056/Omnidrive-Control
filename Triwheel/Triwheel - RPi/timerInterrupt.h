#ifndef timerInterrupt
#define timerInterrupt

int initTimer(float timePeriod_, void (*timerHandler)(void));
void stopTimer();
void enableSlowFuncInterrupt(void (*interruptHandler)(void));

#endif