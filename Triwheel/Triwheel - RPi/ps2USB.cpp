#include "ps2USB.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <linux/input.h>
#include <fcntl.h>

pthread_t PS2thread;
int x = 128, y = 128;

bool ps2StatusInterruptEnabled = false;

void (*PS2activated)(void);
void (*PS2deactivated)(void);

bool circleEnabled = false;
bool squareEnabled = false;
bool crossEnabled = false;
bool triangleEnabled = false;
bool L1Enabled = false;
bool L2Enabled = false;
bool L3Enabled = false;
bool R1Enabled = false;
bool R2Enabled = false;
bool R3Enabled = false;
bool startEnabled = false;
bool selectEnabled = false;

void (*circlePressed)(void);
void (*circleReleased)(void);
void (*squarePressed)(void);
void (*squareReleased)(void);
void (*crossPressed)(void);
void (*crossReleased)(void);
void (*trianglePressed)(void);
void (*triangleReleased)(void);
void (*L1Pressed)(void);
void (*L1Released)(void);
void (*L2Pressed)(void);
void (*L2Released)(void);
void (*L3Pressed)(void);
void (*L3Released)(void);
void (*R1Pressed)(void);
void (*R1Released)(void);
void (*R2Pressed)(void);
void (*R2Released)(void);
void (*R3Pressed)(void);
void (*R3Released)(void);
void (*startPressed)(void);
void (*startReleased)(void);
void (*selectPressed)(void);
void (*selectReleased)(void);

void* ps2Read(void *param) {
    int fd;
    struct input_event ie;

    if((fd = open(PS2FILE, O_RDONLY)) == -1) {
        if(ps2StatusInterruptEnabled) {
        	(*PS2deactivated)();
        }
        perror("Cant detect ps2USB:\n");
        exit(EXIT_FAILURE);
    }

    while(read(fd, &ie, sizeof(struct input_event)))  {
  	//printf("%d %d %d\n", ie.type, ie.code, ie.value);
  	if(ie.type == 3) {
  		if(ie.code == 0) {
  			x = ie.value;
  		} else if(ie.code == 1) {
			y = ie.value;
  		}
  	} else if(ie.type == 1) {
  		switch(ie.code) {
  			case 288: //triangle (0-1)
  				if(triangleEnabled) {
					if(ie.value) {
						(*trianglePressed)();
					} else {
						(*triangleReleased)();
					}
				}
				break;
  			case 289: //Cirlce
				if(circleEnabled) {
					if(ie.value) {
						(*circlePressed)();
					} else {
						(*circleReleased)();
					}
				}
  				break;
  			case 290: //Cross
  				if(crossEnabled) {
					if(ie.value) {
						(*crossPressed)();
					} else {
						(*crossReleased)();
					}
				}
  				break;
  			case 291: //Square
  				if(squareEnabled) {
					if(ie.value) {
						(*squarePressed)();
					} else {
						(*squareReleased)();
					}
				}
  				break;
  			case 292: //L1
  				if(L2Enabled) {
					if(ie.value) {
						(*L1Pressed)();
					} else {
						(*L1Released)();
					}
				}
  				break;
  			case 293: //R1
  				if(R2Enabled) {
					if(ie.value) {
						(*R1Pressed)();
					} else {
						(*R1Released)();
					}
				}
  				break;
  			case 294: //L2
  				if(L2Enabled) {
					if(ie.value) {
						(*L2Pressed)();
					} else {
						(*L2Released)();
					}
				}
  				break;
  			case 295: //R2
  				if(R2Enabled) {
					if(ie.value) {
						(*R2Pressed)();
					} else {
						(*R2Released)();
					}
				}
  				break;
  			case 296: //select
  				if(selectEnabled) {
					if(ie.value) {
						(*selectPressed)();
					} else {
						(*selectReleased)();
					}
				}
  				break;
  			case 297: //start
  				if(startEnabled) {
					if(ie.value) {
						(*startPressed)();
					} else {
						(*startReleased)();
					}
				}
  				break;
  			case 298: //L3
  				if(L3Enabled) {
					if(ie.value) {
						(*L3Pressed)();
					} else {
						(*L3Released)();
					}
				}
  				break;
  			case 299: //R3
  				if(R3Enabled) {
					if(ie.value) {
						(*R3Pressed)();
					} else {
						(*R3Released)();
					}
				}
  				break;
  		}
  	}
    }
}

int initPS2(const char* device) {
	int i = pthread_create(&PS2thread, NULL, ps2Read, NULL);
	if(ps2StatusInterruptEnabled) {
		if(i == -1) {
			(*PS2deactivated)();
        	} else {
        		(*PS2activated)();
        	}
        }
        return i;
}

int initPS2() {
	return initPS2(PS2FILE);
}

void stopPS2(){
	pthread_cancel(PS2thread);
	if(ps2StatusInterruptEnabled) {
        	(*PS2deactivated)();
        }
}

int ps2_getX() {
	return x;
}

int ps2_getY() {
	return y;
}

void enablePS2StatusInterrupt(void (*activated)(void), void (*deactivated)(void)) {
	ps2StatusInterruptEnabled = true;
	PS2activated = activated;
	PS2deactivated = deactivated;
}

void enableCircleButton(void (*pressed)(void), void (*released)(void)){
	circleEnabled = true;
	circlePressed = pressed;
	circleReleased = released;
}

void disableCircleButton() {
	circleEnabled = false;
}

void enableSquareButton(void (*pressed)(void), void (*released)(void)){
	squareEnabled = true;
	squarePressed = pressed;
	squareReleased = released;
}

void disableSquareButton() {
	squareEnabled = false;
}

void enableCrossButton(void (*pressed)(void), void (*released)(void)){
	crossEnabled = true;
	crossPressed = pressed;
	crossReleased = released;
}

void disableCrossButton() {
	crossEnabled = false;
}

void enableTriangleButton(void (*pressed)(void), void (*released)(void)){
	triangleEnabled = true;
	trianglePressed = pressed;
	triangleReleased = released;
}

void disableTriangleButton() {
	triangleEnabled = false;
}

void enableL1Button(void (*pressed)(void), void (*released)(void)){
	L1Enabled = true;
	L1Pressed = pressed;
	L1Released = released;
}

void disableL1Button() {
	L1Enabled = false;
}

void enableL2Button(void (*pressed)(void), void (*released)(void)){
	L2Enabled = true;
	L2Pressed = pressed;
	L2Released = released;
}

void disableL2Button() {
	L2Enabled = false;
}

void enableL3Button(void (*pressed)(void), void (*released)(void)){
	L3Enabled = true;
	L3Pressed = pressed;
	L3Released = released;
}


void disableL3Button() {
	L3Enabled = false;
}

void enableR1Button(void (*pressed)(void), void (*released)(void)){
	R1Enabled = true;
	R1Pressed = pressed;
	R1Released = released;
}


void disableR1Button() {
	R1Enabled = false;
}

void enableR2Button(void (*pressed)(void), void (*released)(void)){
	R2Enabled = true;
	R2Pressed = pressed;
	R2Released = released;
}

void disableR2Button() {
	R2Enabled = false;
}


void enableR3Button(void (*pressed)(void), void (*released)(void)){
	R3Enabled = true;
	R3Pressed = pressed;
	R3Released = released;
}

void disableR3Button() {
	R3Enabled = false;
}

void enableStartButton(void (*pressed)(void), void (*released)(void)){
	startEnabled = true;
	startPressed = pressed;
	startReleased = released;
}

void disableStartButton() {
	startEnabled = false;
}

void enableSelectButton(void (*pressed)(void), void (*released)(void)){
	selectEnabled = true;
	selectPressed = pressed;
	selectReleased = released;
}

void disableSelectButton() {
	selectEnabled = false;
}
