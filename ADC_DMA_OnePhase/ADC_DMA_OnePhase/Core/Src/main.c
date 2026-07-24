#include "main.h"
#include "stdio.h"
#include "string.h"
#include "arm_math.h"

// --- All Base Registers ---

#define RCC_Base 0x40023800
#define PWR_Base 0x40007000
#define Flash_Base 0x40023C00 //Flash Interface Register Base
#define SysTick_Base 0xE000E010
#define USART3_Base 0x40004800
#define GPIOD_Base 0x40020C00
#define TIM2_Base 0x40000000
#define DMA2_Base 0x40026400
#define ADC1_Base 0x40012000
#define ADC2_Base 0x40012100
#define GPIOA_Base 0x40020000
#define ADC_COMMON_Base 0x40012300
#define GPIOB_Base 0x40020400


// --- High Power Mode setup for 216MHz Operations ---

#define RCC_APB1ENR ((volatile uint32_t *)(RCC_Base + 0x40))
#define PWR_CR1 ((volatile uint32_t *)(PWR_Base + 0x00))
#define PWR_CSR1 ((volatile uint32_t *)(PWR_Base + 0x04))


// --- 216MHz PLL Clock setup---

#define RCC_CR ((volatile uint32_t *)(RCC_Base + 0x00)) //base address for RCC Clock Register
#define RCC_PLLCFGR ((volatile uint32_t *)(RCC_Base + 0x04))//PLL Configuration Register
#define RCC_CFGR ((volatile uint32_t *)(RCC_Base + 0x08))//Clock configuration register


// --- Flash memory register ---

#define FLASH_ACR ((volatile uint32_t *)(Flash_Base + 0x00))


// --- SysTick Configuration registers ---

#define SYST_CSR ((volatile uint32_t *)(SysTick_Base + 0x00))
#define SYST_RVR ((volatile uint32_t *)(SysTick_Base + 0x04))
#define SYST_CVR ((volatile uint32_t *)(SysTick_Base + 0x08))


// --- Cycle Counter (DWT) Registers ---

#define DEMCR ((volatile uint32_t *)0xE000EDFC)
#define DWT_CTRL ((volatile uint32_t *)0xE0001000)
#define DWT_CYCCNT ((volatile uint32_t *)0xE0001004)
#define DWT_LAR ((volatile uint32_t *)0xE0001FB0) // need an access key at this location to use the DWT register


// --- USART3 Registers ---

//#define RCC_APB1ENR ((volatile uint32_t *)(RCC_Base + 0x40)) // Already defined earlier
#define USART3_CR1 ((volatile uint32_t *)(USART3_BASE + 0x00))
#define USART3_BRR ((volatile uint32_t *)(USART3_BASE + 0x0C))
#define USART3_ISR ((volatile uint32_t *)(USART3_BASE + 0x1C))
#define USART3_TDR ((volatile uint32_t *)(USART3_BASE + 0x28))


// ---GPIOA Registers for ADC ---

//#define RCC_APB1ENR ((volatile uint32_t *)(RCC_BASE + 0x40)) // Already defined earlier
#define RCC_AHB1ENR ((volatile uint32_t *)(RCC_BASE + 0x30))
#define GPIOA_MODER ((volatile uint32_t *)(GPIOA_BASE + 0x00))
#define GPIOA_AFRH ((volatile uint32_t *)(GPIOA_BASE + 0x24))


// --- GPIOB for LED ---
//#define RCC_APB1ENR ((volatile uint32_t *)(RCC_BASE + 0x40)) // Already defined earlier
//#define RCC_AHB1ENR ((volatile uint32_t *)(RCC_BASE + 0x30)) //Already defined earlier
#define GPIOB_MODER ((volatile uint32_t *)(GPIOB_Base + 0x00))
#define GPIOB_ODR ((volatile uint32_t *)(GPIOB_Base + 0x14))
#define GPIOB_BSRR ((volatile uint32_t *)(GPIOB_Base + 0x18))

// ---GPIOD Registers for UART ---

//#define RCC_APB1ENR ((volatile uint32_t *)(RCC_BASE + 0x40)) // Already defined earlier
//#define RCC_AHB1ENR ((volatile uint32_t *)(RCC_BASE + 0x30)) // Already defined earlier
#define GPIOD_MODER ((volatile uint32_t *)(GPIOD_BASE + 0x00))
#define GPIOD_AFRH ((volatile uint32_t *)(GPIOD_BASE + 0x24))


// --- TIM2 Setup Registers ---

//#define RCC_APB1ENR   ((volatile uint32_t *)(RCC_BASE + 0x40)) // Already defined earlier
#define TIM2_CR1 ((volatile uint32_t *)(TIM2_Base + 0x00))
#define TIM2_CR2 ((volatile uint32_t *)(TIM2_Base + 0x04))
#define TIM2_PSC  ((volatile uint32_t *)(TIM2_Base + 0x28))
#define TIM2_ARR ((volatile uint32_t *)(TIM2_Base + 0x2C))
#define TIM2_EGR ((volatile uint32_t *)(TIM2_Base + 0x14))
#define TIM2_SR ((volatile uint32_t *)(TIM2_Base + 0x10))


// --- DMA setup Registers ---

//#define RCC_AHB1ENR   ((volatile uint32_t *)(RCC_BASE + 0x30)) //Already defined earlier
//ADC1 offset for stream 0 = 0x18 * 0
#define DMA2_S0CR ((volatile uint32_t *)(DMA2_Base + 0x010)) // ADC 1 can only be configured to stream 0 or 4
#define DMA2_S0PAR ((volatile uint32_t *)(DMA2_Base + 0x018))
#define DMA2_S0M0AR ((volatile uint32_t *)(DMA2_Base + 0x01C))
#define DMA2_S0NDTR ((volatile uint32_t *)(DMA2_Base + 0x014))
//ADC2 offset for stream 2 = 0x18 * 2 = 0x30
#define DMA2_S2CR ((volatile uint32_t *)(DMA2_Base + 0x010 + 0x30)) // ADC 2 can only be configured to stream 2 or 3
#define DMA2_S2PAR ((volatile uint32_t *)(DMA2_Base + 0x018 + 0x30))
#define DMA2_S2M0AR ((volatile uint32_t *)(DMA2_Base + 0x01C + 0x30))
#define DMA2_S2NDTR ((volatile uint32_t *)(DMA2_Base + 0x014 + 0x30))


// --- ADC Registers ---
#define RCC_APB2ENR ((volatile uint32_t *)(RCC_BASE + 0x44))
//common register
#define ADC_CCR ((volatile uint32_t *)(ADC_COMMON_Base + 0x04))
//ADC1
#define ADC1_DR ((volatile uint32_t *)(ADC1_Base + 0x4C))
#define ADC1_CR1 ((volatile uint32_t *)(ADC1_Base + 0x04))
#define ADC1_CR2 ((volatile uint32_t *)(ADC1_BASE + 0x08))
#define ADC1_SMPR2 ((volatile uint32_t *)(ADC1_BASE + 0x10))
#define ADC1_SQR1 ((volatile uint32_t *)(ADC1_BASE + 0x2C))
#define ADC1_SQR3 ((volatile uint32_t *)(ADC1_BASE + 0x34))
//ADC2
#define ADC2_DR ((volatile uint32_t *)(ADC2_Base + 0x4C))
#define ADC2_CR1 ((volatile uint32_t *)(ADC2_BASE + 0x04))
#define ADC2_CR2 ((volatile uint32_t *)(ADC2_BASE + 0x08))
#define ADC2_SMPR2 ((volatile uint32_t *)(ADC2_BASE + 0x10))
#define ADC2_SQR1 ((volatile uint32_t *)(ADC2_BASE + 0x2C))
#define ADC2_SQR3 ((volatile uint32_t *)(ADC2_BASE + 0x34))


// --- Hardware FPU registers ---

#define SCB_CPACR (*(volatile uint32_t *)0xE000ED88)


// --- Buffers ---

volatile uint16_t voltage_buffer[3]; //V_a, V_b, V_c
volatile uint16_t current_buffer[3]; //I_a, I_b, I_c


// --- CMSIS-DSP Biquad Globals ---
arm_biquad_casd_df1_inst_f32 LPF_instance;
float32_t LPF_state[4] = {0.0f, 0.0f, 0.0f, 0.0f};


// 2nd-Order Butterworth LPF (Fs = 50 kHz, Fc = 10 Hz)
// a1 and a2 signs are intentionally inverted for CMSIS-DSP compatibility
float32_t LPF_coeffs[5] = {
    0.000000394f,  // b0
    0.000000789f,  // b1
    0.000000394f,  // b2
    1.998220000f,  // a1
   -0.998220000f   // a2
};


// --- constants ---
float32_t p_bar_out = 0.0f;
volatile uint32_t execution_cycles = 0;
volatile float32_t execution_time_us = 0.0f;


//--- hardware initialisation functions ---

void HighSpeedClkInit(void){
	//Step 0: Ensure we are running on HSI for configuration, post which we will switch to PLL
	*RCC_CR |= (1<<0);
	while (!(*RCC_CR & (1<<1))); //Wait for HSIRDY
	*RCC_CFGR &= ~(0x3<<0);
	while((*RCC_CFGR & (0x3 << 2)) != 0); //Wait for System to configure to HSI
	*RCC_CR &= ~(1 << 24);
	while(*RCC_CR & (1 << 25));//explicitly switching PLL off

	//Step 1: setting up high power mode
	*RCC_APB1ENR |= (1<<28); // Enable Clock for Power Registers
	(void)*RCC_APB1ENR;
	*PWR_CR1 |= (3 << 14); // Set voltage scale 1
	*PWR_CR1 |= (1<<16);
	while (!(*PWR_CSR1 & (1<<16)));//Set OverDrive Enable to 1 and waiting for Status to go to 1
	*PWR_CR1 |= (1<<17);
	while (!(*PWR_CSR1 & (1<<17))); //Set OverDrive Switch to 1 and waiting for Status to go to 1

	//Step 2: Configure PLL and HSE Registers
	*RCC_CR |= (1 << 16);
	while (!(*RCC_CR & (1 << 17))); // Wait for HSERDY
	*RCC_PLLCFGR = (4 << 0) | (216 << 6) | (0 << 16) | (1 << 22) | (9 << 24); //PLL configuration
	*RCC_CR |= (1 << 24);
	while (!(*RCC_CR & (1 << 25)));

	//Step 3: Configure Flash, AHB and APB settings and switch the main system clock to PLL
	*FLASH_ACR = (7 << 0) | (1 << 8) | (1 << 9); // 7 wait states, ART accelerator enabled, pre-fetch enabled
	*RCC_CFGR |= (5 << 10) | (4 << 13); // AHB=/1, APB1=/4, APB2=/2
	*RCC_CFGR |= (2 << 0);
	while ((*RCC_CFGR & 0x0C) != 0x08); // Wait for PLL is active signal
}


void FilterInit(void) {
    // Link the biquad memory and coefficients for the DSP block
    arm_biquad_cascade_df1_init_f32(&LPF_instance, 1, LPF_coeffs, LPF_state);
}


void SysTickInit(void){
	*SYST_CSR = 0; // setting to 0 so we know everything starts from scratch at initialization
	*SYST_RVR = 215999;
	*SYST_CVR = 0;
	*SYST_CSR = (1 << 2) | (1 << 0); //setting source to processor and enable bit to 1
}


void DWTInit(void) {
    *DEMCR |= (1 << 24);    // Enable debug block

    *DWT_LAR = 0xC5ACCE55;

    *DWT_CYCCNT = 0;        // Clear cycle counter
    *DWT_CTRL |= (1 << 0);  // Enable cycle counter
}


void UARTInit(void){
	//Step 1: enabling USART3 and GPIOD Clocks
	*RCC_APB1ENR |= (1<<18);
	(void)*RCC_APB1ENR;
	*RCC_AHB1ENR |= (1<<3);
	(void)*RCC_AHB1ENR;
	//Step 2: Set GPIOD to alternate Mode and then set the pins to PD8 AND PD9
	*GPIOD_MODER &= ~((3<<16) | (3<<18));
	*GPIOD_MODER |= (2<<16) | (2<<18);
	*GPIOD_AFRH &= ~((0xF<<0) | (0xF<<4));
	*GPIOD_AFRH |= (7<<0) | (7<<4);

	//Step 3: Setting Baud Rate and configuring USART3 Register
	*USART3_BRR = 469; // 54,000,000/115200 = 468.75 ~ 469
	*USART3_CR1 = (1 << 0) | (1 << 2) | (1 << 3); // RX EN ,TX EN,USART EN bits turned on

}


void TIM2Init(void){
	*RCC_APB1ENR |= (1<<0); // enable TIM2 clock
	(void)*RCC_APB1ENR;
	*TIM2_PSC = 0;
	*TIM2_ARR = 2159;
	*TIM2_CR2 &= ~(7 << 4);    // Clear bits 4, 5, 6
	*TIM2_CR2 |=  (2 << 4);
	*TIM2_EGR |= (1 << 0);

}


void DMAInit(void){
	*RCC_AHB1ENR |= (1<<22); // clock enable
	(void)*RCC_AHB1ENR;


	// ADC 1 CONFIG
	*DMA2_S0CR &= ~(1 << 0);// disabling the DMA before configuring it
	while (*DMA2_S0CR & (1 << 0)); // waiting for DMA to be disabled, status and enable bit are the same
	*DMA2_S0PAR = (uint32_t) ADC1_DR; // setting up peripheral register address to point to data from ADC1
	*DMA2_S0M0AR = (uint32_t) voltage_buffer; // setting up memory register address to point to our array
	*DMA2_S0NDTR = 0x3; // sending 3 datapoints every time to the memory
	*DMA2_S0CR = 0x00032D00; // channel 0, burst mode off, double buffer mode off, priority very high, peripheral and memory data size 16 bits, memory increment mode on and circular data mode on
	*DMA2_S0CR |= (1 << 0); // enable register

	// ADC2 CONFIG
	*DMA2_S2CR &= ~(1 << 0);
	while (*DMA2_S2CR & (1 << 0));
	*DMA2_S2PAR = (uint32_t) ADC2_DR;
	*DMA2_S2M0AR = (uint32_t) current_buffer;
	*DMA2_S2NDTR = 0x3;
	*DMA2_S2CR = 0x02032D00; // channel 1
	*DMA2_S2CR |= (1 << 0);
}


void LEDInit(void){
	*RCC_AHB1ENR |= (1<<1);//set clock
	(void)*RCC_AHB1ENR;
	*GPIOB_MODER &= ~(3<<0);
	*GPIOB_MODER |= (1<<0);//set PB0 pin as output

}


void ADCInit(void){
	*RCC_APB2ENR |= (0x3<<8); // enable clock
	(void)*RCC_APB2ENR;

	//initialise GPIOA Ports to analog mode
	*RCC_AHB1ENR |= (1<<0);
	(void)*RCC_AHB1ENR;

	*GPIOA_MODER |= 0x3FFF;

	// Setting common clock to 27MHz
	*ADC_CCR &= ~(3 << 16);
	*ADC_CCR |=  (1 << 16); // enable PCLK/4 to ensure we stay below the 36MHz ADC limit (27MHz)

	//ADC1 Config
	*ADC1_CR1 = (1 << 8); // scan mode
	*ADC1_SQR1 = (2 << 20); // 3 conversions
	*ADC1_SQR3 &= ~(0x7FFF);// pins PA0 , (~P~A~1~) PA6 and (~P~A~2~) PA7 used
	*ADC1_SQR3 |= (6 << 5) | (7 << 10);
	*ADC1_SMPR2 = (1 << 0) | (1 << 18) | (1 << 21);//15 cycles per channel (due to 12 bit resolution)
	*ADC1_CR2 = (1 << 8) | (1 << 9) | (11 << 24) | (1 << 28);
	*ADC1_CR2 |= (1 << 0);

	//ADC2 Config
	*ADC2_CR1 = (1 << 8);
	*ADC2_SQR1 = (2 << 20);
	*ADC2_SQR3 = (3 << 0) | (4 << 5) | (5 << 10);
	*ADC2_SMPR2 = (1 << 9) | (1 << 12) | (1 << 15);
	*ADC2_CR2 = (1 << 8) | (1 << 9) | (11 << 24) | (1 << 28);
	*ADC2_CR2 |= (1 << 0);


}


void FPUInit(void) {
    SCB_CPACR |= (0xF << 20); // Grant Full Access to Coprocessors 10 and 11
    __asm("dsb");
    __asm("isb");
}




// --- control functions ---


void Start_Dual_ADCs(void) {
    // Both ADCs are now waiting for the TIM2 pulse.
    // We prime both machines with the SWSTART bit.
    *ADC1_CR2 |= (1 << 30);
    *ADC2_CR2 |= (1 << 30);
}


void Start_TIM2(void){
	*TIM2_CR1 |= (1 << 0);
}


void UART_Transmit(const char *pData, uint16_t size) {
    if (pData == NULL || size == 0) return;

    for (uint16_t i = 0; i < size; i++) {
        while ((*USART3_ISR & (1<<7)) == 0); // Wait for TXE
        *USART3_TDR = pData[i];                     // Send byte
    }

    while ((*USART3_ISR & (1<<6)) == 0);      // Wait for TC
}


void Delayms(int ms){
	uint32_t count =0;
	*SYST_CVR = 0; //reset system tick counter to ensure we start from 0
	while (count <ms){
		if (*SYST_CSR & (1<<16)) count++; //checking if flag bit is enabled also resets it if it is enabled
	}
}


// --- main ---

int main(void){
	//core hardware config
	HighSpeedClkInit();
	FPUInit();
	FilterInit();
	SysTickInit();
	DWTInit();

	//peripheral config
	LEDInit();
	UARTInit();
	ADCInit();
	DMAInit();
	TIM2Init();


	Delayms(100); // wait for initialisations to settle down

	//actual main starts here!!

	Start_Dual_ADCs();
	Start_TIM2();

	//sending a Boot Message
	UART_Transmit("System Booted!\r\n", 16);

	uint8_t fault_triggered = 0;
	uint32_t positive_cycle_count = 0; // for turning off LED, we need 10 consecutive positive P_BAR cycles
	while(1){
		uint32_t start_cycles = *DWT_CYCCNT;

		// track voltage and current values from the memory
		//NOTE: 0.05f and 0.02f are placeholders that will be changed to whatever conversion factor is applied to the voltage and current before they're fed to the microcontroller
		float32_t va = ((float32_t)voltage_buffer[0] - 2048.0f);
		float32_t ia = ((float32_t)current_buffer[0] - 2048.0f);

		float32_t vb = ((float32_t)voltage_buffer[1] - 2048.0f);
		float32_t ib = ((float32_t)current_buffer[1] - 2048.0f);

		float32_t vc = ((float32_t)voltage_buffer[2] - 2048.0f);
		float32_t ic = ((float32_t)current_buffer[2] - 2048.0f);

		// fpu math
		float32_t p_inst = (va * ia) + (vb * ib) + (vc * ic);

		p_bar_out = p_inst;
		//arm_biquad_cascade_df1_f32(&LPF_instance, &p_inst, &p_bar_out, 1);

		//float32_t p_tilde = p_inst - p_bar_out;

		// start of error detection code
		if (p_bar_out < 0.0f) {

		            // Fault state: Reset the recovery counter immediately.
		            positive_cycle_count = 0;

		            // Trigger the fault if it isn't already active
		            if (fault_triggered == 0) {
		                *GPIOB_BSRR = (1 << 0); // Fast Pin Set (Turn ON LED / Relay)

		                // Snapshot time taken specifically for the math and DSP filter
		                uint32_t current_cycles = *DWT_CYCCNT - start_cycles;
		                uint32_t current_time_us = current_cycles /216; // Integer division for %lu

		                // Format the message into a local buffer
		                char msg_buf[128];
		                uint16_t msg_len = snprintf(msg_buf, sizeof(msg_buf),
		                                            "CRITICAL: Reverse P_bar Detected!\r\nTook: %lu microseconds\r\n",
		                                            current_time_us);

		                UART_Transmit(msg_buf, msg_len);
		                fault_triggered = 1;
		            }

		        }
		else {

			// P_bar is positive. Check if we are trying to recover from a fault.
			if (fault_triggered == 1) {
				if(*TIM2_SR & (1 << 0)){
					*TIM2_SR &= ~(1 << 0); // Clear the update interrupt flag
					positive_cycle_count++;// Log a successful healthy cycle
				}

				// Only release the fault if we hit 10 consecutive healthy cycles
				if (positive_cycle_count >= 10) {
					*GPIOB_BSRR = (1 << 16); // Fast Pin Reset (Turn OFF LED / Relay)

					// Snapshot time taken for recovery
					uint32_t current_cycles = *DWT_CYCCNT - start_cycles;
					uint32_t current_time_us = current_cycles / 216;

					// Format the message into a local buffer
					char msg_buf[128];
					uint16_t msg_len = snprintf(msg_buf, sizeof(msg_buf),
												"SYSTEM RECOVERED: P_bar is positive.\r\nTook: %lu microseconds\r\n",
												current_time_us);

					UART_Transmit(msg_buf, msg_len);

					fault_triggered = 0;      // Clear the fault state
					positive_cycle_count = 0; // Reset counter for the next event
				}
			}
			else {
				// System is operating normally. Keep the counter zeroed out.
				positive_cycle_count = 0;
			}
		}
	}
}
