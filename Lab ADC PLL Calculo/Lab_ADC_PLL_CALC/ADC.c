#include <stdint.h>
#include "ADC.h"
#include "tm4c1294ncpdt.h"					

void ADC0_InitTimer0ATrigger(){
		volatile uint32_t delay;
		// **** GPIO pin initialization ****
		SYSCTL->RCGCGPIO |= (1UL << 4);                                      
		while((SYSCTL->PRGPIO & (1UL << 4)) == 0){};	//     allow time for clock to stabilize

		GPIOE_AHB->AFSEL |= 0x08;     // enable alternate function on PE3
		GPIOE_AHB->DEN &= ~0x08;      // disable digital I/O on PE3
		GPIOE_AHB->AMSEL |= 0x08;     // enable analog functionality on PE3

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
		ADC0->SSCTL3 = 0x0004;           														// 0x04 = Sample interrupt enable (IE0), 0x0C =  IE0 e Temp sensor select
		ADC0->IM |= 0x00000008;          														// enable SS3 interrupts
		ADC0->CC = ((ADC0->CC &~ 0x000003F0)+(14<<4)) |
               ((ADC0->CC &~ 0x0000000F)+0);

		ADC0->ACTSS = 0x00000008;       														// enable sample sequencer 3
		ADC0->EMUX = (ADC0->EMUX &~ 0x0000F000)+0x00005000;					// configure seq3 for timer trigger
			
		/* Enable interrupt */
		NVIC_EnableIRQ(ADC0SS3_IRQn);	
}

