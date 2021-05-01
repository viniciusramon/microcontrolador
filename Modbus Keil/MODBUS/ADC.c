#include <stdint.h>
#include "ADC.h"
#include "tm4c1294ncpdt.h"					

void ADC0_InitTimer0ATrigger(){
		volatile uint32_t delay;
		// **** GPIO pin initialization ****
		SYSCTL->RCGCGPIO |= (1UL << 4);                                      
		while((SYSCTL->PRGPIO & (1UL << 4)) == 0){};	//     allow time for clock to stabilize

//		GPIOE_AHB->AFSEL |= 0x08;     // enable alternate function on PE3
//		GPIOE_AHB->DEN &= ~0x08;      // disable digital I/O on PE3
//		GPIOE_AHB->AMSEL |= 0x08;     // enable analog functionality on PE3

		GPIOE_AHB->AFSEL |= 0x04;     // enable alternate function on PE2
		GPIOE_AHB->DEN &= ~0x04;      // disable digital I/O on PE2
		GPIOE_AHB->AMSEL |= 0x04;     // enable analog functionality on PE2
			
		// **** general initialization ****
    SYSCTL->RCGCTIMER |= 0x01;     									// activate TIMER0
		while((SYSCTL->PRTIMER & 0x01) == 0){};					// allow time for clock to stabilize

		TIMER0->CTL		= 0x00000000;      // disable TIMER0A during setup
		TIMER0->CTL  |= 0x00000020;      // enable timer0A trigger to ADC
		TIMER0->ADCEV |= 0x00000001;		 // timer0A time-out event ADC trigger enabled
		TIMER0->CFG   = 0x00000000;      // configure for 32-bit mode
		TIMER0->TAMR  = 0x00000002;      // configure for periodic mode, default down-count settings
		TIMER0->TAILR = (16000000/10)-1; // reload value ( 100 ms )
		TIMER0->TAPR  = 0;               // bus clock resolution
		TIMER0->ICR   = 0x00000001;      // clear TIMER0A timeout flag
		TIMER0->IMR   = 0x00000000;      // disable arm timeout interrupt
		TIMER0->CTL |= 0x00000001;    	 // enable TIMER0A
                                    
		// **** ADC initialization **** //                                      
		SYSCTL->RCGCADC |= 0x00000001;															// activate clock for ADC0
		while((SYSCTL->PRADC & 0x00000001) == 0){}; 								// allow time for clock to stabilize

		ADC0->ACTSS = 0x00000000;   																// disable sample sequencer 3
		// 2 AI, first for E3 and sencond for temperature sensor	
		ADC0->SSCTL2 = 0x000000E4;           												// TS1 = 1; IE1 = 1; END1 = 1; IE0 = 1;
		ADC0->IM |= 0x00000004;          														// enable SS2 interrupts
		ADC0->EMUX = (ADC0->EMUX &~ 0x00000000)+0x00000500;					// configure seq2 for timer trigger
		ADC0->SSMUX2 = 0x00000001;																	// PE2
		ADC0->SSTSH2 = 0x00000040;
		ADC0->CC = 0xE1;																						// PIOSC / 0x0E

		ADC0->ACTSS = 0x00000004;       														// enable sample sequencer 2
			
		/* Enable interrupt */
		NVIC_EnableIRQ(ADC0SS2_IRQn);	
}

