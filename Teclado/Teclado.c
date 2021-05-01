/*----------------------------------------------------------------------------
		TECLADO

*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
		Includes
 *----------------------------------------------------------------------------*/
 #include <TM4C129.h>

/*----------------------------------------------------------------------------
		Defines
 *----------------------------------------------------------------------------*/
#define	PF1			(1UL << 1)
#define	PF2			(1UL << 2)
#define	PF3			(1UL << 3)
		
#define	PG0			(1UL << 0)

#define	GrupoF 	(1UL << 5)
#define	GrupoG 	(1UL << 6)
#define	GrupoL 	(1UL << 10)

#define	PL0			(1UL << 0)
#define	PL1			(1UL << 1)
#define	PL4			(1UL << 4)
#define	PL5			(1UL << 5)

/*----------------------------------------------------------------------------
		Global Variables
 *----------------------------------------------------------------------------*/
char		Tecla;	
	
/*----------------------------------------------------------------------------
    Functions
 *----------------------------------------------------------------------------*/
void Delay(int delay)
{
		int i;
		for(i=0; i < delay; i++)
		{
		}
}

/*----------------------------------------------------------------------------
    MAIN
 *----------------------------------------------------------------------------*/
int main()
{
	
		/***********************  INIT  ***********************/
		/* Enable clock and init GPIO outputs */
		SYSCTL->RCGCGPIO |= GrupoF | GrupoG | GrupoL;  /* enable clock for GPIOs  */

		/* TECLADO */
		/* OUTPUTS */
		GPIOF_AHB->DIR	 |= PF1 | PF2 | PF3;  /* direcao: 0 = entrada ou 1 = saida */ 	
		GPIOF_AHB->DEN	 |= PF1 | PF2 | PF3;  /* habilita o pino */ 	

		GPIOG_AHB->DIR	 |= PG0;   	  								
		GPIOG_AHB->DEN	 |= PG0;     									
	
		/* INPUTS */
		GPIOL->PDR   		 |=   PL0 | PL1 | PL4 | PL5;  /* PL0, PL1, PL4, PL5 pull-down          */
		GPIOL->DIR   		 &= ~(PL0 | PL1 | PL4 | PL5);	/* PL0, PL1, PL4, PL5 is intput        */
		GPIOL->DEN   		 |=  (PL0 | PL1 | PL4 | PL5);	/* PL0, PL1, PL4, PL5 is digital func. */	

		Tecla = '?';		// ? = nenhuma tecla pressionada 

		GPIOG_AHB->DATA &= ~PG0;	// resetar coluna 1
		GPIOF_AHB->DATA &= ~PF3;	// resetar coluna 2
		GPIOF_AHB->DATA &= ~PF2;	// resetar coluna 3
		GPIOF_AHB->DATA &= ~PF1;	// resetar coluna 4

		/*********************** CYCLIC ***********************/
		while(1)
		{
					GPIOG_AHB->DATA |= PG0;	// seleciona coluna 1
					Delay(1000);
					if(GPIOL->DATA & PL1)
					{
							Tecla = '1';
					}					
					else if(GPIOL->DATA & PL0)
					{
							Tecla = '4';
					}
					else if(GPIOL->DATA & PL5)
					{
							Tecla = '7';
					}
					else if(GPIOL->DATA & PL4)
					{
							Tecla = '*';
					}

					GPIOG_AHB->DATA &= ~PG0;	// resetar coluna 1
					GPIOF_AHB->DATA |= PF3;		// seleciona coluna 2
					Delay(2000);

					if(GPIOL->DATA & PL1)
					{
							Tecla = '2';
					}					
					else if(GPIOL->DATA & PL0)
					{
							Tecla = '5';
					}
					else if(GPIOL->DATA & PL5)
					{
							Tecla = '8';
					}
					else if(GPIOL->DATA & PL4)
					{
							Tecla = '0';
					}
					
					GPIOF_AHB->DATA &= ~PF3;	// resetar coluna 2
					GPIOF_AHB->DATA |= PF2;		// seleciona coluna 3
					Delay(100);

					if(GPIOL->DATA & PL1)
					{
							Tecla = '3';
					}					
					else if(GPIOL->DATA & PL0)
					{
							Tecla = '6';
					}
					else if(GPIOL->DATA & PL5)
					{
							Tecla = '9';
					}
					else if(GPIOL->DATA & PL4)
					{
							Tecla = '#';
					}
						
					GPIOF_AHB->DATA &= ~PF2;	// resetar coluna 3
					GPIOF_AHB->DATA |= PF1;		// seleciona coluna 4
					Delay(500);

					if(GPIOL->DATA & PL1)
					{
							Tecla = 'A';
					}					
					else if(GPIOL->DATA & PL0)
					{
							Tecla = 'B';
					}
					else if(GPIOL->DATA & PL5)
					{
							Tecla = 'C';
					}
					else if(GPIOL->DATA & PL4)
					{
							Tecla = 'D';
					}
					
					GPIOF_AHB->DATA &= ~PF1;	// resetar coluna 4
			
		} // while(1)
}
