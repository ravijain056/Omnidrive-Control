#include "controlmath.h"
#include <math.h>

float sigmoid(int x) {
	return ((1/(1+exp(-x))) - 0.5) * 2;
}

float degreeToRadian(float degree) {
    return degree * PI / 180;
}

float radianToDegree(float radian) {
    return radian * 180 / PI;
}

float normalizeAngle(float degree) {
    return radianToDegree(atan2(sin(degreeToRadian(degree)), cos(degreeToRadian(degree))));
}

float getDistanceBetweenPoints(struct point pointA, struct point pointB) {
	return sqrt(pow((pointB.x - pointA.x), 2) + pow((pointB.y - pointA.y), 2));
}

struct point rotationalTransform(struct point point_, float theta) {
	return rotationalTransform(point_.x, point_.y, theta);
}

struct point rotationalTransform(float x, float y, float theta){
    struct point pos;
    theta = degreeToRadian(theta);
    pos.x = x * cos(theta) + y * sin(theta);
    pos.y = y * cos(theta) - x * sin(theta);
    return pos;
}

struct omniDriveState transformUniToOmni(struct uniCycleState unistate, float alpha) {
    struct omniDriveState transformedState;
    float wR = unistate.w * BotRadius;
/*
    alpha = degreeToRadian(alpha);
    float v = sqrt(unistate.vx * unistate.vx + unistate.vy * unistate.vy);
    float phi = atan2(unistate.vy, unistate.vx);
    
    float cosphi = cos(phi);
    float sinphi = sin(phi);
   
    transformedState.flRPM = ((v * ((cos(alpha + flTheta) * cosphi) + (sin(alpha + flTheta) * sinphi))) + wR ) * 60.0 / wheelCircumference;
    transformedState.frRPM = ((v * ((cos(alpha + frTheta) * cosphi) + (sin(alpha + frTheta) * sinphi))) + wR ) * 60.0 / wheelCircumference;
    transformedState.brRPM = ((v * ((cos(alpha + brTheta) * cosphi) + (sin(alpha + brTheta) * sinphi))) + wR ) * 60.0 / wheelCircumference;
    transformedState.blRPM = ((v * ((cos(alpha + blTheta) * cosphi) + (sin(alpha + blTheta) * sinphi))) + wR ) * 60.0 / wheelCircumference;
*/
    transformedState.aRPM = -(unistate.vx/2) - (unistate.vy*0.866) + wR;
    transformedState.bRPM = -(unistate.vx/2) + (unistate.vy*0.866) + wR;
    transformedState.cRPM = unistate.vx + wR;
    return transformedState;
}

