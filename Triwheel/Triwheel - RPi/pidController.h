#ifndef pidController
#define pidController

#define maxN 4

void initPIDController(float kp_, float ki_, float kd_, int i);
float PID(float error, int i);
float setPIDBounds(float min, float max, int i);
void clearIntegralError(int i);
void resetPIDvar(int i);

#endif