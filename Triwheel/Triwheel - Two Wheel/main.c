#include "userLib/common.h"
#include "userLib/init.h"
#include "userLib/pidController.h"

#define KP_front 0.003
#define KI_front 0.0
#define KD_front 0.0
#define KP_back 0.0025
#define KI_back 0.0
#define KD_back 0.0
#define maxRPM 998
#define minRPM -998

volatile float desiredRPM[2] = {100.0,100.0}, buffered_desiredRPM[2] = {0.0,0.0}, currentRPM[2] = {0.0,0.0}, calculatedRPM[2] = {0.0,0.0};
volatile uint8_t uartReceiveCount = 0;
volatile bool toggle = false;

int main() {
	SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);
	initPIDController(Front,KP_front,KI_front,KD_front);
	initPIDController(Back,KP_back,KI_back,KD_back);
	motorDirInit();
	uart0Init();
	//uart5Init();
	pwmInit();
	qeiInit();
	timerInit();
	IntMasterEnable();
	while(1) {
		GraphPlot0((int)desiredRPM[Back],(int)calculatedRPM[Back],(int)desiredRPM[Front],(int)calculatedRPM[Front]);
	}
}

void Timer0IntHandler(void) {
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	// Clear the timer interrupt
	int i;
	float error;
	if(!toggle) {
		for(i = 0; i < 2; i++) {
			calculatedRPM[i] = calculateRPM(i);
			error = desiredRPM[i] - calculatedRPM[i];
			if(absolute(error) <= 1.5) {
				error = 0;
			}
			currentRPM[i] += PID(i,error);
			if(currentRPM[i] >= maxRPM) {
				currentRPM[i] = maxRPM;
			} else if(currentRPM[i] <= minRPM){
				currentRPM[i] = minRPM;
			}
			setPWM(currentRPM[i], i);
		}
		toggle = true;
	} else {
		for(i = 1; i >= 0; i--) {
			calculatedRPM[i] = calculateRPM(i);
			error = desiredRPM[i] - calculatedRPM[i];
			if(absolute(error) <= 1.5) {
				error = 0;
			}
			currentRPM[i] += PID(i,error);
			if(currentRPM[i] >= maxRPM) {
				currentRPM[i] = maxRPM;
			} else if(currentRPM[i] <= minRPM){
				currentRPM[i] = minRPM;
			}
			setPWM(currentRPM[i], i);
		}
		toggle = false;
	}
/*
	UART_OutDec(desiredRPM[Front],0);
	UARTCharPut(UART0_BASE,' ');
	UART_OutDec(desiredRPM[Back],0);
	UARTCharPut(UART0_BASE,'\r');*/
}

void UARTIntHandler(void)
{
	uint32_t ui32Status;
	ui32Status = UARTIntStatus(UART5_BASE, true); //get interrupt status
	UARTIntClear(UART5_BASE, ui32Status); //clear the asserted interrupts
	char data = HWREG(UART5_BASE + UART_O_DR);
	if(data == 0x0A) {
		uartReceiveCount = 0;
		desiredRPM[Front] = buffered_desiredRPM[Front];
		desiredRPM[Back] = buffered_desiredRPM[Back];
	} else {
		switch(uartReceiveCount) {
			case 0:
				buffered_desiredRPM[Front] = (int)(data * ((data & 0x01) ? -1 : 1));
				break;
			case 1:
				// Ignore front right
				break;
			case 2:
				// Ignore back right
				break;
			case 3:
				buffered_desiredRPM[Back] = (int)(data * ((data & 0x01) ? -1 : 1));
				break;
		}
		uartReceiveCount++;
	}
}
