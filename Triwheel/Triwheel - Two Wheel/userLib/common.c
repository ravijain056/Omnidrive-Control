#include "common.h"

void setPWM(int pwm,int i) {
	if(pwm > maxPWM) {
		pwm = maxPWM;
	} else if(pwm < -maxPWM) {
		pwm = -maxPWM;
	}
	if(i == A) {
		if(pwm > 0) {
			GPIOPinWrite(motorDirectionRegister,A1|A2,0x02);
			PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, pwm);
		} else if(pwm < 0) {
			GPIOPinWrite(motorDirectionRegister,A1|A2,0x01);
			PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, -pwm);
		} else {
			GPIOPinWrite(motorDirectionRegister,A1|A2,0x00);
			PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, 0);
		}
	} else if(i == B) {
		if(pwm > 0) {
			GPIOPinWrite(motorDirectionRegister,B1|B2,0x08);
			PWMPulseWidthSet(PWM1_BASE, PWM_OUT_1, pwm);
		} else if(pwm < 0) {
			GPIOPinWrite(motorDirectionRegister,B1|B2,0x04);
			PWMPulseWidthSet(PWM1_BASE, PWM_OUT_1, -pwm);
		} else {
			GPIOPinWrite(motorDirectionRegister,B1|B2,0x00);
			PWMPulseWidthSet(PWM1_BASE, PWM_OUT_1, 0);
		}
	}
}

int calculateRPM(int i) {
	if(i == A) {
		int rpm = (QEIVelocityGet(QEI0_BASE)*QEIDirectionGet(QEI0_BASE))*QEIfrequency;
		float RPM = rpm * 0.015;
		return RPM;
	} else if(i == B) {
		int rpm = (-QEIVelocityGet(QEI1_BASE)*QEIDirectionGet(QEI1_BASE))*QEIfrequency;
		float RPM = rpm * 0.015;
		return RPM;
	} else {
		return 0;
	}
}

unsigned int absolute(int x) {
	return ((x < 0) ? -x : x);
}

void UART_TransmitString(char *str,int i) {
	while(*str>0)	 {
		switch(i)	{
		case 0: 		UARTCharPut(UART0_BASE,(*str));
		break;
		case 1:
			UARTCharPut(UART1_BASE,(*str));
			break;
		case 2:
			UARTCharPut(UART2_BASE,(*str));
			break;
		case 3:
			UARTCharPut(UART3_BASE,(*str));
			break;
		case 4:
			UARTCharPut(UART4_BASE,(*str));
			break;
		case 5:
			UARTCharPut(UART5_BASE,(*str));
			break;
		case 6:
			UARTCharPut(UART6_BASE,(*str));
			break;
		case 7:
			UARTCharPut(UART7_BASE,(*str));
			break;
		}
		str++;
	}
}

void UART_OutUDec(unsigned long n,int i) {
	if(n >= 10){
		UART_OutUDec(n/10,i);
		n = n%10;
	}
	switch(i)
	{
	case 0:
		UARTCharPut(UART0_BASE,(n+'0'));
		break;
	case 1:
		UARTCharPut(UART1_BASE,(n+'0'));
		break;
	case 2:
		UARTCharPut(UART2_BASE,(n+'0'));
		break;
	case 3:
		UARTCharPut(UART3_BASE,(n+'0'));
		break;
	case 4:
		UARTCharPut(UART4_BASE,(n+'0'));
		break;
	case 5:
		UARTCharPut(UART5_BASE,(n+'0'));
		break;
	case 6:
		UARTCharPut(UART6_BASE,(n+'0'));
		break;
	case 7:
		UARTCharPut(UART7_BASE,(n+'0'));
	    break;
	}
}

void UART_OutDec(int x, int i) {
	if(x < 0) {
		switch(i)
		{
		case 0:
			UARTCharPut(UART0_BASE,'-');
			break;
		case 1:
			UARTCharPut(UART1_BASE,'-');
			break;
		case 2:
			UARTCharPut(UART2_BASE,'-');
			break;
		case 3:
			UARTCharPut(UART3_BASE,'-');
			break;
		case 4:
			UARTCharPut(UART4_BASE,'-');
			break;
		case 5:
			UARTCharPut(UART5_BASE,'-');
			break;
		case 6:
			UARTCharPut(UART6_BASE,'-');
			break;
		case 7:
			UARTCharPut(UART7_BASE,'-');
			break;
		}
		x *= -1;
	}
	UART_OutUDec(x, i);
}

void GraphPlot0(int data1, int data2, int data3, int data4){
	UARTCharPut(UART0_BASE,0xAB);
	UARTCharPut(UART0_BASE,0xCD);
	UARTCharPut(UART0_BASE,0x08);
	UARTCharPut(UART0_BASE,0x00);
	UARTCharPut(UART0_BASE,(int)data1 & 0x00FF);
	UARTCharPut(UART0_BASE,(((int)data1 & 0xFF00) >> 8));
	UARTCharPut(UART0_BASE,(int)data2 & 0x00FF);
	UARTCharPut(UART0_BASE,(((int)data2 & 0xFF00) >> 8));
	UARTCharPut(UART0_BASE,(int)data3 & 0x00FF);
	UARTCharPut(UART0_BASE,(((int)data3 & 0xFF00) >> 8));
	UARTCharPut(UART0_BASE,(int)data4 & 0x00FF);
	UARTCharPut(UART0_BASE,(((int)data4 & 0xFF00) >> 8));
}

void GraphPlot1(int data1, int data2, int data3, int data4){
	UARTCharPut(UART1_BASE,0xAB);
	UARTCharPut(UART1_BASE,0xCD);
	UARTCharPut(UART1_BASE,0x08);
	UARTCharPut(UART1_BASE,0x00);
	UARTCharPut(UART1_BASE,(int)data1 & 0x00FF);
	UARTCharPut(UART1_BASE,(((int)data1 & 0xFF00) >> 8));
	UARTCharPut(UART1_BASE,(int)data2 & 0x00FF);
	UARTCharPut(UART1_BASE,(((int)data2 & 0xFF00) >> 8));
	UARTCharPut(UART1_BASE,(int)data3 & 0x00FF);
	UARTCharPut(UART1_BASE,(((int)data3 & 0xFF00) >> 8));
	UARTCharPut(UART1_BASE,(int)data4 & 0x00FF);
	UARTCharPut(UART1_BASE,(((int)data4 & 0xFF00) >> 8));
}
