/*
 * main.c
 */
#include "userLib/common.h"
#include "userLib/init.h"
#include "userLib/pidController.h"
#include "userLib/movingArray.h"

#define KP_C 0.06
#define KI_C 0.0
#define KD_C 0.4

volatile float desiredRPM = 0.0, currentRPM = 0.0, out = 0.0;
int uartReceiveCount = 0;
int32_t maxPWM = 0, minPWM = 0;

int main(void) {
	SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);
	initPIDController(0,KP_C,KI_C,KD_C);
	motorDirInit();
	uart0Init();
	uart5Init();
	pwmInit();
	maxPWM = SysCtlClockGet()/(PWMfrequency*64);
	qeiInit();
	IntMasterEnable();
	while(1) {
		GraphPlot0((int)currentRPM,(int)desiredRPM,0,0);
	}
}

void Timer0IntHandler(void) {
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	// Clear the timer interrupt

}

void UARTIntHandler(void) {
	uint32_t ui32Status;
	ui32Status = UARTIntStatus(UART5_BASE, true); //get interrupt status
	UARTIntClear(UART5_BASE, ui32Status); //clear the asserted interrupts
	char data = HWREG(UART5_BASE + UART_O_DR);
	if(data == 0x0A) {
		uartReceiveCount = 0;
	} else {
		switch(uartReceiveCount) {
			case 0: // Ignore motor A
				break;
			case 1: // Ignore motor B
				break;
			case 2:
				desiredRPM = ((data * ((data & 0x01) ? -1 : 1)));
				break;
		}
		uartReceiveCount++;
	}
}

void QEIIntHandler(void) {
	QEIIntClear(QEI0_BASE, QEI_INTTIMER);
	currentRPM = movingArrayOut(calculateRPM());
	out += PID(0,desiredRPM - currentRPM);
	setPWM(out);
}
