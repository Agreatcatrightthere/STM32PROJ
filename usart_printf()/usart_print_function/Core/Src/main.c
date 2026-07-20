#include <stdint.h>
#include <stdio.h>
#include <string.h>

// ==============================================================================
// 1. REGISTER DEFINITIONS
// ==============================================================================

// --- RCC (Clock) Registers ---
#define RCC_AHB1ENR   ((volatile uint32_t *)(0x40023800 + 0x30)) // GPIO Clocks
#define RCC_APB1ENR   ((volatile uint32_t *)(0x40023800 + 0x40)) // UART Clocks

// --- SysTick Timer Registers ---
#define SYST_CSR      ((volatile uint32_t *)0xE000E010)
#define SYST_RVR      ((volatile uint32_t *)0xE000E014)
#define SYST_CVR      ((volatile uint32_t *)0xE000E018)

// --- DWT (Cycle Counter) Registers ---
#define DEMCR         ((volatile uint32_t *)0xE000EDFC)
#define DWT_CTRL      ((volatile uint32_t *)0xE0001000)
#define DWT_CYCCNT    ((volatile uint32_t *)0xE0001004)

// --- GPIO Registers ---
#define GPIOB_BASE    0x40020400
#define GPIOB_MODER   ((volatile uint32_t *)GPIOB_BASE)
#define GPIOB_BSRR    ((volatile uint32_t *)(GPIOB_BASE + 0x18)) // Bit Set/Reset

#define GPIOC_BASE    0x40020800
#define GPIOC_MODER   ((volatile uint32_t *)GPIOC_BASE)
#define GPIOC_IDR     ((volatile uint32_t *)(GPIOC_BASE + 0x10)) // Input Data

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

// ==============================================================================
// 2. HARDWARE INITIALIZATION FUNCTIONS
// ==============================================================================

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

// ==============================================================================
// 3. BARE-METAL TRANSMIT FUNCTION
// ==============================================================================

void BareMetal_UART_Transmit(const char *pData, uint16_t size) {
    if (pData == NULL || size == 0) return;

    for (uint16_t i = 0; i < size; i++) {
        while ((*USART3_ISR & UART_TXE_FLAG) == 0); // Wait for TXE
        *USART3_TDR = pData[i];                     // Send byte
    }

    while ((*USART3_ISR & UART_TC_FLAG) == 0);      // Wait for TC
}

// ==============================================================================
// 4. MAIN APPLICATION
// ==============================================================================

int main(void) {
    // Initialize Hardware
    sysTickInit();
    DWT_Init();
    UART_Init();

    // Enable Clocks for GPIOB (Bit 1) and GPIOC (Bit 2)
    *RCC_AHB1ENR |= 0x02;
    *RCC_AHB1ENR |= 0x04;

    // Configure LEDs (PB0, PB7, PB14) as Outputs
    *GPIOB_MODER &= ~(0x03);
    *GPIOB_MODER |= (0x01);
    *GPIOB_MODER &= 0xFFFF3FFF;
    *GPIOB_MODER |= (1<<14);
    *GPIOB_MODER &= 0xCFFFFFFF;
    *GPIOB_MODER |= (1<<28);

    // Configure User Button (PC13) as Input
    *GPIOC_MODER &= 0xF3FFFFFF;

    // State Variables
    uint8_t flag1 = 0;
    uint8_t flag2 = 0;
    uint8_t flag3 = 0;
    uint32_t lastbuttonstate = 0x00;
    uint32_t currbuttonstate = 0x00;

    // Buffer for formatting our UART text
    char msgBuffer[100];

    // Send a boot message
    BareMetal_UART_Transmit("System Booted! Press the button...\r\n", 36);

    while(1) {
        // Isolate Pin 13
        currbuttonstate = *GPIOC_IDR & (1 << 13);

        // Edge Detection
        if (currbuttonstate && (lastbuttonstate == 0)) {

            Delay_ms(100); // Debounce

            // ----------------------------------------------------
            // BENCHMARK START
            uint32_t start_cycles = *DWT_CYCCNT;

            // Your Ripple Counter Logic
            if (flag1) {
                *GPIOB_BSRR = 0x40000000;
                flag1--;
                if (flag2) {
                    *GPIOB_BSRR = 0x00800000;
                    flag2--;
                    if (flag3) {
                        *GPIOB_BSRR = 0x00010000;
                        flag3--;
                    } else {
                        *GPIOB_BSRR = 0x00000001;
                        flag3++;
                    }
                } else {
                    *GPIOB_BSRR = 0x00000080;
                    flag2++;
                }
            } else {
                *GPIOB_BSRR = 0x00004000;
                flag1++;
            }

            uint32_t end_cycles = *DWT_CYCCNT;
            uint32_t elapsed_cycles = end_cycles - start_cycles;
            // BENCHMARK END
            // ----------------------------------------------------

            // Format the string into the buffer, then transmit it!
            sprintf(msgBuffer, "Logic Time: %lu CPU Cycles\r\n", elapsed_cycles);
            BareMetal_UART_Transmit(msgBuffer, strlen(msgBuffer));
        }

        lastbuttonstate = currbuttonstate;
    }
}
