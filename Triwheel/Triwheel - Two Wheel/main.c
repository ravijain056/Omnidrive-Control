#include "userLib/common.h"
#include "userLib/init.h"
#include "userLib/pidController.h"
#include "userLib/movingArray.h"

#define KP_motorA 0.06
#define KI_motorA 0.0
#define KD_motorA 0.4
#define KP_motorB 0.06
#define KI_motorB 0.0
#define KD_motorB 0.4

volatile float desiredRPM[2] = {0.0,0.0}, currentRPM[2] = {0.0,0.0}, out[2] = {0.0,0.0},bufferDesiredRPM[2] = {0.0,0.0};
int uartReceiveCount = 0;
int32_t maxPWM = 0;
int32_t minPWM = 0;

int main() {
	SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);
	initPIDController(A,KP_motorA,KI_motorA,KD_motorA);
	initPIDController(B,KP_motorB,KI_motorB,KD_motorB);
	motorDirInit();
	uart1Init();
	uart5Init();
	pwmInit();
	maxPWM = SysCtlClockGet()/(PWMfrequency*64);
	qeiInit();
	//setPWM(55,A);
	//setPWM(57,B);
	//setPWM(0,A);
	//setPWM(0,B);
	IntMasterEnable();
	while(1) {
		GraphPlot1(currentRPM[A],desiredRPM[A],currentRPM[B],desiredRPM[B]);
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
			case 0:
				bufferDesiredRPM[A] = ((data * ((data & 0x01) ? -1 : 1)));
				break;
			case 1:
				bufferDesiredRPM[B] = ((data * ((data & 0x01) ? -1 : 1)));
				break;
			case 2:
				desiredRPM[A] = bufferDesiredRPM[A];
				desiredRPM[B] = bufferDesiredRPM[B];
				// Ignore motor C
				break;
		}
		uartReceiveCount++;
	}
}

void QEI0IntHandler(void) {
	QEIIntClear(QEI0_BASE, QEI_INTTIMER);
	currentRPM[A] = movingArrayOut(A,calculateRPM(A));
	out[A] += PID(A,desiredRPM[A] - currentRPM[A]);
	setPWM(out[A],A);
}

void QEI1IntHandler(void) {
	QEIIntClear(QEI1_BASE, QEI_INTTIMER);
	currentRPM[B] = movingArrayOut(B,calculateRPM(B));
	out[B] += PID(B,desiredRPM[B] - currentRPM[B]);
	setPWM(out[B],B);
}
