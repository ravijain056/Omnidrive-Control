/*
 * movingArray.c
 *
 *  Created on: Jun 16, 2016
 *      Author: Aniket
 */
#include "movingArray.h"

float movingArray[movingArrayNumber][movingArrayFrequency];
float sum[movingArrayNumber] = {0,0};
long int index[movingArrayNumber] = {0,0};

float movingArrayOut(int i,float in) {
	if(index[i] < movingArrayFrequency) {
		movingArray[i][index[i]] = in;
		index[i]++;

		sum[i] += in;
		return sum[i]/index[i];
	}
	sum[i] = sum[i] - movingArray[i][(index[i])%10];
	movingArray[i][(index[i])%10] = in;
	index[i]++;

	sum[i] += in;
	return sum[i]/movingArrayFrequency;
}

