#ifndef minimu9
#define minimu9

void initIMU(void);
void stopIMU(void);
void resetRefHeading(void);

float getHeading(void);
float getAbsoluteHeading(void);
float getRefHeading(void);

void enableIMUStatusInterrupt(void (*activated)(void), void (*deactivated)(void));

#endif