/************************************************************/
/*	LCD & Teclado 																					*/
/*																													*/
/************************************************************/

/************************	INCLUDES **************************/
#include <TM4C129.h>
#include <string.h>
#include <stdlib.h>

/************************	DEFINES ***************************/
#define PN0				(1UL << 0)
#define PN1				(1UL << 1)
#define PF0				(1UL << 0)

/************************	TECLADO	***************************/
/* Saidas */
#define PF1				(1UL << 1)
#define PF2				(1UL << 2)
#define PF3				(1UL << 3)
#define PG0				(1UL << 0)

/* Entradas */
#define PL4				(1UL << 4)
#define PL5				(1UL << 5)
#define PL0				(1UL << 0)
#define PL1				(1UL << 1)

/************************	  LCD   ***************************/
/* Saidas */
#define PB5				(1UL << 5)	// RS
#define PK0				(1UL << 0)	// R/W
#define PK1				(1UL << 1)	// E
#define PK2				(1UL << 2)	// CB5
#define PK3				(1UL << 3)	// CB6
#define PA4				(1UL << 4)	// CB7
#define PA5				(1UL << 5)	// CB8

#define	PORTA			 GPIOA_AHB->DATA
#define	PORTB			 GPIOB_AHB->DATA
#define	PORTK			 GPIOK->DATA

/* LCD ERROR  */
#define LCD_ERROR_SIZE	1

/* LCD TEXTOS */
//								"1234567890123456"
#define	TSCR_1_1	"** FACULDADES **"
#define	TSCR_1_2	"**  EINSTEIN  **"

#define	TSCR_2_1	"Tempo:          "


/***********************	  OUTROS  *************************/
#define PJ0				(1ul << 0)
#define PJ1				(1ul << 1)

#define	InPJ0			(GPIOJ_AHB->DATA) & (1ul << 0)
#define	PortN			GPION->DATA
#define	PortF			GPIOF_AHB->DATA

/************************	ESTRUTURAS ************************/
typedef struct{
		char 		in;
		char		inPin[4];
		char		out;
		char		key;	
		char		*keyChar;
} key_typ;

/********************	 VARIAVEIS GLOBAIS ********************/
key_typ							Key;
char								actualKey;
int									Valor;
char								SValor[4];
char								keyStep;
char								keyCol;
int 								msTicks;                 // systick counter      
long								TimerDownCnt;

/********************	 Predefined functions ********************/
void MainTask(void);

/************************	  FUNCOES   ***********************/
/*----------------------------------------------------------------------------
    SysTick_Handler - 1ms
 *----------------------------------------------------------------------------*/
void SysTick_Handler(void) {
		msTicks++;
		if(TimerDownCnt>0)
		{
				TimerDownCnt--;
		}
		MainTask();
}

/*----------------------------------------------------------------------------
    delays number of tick Systicks (happens every 1 ms)
 *----------------------------------------------------------------------------*/
void Delay (long dlyTicks) {
		int curTicks;

		curTicks = msTicks;
		while ((msTicks - curTicks) < dlyTicks)
		{
			__NOP();
		}
}

	
void CheckKeyIOs(char keyColumn)
{
		int i;
	
		Key.out = (Key.out >= 4)? 0: keyColumn;
		
		/* write */
		switch(Key.out)
		{
				case 0:
					 GPIOF_AHB->DATA &= ~(1UL << 1);
					 GPIOF_AHB->DATA &= ~(1UL << 2);
					 GPIOF_AHB->DATA &= ~(1UL << 3);
					 GPIOG_AHB->DATA |=  (1UL << 0);
  			break;				
				case 1:
					 GPIOF_AHB->DATA &= ~(1UL << 1);
					 GPIOF_AHB->DATA &= ~(1UL << 2);
					 GPIOF_AHB->DATA |=  (1UL << 3);
					 GPIOG_AHB->DATA &= ~(1UL << 0);
  			break;
				case 2:
					 GPIOF_AHB->DATA &= ~(1UL << 1);
					 GPIOF_AHB->DATA |=  (1UL << 2);
					 GPIOF_AHB->DATA &= ~(1UL << 3);
					 GPIOG_AHB->DATA &= ~(1UL << 0);
  			break;
				case 3:
					 GPIOF_AHB->DATA |=  (1UL << 1);
					 GPIOF_AHB->DATA &= ~(1UL << 2);
					 GPIOF_AHB->DATA &= ~(1UL << 3);
					 GPIOG_AHB->DATA &= ~(1UL << 0);
  			break;
		}

		Delay(1);

	  /* read */		
	  Key.inPin[0] =   ((GPIOL->DATA) & (1ul << 1))? 1:0;
		Key.inPin[1] =   ((GPIOL->DATA) & (1ul << 0))? 1:0;
		Key.inPin[2] =   ((GPIOL->DATA) & (1ul << 5))? 1:0;
		Key.inPin[3] =   ((GPIOL->DATA) & (1ul << 4))? 1:0;

		Key.in = 0;
		for(i=0;i<4;i++)
		{
				if(Key.inPin[i])
				  	Key.in = i+1;
		}		
}

void LCD_RS(char value)
{
	  if(value)
	  {
				PORTB |= PB5;
		}
	  else 
		{
				PORTB &= ~PB5;			
		}	
}

void LCD_RW(char value)
{
	  if(value)
	  {
				PORTK |= PK0;
		}
	  else 
		{
				PORTK &= ~PK0;			
		}	
}

void LCD_E(char value)
{
	  if(value)
	  {
				PORTK |= PK1;
		}
	  else 
		{
				PORTK &= ~PK1;			
		}	
}

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

void LCD_EX()
{
		Delay(1);
		LCD_E(1);
		Delay(3);
		LCD_E(0);
		Delay(1);
}

void LCD_COut(char data)
{
		char nibble;
		nibble = data & 0xF0;
		nibble = nibble >> 4;
		LCD_DATA(nibble);
		LCD_EX();
		nibble = data & 0x0F;
		LCD_DATA(nibble);
		LCD_EX();		
}

void SetLoc(char line, char loc)
{
		LCD_RS(0);
		if(line==1)
		{
				LCD_COut(0x80 + loc);	
		}
		else
		{
				LCD_COut(0xC0 + loc);	
		}
		LCD_RS(1);
}

char LCD_Out(char text[], char line, char loc)
{
		int i;
		int size = strlen(text);
		
		if(size>16)
		{
				return LCD_ERROR_SIZE;
		}
		
		SetLoc(line, loc);
		
		for(i=0;i<size;i++)
		{
				LCD_COut(text[i]);
		}
		return 0;
}

void LCD_Limpa()
{
		LCD_RS(0);
		LCD_COut(0x01);
		LCD_RS(1);
}

void LCD_Cursor(char type)
{
		LCD_RS(0);
		if(type)
		{
				// configura display - cursor piscante 
				LCD_COut(0x0F);
		}
		else
		{
				// configura display - cursor piscante 
				LCD_COut(0x0C);
		}
		LCD_RS(1);
}

void LCDConfig()
{
		/* modo de operacao 4 bits */
		LCD_RS(0);
		LCD_RW(0);	
		LCD_EX();
		LCD_DATA(0x02);
		LCD_EX();
	
		// configura display - 2 linhas 
		LCD_COut(0x28);

		// cursor piscante	
		LCD_Cursor(1);
	
		LCD_Limpa();
	
		LCD_RS(1);
}

void TestButton()
{
		if (~InPJ0)
		{
				 PortN  ^= PN0;		// liga PN0
				 Delay(250);
		}
}

char GetKey()
{
		char keyIdx;

		switch(keyStep)
		{
				case 0:
						keyCol = (keyCol >= 4)? 0 : keyCol + 1;
						CheckKeyIOs(keyCol);
						if(Key.in)
						{
								keyStep = 1;
						}
				break;

				case 1:
						CheckKeyIOs(keyCol);
						keyIdx = (Key.out * 4) + (Key.in-1);
						Key.key = Key.keyChar[keyIdx]; 
						keyStep = 2;					
				break;
				case 2:
						CheckKeyIOs(keyCol);
						if(!Key.in)
						{									
								keyStep = 0;
								return Key.key;
						}
				break;
		}		
		return '?';
}


int main()
{
		//*** variaveis locais
		char val_step, keyPressed, keyPressedCnt;
		char i;
	
		//*** inicializoes
		/* Enable clock and init GPIO outputs */
		SYSCTL->RCGCGPIO |= (1UL << 0) | (1UL << 1) | (1UL << 9) | (1UL << 12)	| (1UL << 5) | (1UL << 6) | (1UL << 8) | (1UL << 10);  /* enable clock for GPIOs  */

		/* TECLADO */
		/* OUTPUTS */
		GPIOF_AHB->DIR	 |= PF0 | PF1 | PF2 | PF3;   	
		GPIOF_AHB->DEN	 |= PF0 | PF1 | PF2 | PF3;   	

		GPIOG_AHB->DIR	 |= PG0;   	  								
		GPIOG_AHB->DEN	 |= PG0;     									
	
		/* INPUTS */
		GPIOL->PDR   		 |=  PL0 | PL1 | PL4 | PL5;  	/* PL0, PL1, PL4, PL5 pull-up          */
		GPIOL->DIR   		 &= ~(PL0 | PL1 | PL4 | PL5);	/* PL0, PL1, PL4, PL5 is intput        */
		GPIOL->DEN   		 |=  (PL0 | PL1 | PL4 | PL5);	/* PL0, PL1, PL4, PL5 is digital func. */	
	
		/* LCD */
		/* OUTPUTS */
		GPIOB_AHB->DIR       |= PB5;   								
		GPIOB_AHB->DEN       |= PB5;   								

		GPIOA_AHB->DIR       |= PA4 | PA5;   					
		GPIOA_AHB->DEN       |= PA4 | PA5;   

		GPIOK->DIR       		 |= PK0 | PK1 | PK2 | PK3;   
		GPIOK->DEN       		 |= PK0 | PK1 | PK2 | PK3;   

		/* OUTROS */
		GPION->DIR      		 |= PN0 | PN1;   								/* PN1, PN0 is output        */
		GPION->DEN       		 |= PN0 | PN1;   								/* PN1, PN0 enable 				   */

		GPIOJ_AHB->PUR   		 |=  PJ0 | PJ1;  								/* PJ0, PJ1 pull-up          */
		GPIOJ_AHB->DIR   		 &= ~(PJ0 | PJ1);								/* PJ0, PJ1 is intput        */
		GPIOJ_AHB->DEN   		 |=  (PJ0 | PJ1);								/* PJ0, PJ1 is digital func. */

		SystemCoreClockUpdate();                       	 		// Get Core Clock Frequency   
		SysTick_Config(SystemCoreClock / 1000);     				// Setup SysTick for 1 msec   

		/* preset */
		Key.keyChar = "147*2580369#ABCD";
		TimerDownCnt = 0;
		memset(SValor, ' ', sizeof(SValor));

		LCDConfig();

		/* LCD */
		LCD_Out(TSCR_1_1,1,0);
		LCD_Out(TSCR_1_2,2,0);

		Delay(2000);

		LCD_Limpa();
		LCD_Out(TSCR_2_1,1,0);
		SetLoc(1, 10);

		//*** ciclica
		while(1)
		{
				switch(val_step)
				{
						case 0:
								if(GetKey() == '#')
								{								
										keyPressedCnt = 4;
										val_step = 1;
										memset(SValor, '_', sizeof(SValor));
										SValor[2] = '.';
										LCD_Out(SValor,1,7);
								}
						break;
								
						case 1:
								keyPressed = GetKey();
						
								if((keyPressed != '#') && (keyPressed != '?') && (keyPressedCnt > 0 ))
								{
									  for(i=1;i<4;i++)
										{								
												if(i==3)
												{
														SValor[i-2] = SValor[i];
														SValor[2] = '.';
												}
												else
												{
														SValor[i-1] = SValor[i];
												}
										}
										SValor[3] = keyPressed;			
										LCD_Out(SValor,1,7);
										keyPressedCnt--;										
								}	
								
								if(keyPressed == '#')
								{		
									  for(i=0;i<4;i++)
										{
												if(SValor[i] == '_')
												{
														SValor[i] = ' ';
												}												
										}										

										LCD_Out(SValor,1,7);

									  for(i=2;i>0;i--)
										{
												SValor[i] = SValor[i-1];
										}										
										SValor[0] = ' ';										

										Valor = atoi(SValor);

										val_step = 0;
								}
						break;
							
				}				
			
				/* outros */
				TestButton();
		}
}

void MainTask()
{
		int ledTime;
		ledTime = Valor *100;
	
		TimerDownCnt = (TimerDownCnt == 0)?ledTime: TimerDownCnt;

		if(TimerDownCnt > ledTime/2)
		{
				PortN |= PN1;				
		}	
		else
		{
				PortN &= ~PN1;
		}
}
