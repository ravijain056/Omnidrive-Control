#include "userLib/common.h"
#include "userLib/init.h"
#include "userLib/pidController.h"

#define KP 0.0035
#define KI 0.0
#define KD 0.0
#define maxRPM 998
#define minRPM -998

volatile float desiredRPM = 100.0, buffered_desiredRPM = 0.0, currentRPM = 0.0, calculatedRPM = 0.0;
volatile uint8_t uartReceiveCount = 0;

int main() {
	SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);
	initPIDController(0,KP,KI,KD);
	motorDirInit();
	uart1Init();
	//uart5Init();
	pwmInit();
	qeiInit();
	timerInit();
	IntMasterEnable();
	while(1) {
		GraphPlot1((int)desiredRPM,(int)calculatedRPM,0,0);
	}
}

void Timer0IntHandler(void) {
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	// Clear the timer interrupt
	float error;
	calculatedRPM = calculateRPM();
	error = desiredRPM - calculatedRPM;
	if(absolute(error) <= 1.5) {
		error = 0;
	}
	currentRPM += PID(0,error);
	if(currentRPM >= maxRPM) {
		currentRPM = maxRPM;
	} else if(currentRPM <= minRPM){
		currentRPM = minRPM;
	}
	setPWM(currentRPM);
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
		desiredRPM = buffered_desiredRPM;
	} else {
		switch(uartReceiveCount) {
			case 0:
//				buffered_desiredRPM[Front] = (int)(data * ((data & 0x01) ? -1 : 1));
				break;
			case 1:
				// Ignore front right
				break;
			case 2:
				// Ignore back right
				break;
			case 3:
//				buffered_desiredRPM[Back] = (int)(data * ((data & 0x01) ? -1 : 1));
				break;
		}
		uartReceiveCount++;
	}
}
