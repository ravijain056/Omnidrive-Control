#ifndef controlmath
#define controlmath

#define PI 3.1415926

#ifndef maxRPM
#define maxRPM 200
#endif

/*
	From the zero reference of the robot, the first wheel that comes in anticlockwise direction is A then B and finally C.
*/

//Bot specifications
#define wheelRadius 5.0
#define wheelCircumference (2 * PI * wheelRadius)
#define BotRadius  46.2
#define wheelA_theta degreeToRadian(60.0)
#define wheelB_theta degreeToRadian(180.0)
#define wheelC_theta degreeToRadian(300.0)

struct point { float x; float y; };
struct uniCycleState {float vx; float vy; float w;};
struct omniDriveState {int aRPM; int bRPM; int cRPM;};

float sigmoid(float x);
float degreeToRadian(float degree);
float radianToDegree(float radian);
float normalizeAngle(float degree);
float getDistanceBetweenPoints(struct point pointA, struct point pointB);
struct point rotationalTransform(struct point point_, float theta);
struct point rotationalTransform(float x_, float y_, float theta);
struct omniDriveState transformUniToOmni(struct uniCycleState unistate, float alpha); //alpha is the angular offset given to the robot 
																					  //heading from the zero reference
struct omniDriveState rpmLimiter(struct omniDriveState);

#endif
