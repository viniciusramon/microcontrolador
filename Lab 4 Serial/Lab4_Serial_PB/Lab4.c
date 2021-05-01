/*----------------------------------------------------------------------------
		BASICO
 *----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
		Includes
 *----------------------------------------------------------------------------*/
 #include <TM4C129.h>
 #include <stdio.h>

/*----------------------------------------------------------------------------
		Defines
 *----------------------------------------------------------------------------*/
#define PN0				(1UL << 0)						

#define PJ0				(1UL << 0)						
#define PJ1				(1UL << 1)

#define	PortN			GPION->DATA
#define	PortF			GPIOF_AHB->DATA

/*----------------------------------------------------------------------------
		Global Variables
 *----------------------------------------------------------------------------*/
long				TimerUpCnt;	// timer up counter
char				S1, S2;
char				step;
long				Time;
char				SerialData;

/*----------------------------------------------------------------------------
    SysTick_Handler
 *----------------------------------------------------------------------------*/
void SysTick_Handler(void) {
		TimerUpCnt ++;
}

void UART0_Handler(void){
		
		SerialData = (UART0->DR & 0xFF);

		UART0->ICR |= (1ul << 4);
		UART0->ICR |= (1ul << 4);
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

/*----------------------------------------------------------------------------
    MAIN
 *----------------------------------------------------------------------------*/
int main()
{
		// Local variables
	
		/***********************  INIT  ***********************/
		/* Enable clock and init GPIO outputs */
		SYSCTL->RCGCGPIO |= (1UL << 12);  
		
		GPION->DIR       |= PN0;   							 	
		GPION->DEN       |= PN0;   							 	
	
		SYSCTL->RCGCGPIO |=  (1ul << 8); 								
		GPIOJ_AHB->PUR   |=  PJ0 | PJ1;  								
		GPIOJ_AHB->DIR   &= ~(PJ0 | PJ1);								
		GPIOJ_AHB->DEN   |=  (PJ0 | PJ1);								

		SystemCoreClockUpdate();                       
		SysTick_Config(SystemCoreClock / 1000);     	 

		// serial init
	  SER_Initialize();	
	
		SerialData = '0';
	
		/*********************** CYCLIC ***********************/
		while(1)
		{
				printf("%d\n\r",(int)(GPIOJ_AHB->DATA & PJ0));
			
				if(SerialData == '0')
				{
					  GPION->DATA &= ~PN0;
				}
				else
				{
						GPION->DATA |= PN0;
				}

		} // while(1)
		
}
