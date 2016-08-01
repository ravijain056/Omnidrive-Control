#ifndef RPS
#define RPS

// Relative Positioning System

struct encoder { int channelA_pin; int channelB_pin; int channelZ_pin; int PPR; float radius; };

void initRPS(struct encoder encoderX,struct encoder encoderY);
void initRPS(struct encoder encoderX, struct encoder encoderY, float distanceFromCenter_, float offsetAngle_);
void initRPS(struct encoder encoderX, struct encoder encoderY,  float distanceFromCenter_, 
		float offsetAngle_, struct point initPosition);
struct point RPS_getPosition();
void RPS_setPosition(struct point pos);
void setCurrentPosition(struct point pos);
struct point getCurrentPosition();

#endif