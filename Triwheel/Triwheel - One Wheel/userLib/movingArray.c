/*
 * movingArray.c
 *
 *  Created on: Jun 16, 2016
 *      Author: Aniket
 */
#include "movingArray.h"

float movingArray[movingArrayFrequency];
float sum = 0;
long int index = 0;

float movingArrayOut(float in) {
	if(index < movingArrayFrequency) {
		movingArray[index] = in;
		index++;

		sum += in;
		return sum/index;
	}
	sum = sum - movingArray[index%10];
	movingArray[index%10] = in;
	index++;

	sum += in;
	return sum/movingArrayFrequency;
}

