#include "main.h"
#include "stdio.h"
#include "string.h"

// --- Base register values ---
#define APB2_BASE 0x40012000
#define GPIOA_BASE 0x40020000

// --- ADC Timer ---
#define RCC_ADC3 ((volatile uint32_t *) (0x40023800+0x44))
#define ADC3_BASE ((volatile uint32_t *) (APB2_BASE+0x200))

// --- ADC Input pin timers ---
#define RCC_GPIOA ((volatile uint32_t *) (0x40023800+0x30))
#define GPIOA_MODER ((volatile uint32_t *) (GPIOA_BASE+0x00))

#define RCC_AHB1ENR   ((volatile uint32_t *)(0x40023800 + 0x30)) // GPIO Clocks
#define RCC_APB1ENR   ((volatile uint32_t *)(0x40023800 + 0x40)) // UART Clocks

// --- ADC REGISTERS ---
#define ADC3_SQR1 ((volatile uint32_t *) (APB2_BASE+0x200+0x2C)) //ADC3 offset is 0x200 and sqr1 offset is 0x2C
#define ADC3_SQR3 ((volatile uint32_t *) (APB2_BASE+0x200+0x34))
#define ADC3_CR2 ((volatile uint32_t *) (APB2_BASE+0x200+0x08))
#define ADC3_SR ((volatile uint32_t *) (APB2_BASE+0x200+0x00))
#define ADC3_DR ((volatile uint32_t *)(APB2_BASE+0x200+0x4C))

// --- SysTick Timer Registers ---
#define SYST_CSR      ((volatile uint32_t *)0xE000E010)
#define SYST_RVR      ((volatile uint32_t *)0xE000E014)
#define SYST_CVR      ((volatile uint32_t *)0xE000E018)

// --- DWT (Cycle Counter) Registers ---
#define DEMCR         ((volatile uint32_t *)0xE000EDFC)
#define DWT_CTRL      ((volatile uint32_t *)0xE0001000)
#define DWT_CYCCNT    ((volatile uint32_t *)0xE0001004)

// --- USART3 Output pins ---
#define GPIOD_BASE    0x40020C00
#define GPIOD_MODER   ((volatile uint32_t *)(GPIOD_BASE + 0x00))
#define GPIOD_AFRH    ((volatile uint32_t *)(GPIOD_BASE + 0x24))

// --- USART3 Registers ---
#define USART3_BASE   0x40004800
#define USART3_CR1    ((volatile uint32_t *)(USART3_BASE + 0x00))
#define USART3_BRR    ((volatile uint32_t *)(USART3_BASE + 0x0C))
#define USART3_ISR    ((volatile uint32_t *)(USART3_BASE + 0x1C))
#define USART3_TDR    ((volatile uint32_t *)(USART3_BASE + 0x28))

#define UART_TXE_FLAG (1 << 7)
#define UART_TC_FLAG  (1 << 6)

void sysTickInit(void){
    *SYST_CSR = 0;
    *SYST_RVR = 15999;
    *SYST_CVR = 0;
    *SYST_CSR = (1<<2)|(1<<0);
}

void Delay_ms(uint32_t ms){
    uint32_t count = 0;
    while (count < ms) {
        if (*SYST_CSR & (1<<16)) count++;
    }
}

void DWT_Init(void) {
    *DEMCR |= (1 << 24);    // Enable debug block
    *DWT_CYCCNT = 0;        // Clear cycle counter
    *DWT_CTRL |= (1 << 0);  // Enable cycle counter
}

void UART_Init(void) {
    // 1. Clocks for GPIOD (Bit 3) and USART3 (Bit 18)
    *RCC_AHB1ENR |= (1 << 3);
    *RCC_APB1ENR |= (1 << 18);

    // 2. Set PD8 and PD9 to Alternate Function (10)
    *GPIOD_MODER &= ~((0x3 << 16) | (0x3 << 18));
    *GPIOD_MODER |= ((0x2 << 16) | (0x2 << 18));

    // 3. Set Alternate Function to AF7 for USART3
    *GPIOD_AFRH &= ~((0xF << 0) | (0xF << 4));
    *GPIOD_AFRH |= ((0x7 << 0) | (0x7 << 4));

    // 4. Baud Rate 115200 (Assuming 16MHz clock)
    *USART3_BRR = 139;

    // 5. Enable USART (UE, TE, RE)
    *USART3_CR1 |= (1 << 3) | (1 << 2) | (1 << 0);
}

void BareMetal_UART_Transmit(const char *pData, uint16_t size) {
    if (pData == NULL || size == 0) return;

    for (uint16_t i = 0; i < size; i++) {
        while ((*USART3_ISR & UART_TXE_FLAG) == 0); // Wait for TXE
        *USART3_TDR = pData[i];                     // Send byte
    }

    while ((*USART3_ISR & UART_TC_FLAG) == 0);      // Wait for TC
}


void ADC_Init(void){
	*RCC_GPIOA |=(1<<0);// GPIOA clock enabled
	*RCC_ADC3 |= (1<<10);//ADC3 clock enabled
	*GPIOA_MODER |= ((1<<0)|(1<<1));//setting input pin to analog mode

	*ADC3_SQR1 &= ~(0xF << 20);// Clearing length bits (L) to 0000 -> 1 conversion

	*ADC3_SQR3 &= ~(0x1F << 0);// Set first conversion in sequence to Channel 0 (PA0)

	*ADC3_CR2 &= ~(0x3<<28);// Clear EXTEN bits and Set control register to ON
	*ADC3_CR2 |= (1<<0);
	Delay_ms(1); //slight delay to wait for ADC values to settle
}

int main(void){
    sysTickInit();
    DWT_Init();
    UART_Init();
	ADC_Init();
	char msgBuffer[100];
    BareMetal_UART_Transmit("System Booted! Reading ADC Values...\r\n", 38);    // Send a boot message

	while(1){
		*ADC3_CR2 |= (1 << 30); // Set SWSTART bit

		while (!(*ADC3_SR & (1 << 1))) {
		        // Wait here until conversion is complete
		    }
		uint16_t raw_adc_value = *ADC3_DR;
		sprintf(msgBuffer, "ADC Value: %u \r\n", raw_adc_value);
		BareMetal_UART_Transmit(msgBuffer, strlen(msgBuffer));
		Delay_ms(1000);
	}
}
