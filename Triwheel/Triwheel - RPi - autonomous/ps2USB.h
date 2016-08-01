#ifndef ps2USB
#define ps2USB

#define PS2FILE "/dev/input/event0"

int initPS2(void);
int initPS2(char* device);
void stopPS2();
int ps2_getX(void);
int ps2_getY(void);

void enableCircleButton(void (*pressed)(void), void (*released)(void));
void enableSquareButton(void (*pressed)(void), void (*released)(void));
void enableTriangleButton(void (*pressed)(void), void (*released)(void));
void enableCrossButton(void (*pressed)(void), void (*released)(void));
void enableL1Button(void (*pressed)(void), void (*released)(void));
void enableL2Button(void (*pressed)(void), void (*released)(void));
void enableL3Button(void (*pressed)(void), void (*released)(void));
void enableR1Button(void (*pressed)(void), void (*released)(void));
void enableR2Button(void (*pressed)(void), void (*released)(void));
void enableR3Button(void (*pressed)(void), void (*released)(void));
void enableStartButton(void (*pressed)(void), void (*released)(void));
void enableSelectButton(void (*pressed)(void), void (*released)(void));

void disableCircleButton();
void disableSquareButton();
void disableTriangleButton();
void disableCrossButton();
void disableL1Button();
void disableL2Button();
void disableL3Button();
void disableR1Button();
void disableR2Button();
void disableR3Button();
void disableStartButton();
void disableSelectButton();

void enablePS2StatusInterrupt(void (*activated)(void), void (*deactivated)(void));
 
#endif