/*----------------------------------------------------------------------------
		DISPLAY LCD E TECLADO

*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
		Includes
 *----------------------------------------------------------------------------*/
 #include <TM4C129.h>

/*----------------------------------------------------------------------------
		Defines
 *----------------------------------------------------------------------------*/
#define PA4			(1UL << 4) // Habilita a porta PA4
#define PA5			(1UL << 5) // Habilita a porta PA5

#define PB5			(1UL << 5) // Habilita a porta PB5

#define	PF1			(1UL << 1)
#define	PF2			(1UL << 2)
#define	PF3			(1UL << 3)
		
#define	PG0			(1UL << 0)

#define PK0			(1UL << 0) // Habilita a porta PK0
#define PK1			(1UL << 1) // Habilita a porta PK1
#define PK2			(1UL << 2) // Habilita a porta PK2
#define PK3			(1UL << 3) // Habilita a porta PK3

#define	PL0			(1UL << 0)
#define	PL1			(1UL << 1)
#define	PL4			(1UL << 4)
#define	PL5			(1UL << 5)

#define GrupoA (1UL << 0) // Habilita o grupo das portas A
#define GrupoB (1UL << 1) // Habilita o grupo das portas B
#define GrupoK (1UL << 9) // Habilita o grupo das portas K
#define	GrupoF (1UL << 5)
#define	GrupoG (1UL << 6)
#define	GrupoL (1UL << 10)

#define PORTA		GPIOA_AHB->DATA
#define PORTB		GPIOB_AHB->DATA
#define PORTK		GPIOK->DATA


/*----------------------------------------------------------------------------
		Global Variables
 *----------------------------------------------------------------------------*/
char	Tecla;	
int 	TimerUp, Init;
long 	ticks;
	
/*----------------------------------------------------------------------------
    Functions
 *----------------------------------------------------------------------------*/

void LCD_DATA(char value)
{
	if(value & 0x01)
	{
		PORTK |= PK2;
	}
	else
	{
		PORTK &= ~PK2;
	}
	
	if(value & 0x02)
	{
		PORTK |= PK3;
	}
	else
	{
		PORTK &= ~PK3;
	}
	
	if(value & 0x04)
	{
		PORTA |= PA4;
	}
	else
	{
		PORTA &= ~PA4;
	}
	
	if(value & 0x08)
	{
		PORTA |= PA5;
	}
	else
	{
		PORTA &= ~PA5;
	}
}

void SysTick_Handler()
{
			ticks++;
}

void Delay(long tempo)
{
	long start=ticks;
	while((ticks-start)<tempo)
	{
	}
			
}

void Enable()
{
				Delay(5);
				GPIOK->DATA &= ~PK1;
				Delay(5);
				GPIOK->DATA |= PK1;
				Delay(5);
				GPIOK->DATA &= ~PK1;
				Delay(5);
}

/*----------------------------------------------------------------------------
    MAIN
 *----------------------------------------------------------------------------*/
int main()
{
		char Letra, nibble;
		int cnt=0;
	
		/***********************  INIT  ***********************/
		/* Enable clock and init GPIO outputs */
		SYSCTL->RCGCGPIO |= GrupoF | GrupoG | GrupoL | GrupoA | GrupoB | GrupoK;  /* enable clock for GPIOs  */

		/* TECLADO */
		/* OUTPUTS */
		GPIOF_AHB->DIR	 |= PF1 | PF2 | PF3;  /* direcao: 0 = entrada ou 1 = saida */ 	
		GPIOF_AHB->DEN	 |= PF1 | PF2 | PF3;  /* habilita o pino */ 	

		GPIOG_AHB->DIR	 |= PG0; //DEFINE PG0 COMO SAÍDA				
		GPIOG_AHB->DEN	 |= PG0; //HABILITA O PG0
	
		/* INPUTS */
		GPIOL->PDR   		 |=   PL0 | PL1 | PL4 | PL5;  /* PL0, PL1, PL4, PL5 pull-down          */
		GPIOL->DIR   		 &= ~(PL0 | PL1 | PL4 | PL5);	/* PL0, PL1, PL4, PL5 is intput        */
		GPIOL->DEN   		 |=  (PL0 | PL1 | PL4 | PL5);	/* PL0, PL1, PL4, PL5 is digital func. */	

		Tecla = 'x';		// ? = nenhuma tecla pressionada 

		GPIOG_AHB->DATA &= ~PG0;	// resetar coluna 1
		GPIOF_AHB->DATA &= ~PF3;	// resetar coluna 2
		GPIOF_AHB->DATA &= ~PF2;	// resetar coluna 3
		GPIOF_AHB->DATA &= ~PF1;	// resetar coluna 4
		
		//Display LCD
		/* OUTPUTS */
		GPIOA_AHB->DIR	 |= PA4 | PA5;  /* direcao: 0 = entrada ou 1 = saida */ 	
		GPIOA_AHB->DEN	 |= PA4 | PA5;  /* habilita o pino */ 	
	
		GPIOB_AHB->DIR	 |= PB5;  /* direcao: 0 = entrada ou 1 = saida */ 	
		GPIOB_AHB->DEN	 |= PB5;  /* habilita o pino */ 	
	
		GPIOK->DIR	 |= PK0 | PK1 | PK2 | PK3;  /* direcao: 0 = entrada ou 1 = saida */ 	
		GPIOK->DEN	 |= PK0 | PK1 | PK2 | PK3;  /* habilita o pino */ 	
	
		SystemCoreClockUpdate();
		SysTick_Config(SystemCoreClock/1000); // base de tempo 

		/*********************** CYCLIC ***********************/
		
		//Inicialização do Display
		Delay(500);		//Delay de 500ms
				
		GPIOB_AHB->DATA &= ~PB5; // COLOCA RS=0
		GPIOK->DATA &= ~PK0; // COLOCA RW=0
		Enable();
		
		//INICIALIZAÇÃO DO DISPLAY E MODO DE UTILIZAÇÃO
		// 4BITS, 2 LINHAS E 5X7 CARACTERES
		LCD_DATA(2);	//Manda o valor 2
		Enable();	// Faz o Enable
		LCD_DATA(2);	//Manda o valor 2
		Enable();
		
		LCD_DATA(8); //Manda o valor 8
		Enable();

		//CONTROLE DO DISPLAY
		LCD_DATA(0);
		Enable();
		
		LCD_DATA(0x0F);		
		Enable();
		
		//LIMPA DISPLAY
		LCD_DATA(0);
		Enable();
		
		LCD_DATA(1);
		Enable();
		
		GPIOB_AHB->DATA |= PB5; // COLOCA RS=1
		
		while(1)
		{
					GPIOG_AHB->DATA |= PG0;	// seleciona coluna 1
					Delay(100);
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
					Delay(100);

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
					Delay(100);

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
			
					if(Tecla!='x')
					{
						//QUANDO CHEGAR NA ULTIMA COLUNA DA ULTIMA LINHA LIMPA O DISPLAY
						if(cnt==32)
						{
						GPIOB_AHB->DATA &= ~PB5; //Coloca RS=0

							cnt =0;	
							//LIMPA DISPLAY
							LCD_DATA(0);
							Enable();

							LCD_DATA(1);
							Enable();

						GPIOB_AHB->DATA |= PB5; // COLOCA RS=1


						}
						
						//POSICIONA O CURSOR NA PRIMEIRA LNHA DA PRIMEIRA COLUNA					
						if(cnt==0)
						{
						GPIOB_AHB->DATA &= ~PB5; //Coloca RS=0

						LCD_DATA(8); //Manda o valor 8
						Enable();

						LCD_DATA(0); //Manda o valor 0
						Enable();
						
						GPIOB_AHB->DATA |= PB5; // COLOCA RS=1
						
						}
						
						//RECEBE OS VALORES DAS TECLAS DO TECLADO
						Letra=Tecla;
						nibble=(Letra&0xF0);
						nibble=nibble>>4;
						LCD_DATA(nibble);
						Enable();
		
						Letra=Tecla;
						nibble=(Letra&0x0F);
						LCD_DATA(nibble);
						Enable();
						
						Tecla='x';
						
						cnt++;

						/*QUANDO CHEGAR NA ULTIMA COLUNA DA PRIMEIRA LINHA O CURSOR PULA 
						PARA A PRIMEIRA COLUNA DA SEGUNDA LINHA*/
						if(cnt==16)
						{
							
						GPIOB_AHB->DATA &= ~PB5; //Coloca RS=0

						LCD_DATA(0x0C); //Manda o valor 8
						Enable();

						LCD_DATA(0); //Manda o valor 0
						Enable();
						
						GPIOB_AHB->DATA |= PB5; // COLOCA RS=1
						
						}

						Delay(100);
						}
		} // while(1)
}
