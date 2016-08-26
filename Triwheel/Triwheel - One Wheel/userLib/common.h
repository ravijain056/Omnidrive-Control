#ifndef common
#define common

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_ints.h"
#include "inc/hw_uart.h"
#include "inc/tm4c123gh6pm.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"
#include "driverlib/pwm.h"
#include "driverlib/qei.h"

enum {Front, Back};

#define PIDfrequency 1000
#define PWMfrequency 2000
#define motorDirectionRegister GPIO_PORTF_BASE
#define Motor1 GPIO_PIN_2
#define Motor2 GPIO_PIN_3

void setPWM(int pwm);
int calculateRPM(void);
unsigned absolute(int x);
void UART_TransmitString(char* string,int i);
void UART_OutDec(int x, int i);
void GraphPlot0(int data1, int data2, int data3, int data4);
void GraphPlot1(int data1, int data2, int data3, int data4);

#endif
