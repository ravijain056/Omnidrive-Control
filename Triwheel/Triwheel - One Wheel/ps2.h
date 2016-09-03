#ifndef ps2
#define ps2

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_ssi.h"
#include "inc/hw_types.h"
#include "driverlib/ssi.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"

void PS2init(void);
void PS2read(void);
extern volatile uint32_t data_array[21];

void SPIinit(void);
uint32_t SPICharPut(char data);
void SPIArrayPut(char *arr, char cmd_length);

#endif
