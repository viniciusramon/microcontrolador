/************************************************************/
/*	LCD & Teclado 																					*/
/*																													*/
/************************************************************/

/************************	INCLUDES **************************/
#include <TM4C129.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "ADC.h"

/************************	DEFINES ***************************/
#define PN0				(1UL << 0)
#define PN1				(1UL << 1)
#define PF0				(1UL << 0)

/***********************	  OUTROS  *************************/
#define PJ0				(1ul << 0)
#define PJ1				(1ul << 1)

#define	InPJ0			(GPIOJ_AHB->DATA) & (1ul << 0)
#define	PortN			GPION->DATA
#define	PortF			GPIOF_AHB->DATA

#define	MAX_NR_TIMERS		2

#define	F_VOLTAGE	  		1

#define FXTAL 25000000  // fixed, this crystal is soldered to the Connected Launchpad
#define Q            0
#define N            4  // chosen for reference frequency within 4 to 30 MHz
#define MINT        96  // 480,000,000 = (25,000,000/(0 + 1)/(4 + 1))*(96 + (0/1,024))
#define MFRAC        0  // zero to reduce jitter
#define PSYSDIV 		 29
#define SYSCLK (FXTAL/(Q+1)/(N+1))*(MINT+MFRAC/1024)/(PSYSDIV+1)

/************************	ESTRUTURAS ************************/


/********************	 VARIAVEIS GLOBAIS ********************/
int									Valor;
int 								msTicks;			                 // systick counter      
long								TimerDownCnt[MAX_NR_TIMERS];
char								init;
char								NovoValor;
long								Voltage;
int									AVoltage;
long								Counter;
char								refresh;
char								SerialData;

/********************	 Predefined functions ********************/
void MainTask(void);

/************************	  FUNCOES   ***********************/
/*----------------------------------------------------------------------------
    SysTick_Handler - 1ms
 *----------------------------------------------------------------------------*/
void SysTick_Handler(void) {
		int i;
		msTicks++;
		for(i=0;i<MAX_NR_TIMERS;i++)
		{
				if(TimerDownCnt[i]>0)
				{
						TimerDownCnt[i]--;
				}
		}

		MainTask();
}

void ADC0SS3_Handler(void){
		ADC0->ISC = 0x00000008;             // acknowledge ADC sequence 3 completion
																				// execute user task
		Voltage = ADC0->SSFIFO3 & 0x00000FFF;
		
		Counter++;
}

/*----------------------------------------------------------------------------
  Initialize UART pins, Baudrate
      PA0 U0_RX, PA1 U0_TX, 115200 @ 16MHz, 8 N 1
 *----------------------------------------------------------------------------*/
void SER_Initialize (void) {

  SYSCTL->RCGCGPIO |=   ( 1ul << 0);             /* enable clock for GPIOs    */
  GPIOA_AHB->DEN   |=  (( 1ul << 0) | ( 1ul << 1));
  GPIOA_AHB->AFSEL |=  (( 1ul << 0) | ( 1ul << 1));
  GPIOA_AHB->PCTL  &= ~((15ul << 0) | (15ul << 4));
  GPIOA_AHB->PCTL  |=  (( 1ul << 0) | ( 1ul << 4));

  SYSCTL->RCGCUART |=  (1ul << 0);               /* enable clock for UART0    */
  SYSCTL->SRUART   |=  (1ul << 0);               /* reset UART0 */
  SYSCTL->SRUART   &= ~(1ul << 0);               /* clear reset UART0 */
  while ((SYSCTL->PRUART & (1ul << 0)) == 0);    /* wait until UART0 is ready */

  UART0->CTL  =   0;                             /* disable UART              */
  UART0->IM   =   0x10;					                 /* interrupts  enabled       */
  UART0->IBRD =   8;
  UART0->FBRD =   44;
  UART0->LCRH =   (3ul << 5);                     /* 8 bits                   */
  UART0->CC   =   0;                              /* use system clock         */
  UART0->CTL |=  ((1ul << 9) | (1ul << 8));       /* enable RX, TX            */
  UART0->CTL |=   (1ul << 0);                     /* enable UART              */
	/* Enable interrupt */
	NVIC_EnableIRQ(UART0_IRQn);	
}

/*----------------------------------------------------------------------------
  Write character to Serial Port
 *----------------------------------------------------------------------------*/
int SER_PutChar (int c) {

  while ((UART0->FR & (1ul << 7)) == 0x00);
  UART0->DR = (c & 0xFF);

  return (c);
}


/*----------------------------------------------------------------------------
  Read character from Serial Port   (blocking read)
 *----------------------------------------------------------------------------*/
int SER_GetChar (void) {
  return (UART0->DR & 0xFF);
}

int main()
{
		//*** variaveis locais
		long timeout;
	
		//*** inicializoes
		init = 1;

		/* Enable clock and init GPIO outputs */
		SYSCTL->RCGCGPIO |= (1UL << 0) | (1UL << 1) | (1UL << 9) | (1UL << 12)	| (1UL << 5) | (1UL << 6) | (1UL << 8) | (1UL << 10);  /* enable clock for GPIOs  */

		/* OUTROS */
		GPION->DIR      		 |= PN0 | PN1;   								/* PN1, PN0 is output        */
		GPION->DEN       		 |= PN0 | PN1;   								/* PN1, PN0 enable 				   */

		GPIOJ_AHB->PUR   		 |=  PJ0 | PJ1;  								/* PJ0, PJ1 pull-up          */
		GPIOJ_AHB->DIR   		 &= ~(PJ0 | PJ1);								/* PJ0, PJ1 is intput        */
		GPIOJ_AHB->DEN   		 |=  (PJ0 | PJ1);								/* PJ0, PJ1 is digital func. */

	  /*****************    PLL        *****************/

		// Habilita o MOSC resetando o bit NOXTAL e PWRDN bit.
		SYSCTL->MOSCCTL  &= ~(0x00000004 | 0x00000008);
		while((SYSCTL->RIS & 0x00000100)==0){};
		// USEPLL = 1: liga PLL; OSCSRC = 3:MOSC is the oscillator source ; PLLSRC = 3: MOSC is the PLL input clock
		SYSCTL->RSCLKCFG = 0x13300000 | PSYSDIV;
		SYSCTL->PLLFREQ0 |= ((MFRAC << 10) | (MINT << 0));
		SYSCTL->PLLFREQ1 |= ((Q << 8) |(N << 0));
		SYSCTL->PLLFREQ0 |= 0x00800000;       // liga PLL
		SYSCTL->RSCLKCFG |= 0x40000000;       // update PLL clock
		timeout = 0;
		while(((SYSCTL->PLLSTAT & 0x00000001) == 0) && (timeout < 0xFFFF)){
				timeout = timeout + 1;
				if(timeout == 0xFFFF){
					return 0;
				}
		}
	  /*****************   PLL (FIM)  *****************/

		SysTick_Config(SYSCLK / 1000);     							// Setup SysTick for 1 msec   

	  // ADC 
		ADC0_InitTimer0ATrigger();

		/* preset */
		memset(TimerDownCnt, 0, sizeof(TimerDownCnt));
		
		// serial init
	  SER_Initialize();	
	
		SerialData = '0';

		//*** ciclica
		while(1)
		{
				init = 0;
		}
}

void UART0_Handler(void){
		
		SerialData = (UART0->DR & 0xFF);

		UART0->ICR |= (1ul << 4);
		UART0->ICR |= (1ul << 4);
}


void MainTask()
{
		int Temp;
	
		if(!init)
		{
					/* voltage update*/
					if(TimerDownCnt[1]==0)
					{
							TimerDownCnt[1] = 1000;
							AVoltage = 100 * ((float)Voltage * (float)3.3)/4096; // alimetacao 3.3V : 12 bits (4096)
							Temp = 147.5 - (0.75* AVoltage);
							printf("Digits: %d | Voltage: %d | Temp: %d \n\r",(int)Voltage, AVoltage, Temp);
					}						
		}
}
