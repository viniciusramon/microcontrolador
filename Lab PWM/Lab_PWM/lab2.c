/************************************************************/
/*	LCD & Teclado 																					*/
/*																													*/
/************************************************************/

/************************	INCLUDES **************************/
#include <TM4C129.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

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
long								Voltage, TVoltage;
int									AVoltage1, AVoltage2;
long								Counter;
char								refresh;
char								SerialData;
long							  timeout;
short 							period;
short 							duty;

/********************	 Predefined functions ********************/
void MainTask(void);

/************************	  FUNCOES   ***********************/
void Delay(long value)
{
		int i;
		for(i=0;i<value;i++)
		{
		}
}

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
		Voltage = ADC0->SSFIFO3 & 0x00000FFF;
		
		Counter++;
		ADC0->ISC = 0x00000008;             // acknowledge ADC sequence 3 completion
																				// execute user task
	
}

void ADC0SS2_Handler(void){
		TVoltage = ADC0->SSFIFO2 & 0x00000FFF;
		ADC0->PSSI |= 8;
		ADC0->ISC = 0x00000004;             // acknowledge ADC sequence 2 completion
																			  // execute user task
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
  UART0->CC   =   5;                              /* use PIOSC (internal) clock   */
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
	
		//*** inicializoes
		init = 1;

		/* Enable clock and init GPIO outputs */
		SYSCTL->RCGCGPIO |= (1UL << 12) |(1UL << 8);  /* enable clock for GPIOs  */

		/* OUTROS */
		GPION->DIR      		 |= PN0 | PN1;   								/* PN1, PN0 is output        */
		GPION->DEN       		 |= PN0 | PN1;   								/* PN1, PN0 enable 				   */

		GPIOJ_AHB->PUR   		 |=  PJ0 | PJ1;  								/* PJ0, PJ1 pull-up          */
		GPIOJ_AHB->DIR   		 &= ~(PJ0 | PJ1);								/* PJ0, PJ1 is intput        */
		GPIOJ_AHB->DEN   		 |=  (PJ0 | PJ1);								/* PJ0, PJ1 is digital func. */

		// PWM init
		period = 10000;
		duty   = 2000;
		SYSCTL->RCGCPWM |= (1UL << 0);				// activate clock for PWM0																					
		SYSCTL->RCGCGPIO |= (1UL << 5);				// activate clock for Port F																					
		while((SYSCTL->PRGPIO) == 0){};
		GPIOF_AHB->AFSEL |= 0x01;             //  enable alt funct on PF0
		GPIOF_AHB->DEN   |= 0x01;             //  enable digital I/O on PF0																					
		GPIOF_AHB->PCTL  = (GPIOF_AHB->PCTL & 0xFFFFFFF0)+0x00000006; // configure PF0 as M0PWM0
		GPIOF_AHB->AMSEL &= ~0x01;            //    disable analog functionality on PF0
		while((SYSCTL->PRPWM) == 0){};	//    allow time for clock to stabilize
																					
		PWM0->_0_CTL = 0;                     // re-loading down-counting mode
		PWM0->CC = 0 + 0x00000100;						// set clock to divide per 2
		PWM0->_0_GENA = (0x000000C0 | 0x00000008); // PF0 goes low on LOAD / PF0 goes high on CMPA down
		PWM0->_0_LOAD = period - 1;           // cycles needed to count down to 0
		PWM0->_0_CMPA = duty - 1;             // count value when output rises
		PWM0->_0_CTL |= 0x00000001;   			  // start PWM0
		PWM0->ENABLE |= 0x00000001;   				// enable PF0/M0PWM0

		SysTick_Config(SYSCLK / 1000);     							// Setup SysTick for 1 msec   

		/* preset */
		memset(TimerDownCnt, 0, sizeof(TimerDownCnt));
		
		// serial init
	  SER_Initialize();	
	
		SerialData = '0';

		//*** ciclica
		while(1)
		{
				GPION->DATA |= PN0;
				init = 0;
		}
}

void UART0_Handler(void){
		
		SerialData = (UART0->DR & 0xFF);

		UART0->ICR |= (1ul << 4);
		UART0->ICR |= (1ul << 4);
}

void setPWMduty(short duty){
  PWM0->_0_CMPA = duty - 1; 
}

void MainTask()
{
		if(!init)
		{
					/* voltage update*/
					if(TimerDownCnt[1]==0)
					{
							TimerDownCnt[1] = 50;

							if(!(GPIOJ_AHB->DATA & PJ0))
							{
									duty = ((duty + 50) >= period)? duty: duty + 50;
									setPWMduty(duty);		
							}

							if(!(GPIOJ_AHB->DATA & PJ1))
							{
									duty = ((duty - 50) < 50)? duty: duty - 50;
									setPWMduty(duty);		
							}

							printf("%d\n\r", (int) duty);
					}						
		}
}
