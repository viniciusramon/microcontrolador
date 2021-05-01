/*----------------------------------------------------------------------------
		GPIO - SEMAFORO

*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
		Includes
 *----------------------------------------------------------------------------*/
 #include <TM4C129.h>

/*----------------------------------------------------------------------------
		Defines
 *----------------------------------------------------------------------------*/
#define PN0				(1UL << 0)						// Led 2
#define PN1				(1UL << 1)						// Led 1

#define PF0				(1UL << 0)						// Led 4
#define PF4				(1UL << 4)						// Led 3

#define PJ0				(1ul << 0)						// SW1
#define PJ1				(1ul << 1)						// SW2

#define	PortN			GPION->DATA
#define	PortF			GPIOF_AHB->DATA

/*----------------------------------------------------------------------------
		Global Variables
 *----------------------------------------------------------------------------*/
char				AbSw1;				// Input  switch 1 
char				AbSw2;				// Input  switch 2
char				SbLed1;				// Output led1
char				SbLed2;				// Output led2
char				SbLed3;				// Output led3
char				SbLed4;				// Output led4

/*----------------------------------------------------------------------------
    Functions
 *----------------------------------------------------------------------------*/

void Delay(long value)
{
		int i;
		for(i=0;i<value;i++)
		{
		}
}

/*----------------------------------------------------------------------------
    MAIN
 *----------------------------------------------------------------------------*/
int main()
{
		// Local variables
		char 	run;	
		int 	i;
	
		/***********************  INIT  ***********************/
		/* Enable clock and init GPIO outputs */
		SYSCTL->RCGCGPIO |= (1UL << 12)	| (1UL << 5);  	/* enable clock for GPIOs    */
		
		GPION->DIR       |= PN0 | PN1;   							 	/* PN1, PN0 is output        */
		GPION->DEN       |= PN0 | PN1;   							 	/* PN1, PN0 is digital func. */

		GPIOF_AHB->DIR   |= PF4 | PN0;   								/* PF4, PF0 is output        */
		GPIOF_AHB->DEN 	 |= PF4 | PN0;   								/* PF4, PF0 is digital func. */
		
		SYSCTL->RCGCGPIO |=  (1ul << 8); 								/* enable clock for GPIOs    */
		GPIOJ_AHB->PUR   |=   PJ0 | PJ1;  							/* PJ0, PJ1 pull-up        */
		GPIOJ_AHB->DIR   &= ~(PJ0 | PJ1);								/* PJ0, PJ1 is intput        */
		GPIOJ_AHB->DEN   |=  (PJ0 | PJ1);								/* PJ0, PJ1 is digital func. */

		SbLed1 = 0;
		SbLed2 = 0;
		SbLed3 = 0;

		run = 0;

		/*********************** CYCLIC ***********************/
		while(1)
		{
				// read inputs
				AbSw1 = (((GPIOJ_AHB->DATA) & PJ0))? 0 : 1;		// pull up switch
				AbSw2 = (((GPIOJ_AHB->DATA) & PJ1))? 0 : 1;		// pull up switch		
				
				// Start / Stop semaforo
				if(AbSw1)
				{
						run = 1;
				}

				if(AbSw2)
				{
						run = 0;
				}
				
				if(run)
				{
						PortN |= PN1; // seta bit 1 da porta PORTA N
						
						Delay(10000000);
		
						PortN &= ~PN1; // reseta o bit 1 da porta N 	
						PortN |= PN0; 

						Delay(3000000);
						
						PortN &= ~PN0;	
						PortF |= PF4;

						Delay(10000000);
						
						PortF &= ~PF4;
				}								
		} // while(1)
}
