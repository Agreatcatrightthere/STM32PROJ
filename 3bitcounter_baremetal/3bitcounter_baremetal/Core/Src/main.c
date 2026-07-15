#include <stdint.h>
#include <stdio.h>


#define SYST_CSR ((volatile uint32_t *)0xE000E010)
#define SYST_RVR ((volatile uint32_t *)0xE000E014)
#define SYST_CVR ((volatile uint32_t *)0xE000E018)


#define RCCGPIOAHB1 ((volatile uint32_t *) (0x40023800 + 0x30)) // both port B and C are connected to AHB1 bus

#define GPIOBbase 0x40020400
#define GPIOBMode ((volatile uint32_t *)GPIOBbase)
#define GPIOBState ((volatile uint32_t *)(GPIOBbase + 0x18))

#define GPIOCbase 0x40020800
#define GPIOCMode ((volatile uint32_t *)GPIOCbase)
#define GPIOCState ((volatile uint32_t *)(GPIOCbase + 0x10))

void sysTickInit(void){
	*SYST_CSR = 0; // initializes the control and status register to 0 to ensure it starts off exactly as we start our code
	*SYST_RVR = 15999; // 16MHz clock is assumed so 1 millisecond passes in counting up to 16000
	*SYST_CVR = 0; //current count register
	*SYST_CSR = (1<<2)|(1<<0); // start counting
}

void Delay_ms (uint32_t ms){
	uint32_t count =0;
	while (count <ms) {
		if (*SYST_CSR & (1<<16)) count++;
	}
}

int main(void){
	// initializing system clock
	sysTickInit();
	// switching on the clock for GPIOB and C
	*RCCGPIOAHB1 |= 0x02;
	*RCCGPIOAHB1 |= 0x04;

	// setting LED PORT GPIOB to output mode

	*GPIOBMode &= ~(0X03); // LD1 AT PB0
	*GPIOBMode |= (0x01);

	*GPIOBMode &= 0XFFFF3FFF; //LD2 AT PB7
	*GPIOBMode |= (1<<14);

	*GPIOBMode &= 0XCFFFFFFF; //LD3 AT PB14
	*GPIOBMode |= (1<<28);

	// setting Button PORT GPIOC to input mode
	*GPIOCMode &= 0XF3FFFFFF; //USERBUTTON AT PC13

	uint8_t flag1 =0;
	uint8_t flag2 =0;
    uint8_t flag3 =0;
	uint32_t lastbuttonstate = 0x00;
	uint32_t currbuttonstate = 0x00;

	while(1){
		currbuttonstate = *GPIOCState & (1 << 13);

		if (currbuttonstate && (lastbuttonstate ==0)){
			Delay_ms(50);
			if(*GPIOCState & (1 << 13))
				if (flag1){
					*GPIOBState = 0X40000000;
					flag1--;
					if (flag2){
						*GPIOBState = 0X00800000;
						flag2--;
						if (flag3){
							*GPIOBState = 0X00010000;
							flag3--;
						}
						else{
							*GPIOBState = 0X00000001;
							flag3++;
						}
					}
					else {
						*GPIOBState = 0X00000080;
						flag2++;
					}
				}
				else {
					*GPIOBState = 0X00004000;
					flag1++;
				}

		}
		lastbuttonstate = currbuttonstate;
	}
}
