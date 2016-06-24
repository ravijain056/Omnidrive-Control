#ifndef controlmath
#define controlmath

#define PI 3.1415926

//Bot specifications
#define wheelRadius 5.0
#define wheelCircumference (2 * PI * wheelRadius)
#define BotRadius  46.2
#define frTheta degreeToRadian(144.46)
#define flTheta degreeToRadian(215.5376)
#define blTheta degreeToRadian(324.46)
#define brTheta degreeToRadian(35.5376)

struct point { float x; float y; };
struct uniCycleState {float vx; float vy; float w;};
struct omniDriveState {int aRPM; int bRPM; int cRPM;};

float sigmoid(int x);
float degreeToRadian(float degree);
float radianToDegree(float radian);
float normalizeAngle(float degree);
float getDistanceBetweenPoints(struct point pointA, struct point pointB);
struct point rotationalTransform(struct point point_, float theta);
struct point rotationalTransform(float x_, float y_, float theta);
struct omniDriveState transformUniToOmni(struct uniCycleState unistate, float alpha);

#endif