/*
 * ps2.c
 *
 *  Created on: Jun 11, 2016
 *      Author: Aniket
 */

#include "ps2.h"

char PS2_POLLBUTTON[] = {0x01, 0x42, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00};

// Bit-wise reverses a number.
uint8_t Reverse(uint8_t ui8Number) {
	uint8_t ui8Index;
	uint8_t ui8ReversedNumber = 0;
	for(ui8Index = 0; ui8Index < 8; ui8Index++)
	{
		ui8ReversedNumber = ui8ReversedNumber << 1;
		ui8ReversedNumber |= ((1 << ui8Index) & ui8Number) >> ui8Index;
	}
	return ui8ReversedNumber;
}

void SPIinit(void) {
	SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	GPIOPinConfigure(GPIO_PA2_SSI0CLK);
	GPIOPinConfigure(GPIO_PA3_SSI0FSS);
	GPIOPinConfigure(GPIO_PA4_SSI0RX);
	GPIOPinConfigure(GPIO_PA5_SSI0TX);
	GPIOPinTypeSSI(GPIO_PORTA_BASE,GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5);
	SSIConfigSetExpClk(SSI0_BASE,SysCtlClockGet(),SSI_FRF_MOTO_MODE_3,SSI_MODE_MASTER,500000,8);
	SSIEnable(SSI0_BASE);
}

uint32_t SPICharPut(char data) {
	uint32_t received;
	SSIDataPut(SSI0_BASE,Reverse(data));
	while(SSIBusy(SSI0_BASE));
	SysCtlDelay(200000);
	SSIDataGet(SSI0_BASE,&received);
	return received;
}

void SPIArrayPut(char *arr, char cmd_length) {
	uint8_t i;
	for(i = 0; i < cmd_length; i++) {
		data_array[i] = SPICharPut(*arr);
		arr++;
	}
}

void PS2init(void) {
	SPIinit();
	// PS2 initialization
	char  PS2_CONFIGMODE[5]= {0x01, 0x43, 0x00, 0x01, 0x00};
	char  PS2_ANALOGMODE[9]= {0x01, 0x44, 0x00, 0x01, 0x03, 0x00, 0x00, 0x00, 0x00};
	char  PS2_SETUPMOTOR[9]= {0x01, 0x4D, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff};
	//	char  PS2_RETURNPRES[9]= {0x01, 0x4f, 0x00, 0xFF, 0xFF, 0x03, 0x00, 0x00, 0x00};
	char  PS2_EXITCONFIG[9]= {0x01, 0x43, 0x00, 0x00, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a};
	SPIArrayPut(PS2_CONFIGMODE,5);
	SPIArrayPut(PS2_ANALOGMODE,9);
	SPIArrayPut(PS2_SETUPMOTOR,9);
	SPIArrayPut(PS2_EXITCONFIG,9);
}

void PS2read(void) {
	SPIArrayPut(PS2_POLLBUTTON,9);
}
