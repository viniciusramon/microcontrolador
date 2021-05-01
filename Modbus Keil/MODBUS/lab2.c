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

/* Timers */
#define	TIMER_DOWN_SERIAL		0
#define	MAX_NR_TIMER_DOWN		1

/* Analog input */
#define	AI_PE2					0
#define	AI_TEMP					1
#define	MAX_NR_AI				2

#define	F_VOLTAGE	  		1

#define FXTAL 25000000  // fixed, this crystal is soldered to the Connected Launchpad
#define Q            0
#define N            4  // chosen for reference frequency within 4 to 30 MHz
#define MINT        96  // 480,000,000 = (25,000,000/(0 + 1)/(4 + 1))*(96 + (0/1,024))
#define MFRAC        0  // zero to reduce jitter
#define PSYSDIV 		 29
#define SYSCLK (FXTAL/(Q+1)/(N+1))*(MINT+MFRAC/1024)/(PSYSDIV+1)

/************************	ESTRUTURAS ************************/
typedef struct
{
		char		Address;
		char		Function;
		char		DataAdr0;
		char		DataAdr1;
		char		DataLen0;
		char		DataLen1;
		char		DataCRC0;
		char		DataCRC1;	
		char		Request;
		char		Step;
		char		SendD[7];
		char		DataAdress;
		char		Data[10];
} Modbus_typ;

typedef struct
{
		int 		AIDig[2];						/* Analog input in digits  */
		int			AIVolt[2];					/* Analog input in voltage */
		int			TempSensor;					/* temperature sensor in C */
} AI_typ;


/********************	 VARIAVEIS GLOBAIS ********************/
int									Valor;
int 								msTicks;			                	 // systick counter      
long								TimerDownCnt[MAX_NR_TIMER_DOWN];
char								init;
char								NovoValor;
long								Voltage, TVoltage;
int									AVoltage, TAVoltage;
long								Counter;
char								refresh;
char								SerialData;
Modbus_typ					Modbus;
AI_typ							AI;

int									sCnt;
int									send_step;
int									crcValue;
char								crc0;
char								crc1;
char 								seq;

/********************	 Predefined functions ********************/
void MainTask(void);

/************************	  FUNCOES   ***********************/
/*----------------------------------------------------------------------------
    SysTick_Handler - 1ms
 *----------------------------------------------------------------------------*/
void SysTick_Handler(void) {
		int i;
		msTicks++;
		for(i=0;i<MAX_NR_TIMER_DOWN;i++)
		{
				if(TimerDownCnt[i]>0)
				{
						TimerDownCnt[i]--;
				}
		}

		MainTask();
}

void ADC0SS2_Handler(void){

			AI.AIDig[AI_TEMP] = ADC0->SSFIFO2;
			AI.AIDig[AI_PE2] = ADC0->SSFIFO2;
			ADC0->ISC = 0x00000004;             // acknowledge ADC sequence 2 completion
}

void UART0_Handler(void){
		
		SerialData = (UART0->DR & 0xFF);
	
		switch(Modbus.Step)
		{
				case 0: 
						if(Modbus.Address == SerialData)
						{
								Modbus.Step = 1;
						}
				break;
						
				case 1:
						if(SerialData == 0x03) 
						{
								Modbus.Function = 0x03;
								Modbus.Step = 2;							
						}
						else if(SerialData == 0x06)
						{
								Modbus.Function = 0x06;
								Modbus.Step = 0;
						}
						else
						{
								Modbus.Step = 0;
						}
				break;

				case 2:
						Modbus.DataAdr0 = SerialData;
						Modbus.Step = 3;				
				break;

				case 3:
						Modbus.DataAdr1 = SerialData;
						Modbus.Step = 4;				
				break;

				case 4:
						Modbus.DataLen0 = SerialData;
						Modbus.Step = 5;				
				break;

				case 5:
						Modbus.DataLen1 = SerialData;
						Modbus.Step = 6;				
				break;

				case 6:
						Modbus.DataCRC0 = SerialData;
						Modbus.Step = 7;				
				break;

				case 7:
						Modbus.DataCRC1 = SerialData;
						Modbus.Request = 1;
 					  Modbus.Step = 0;
				break;
				
				case 8:					
				break;
		}
				
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

int crc(char *buf, int start, int cnt) 
{
		int i, j;
		int temp, temp2;
		int flag;

		temp = 0xFFFF;

		for (i = start; i < cnt; i++)
		{
				temp = temp ^ buf[i];

				 for (j = 1; j <= 8; j++)
				 {
						flag = temp & 0x0001;
						temp = temp >> 1;
						if (flag==1)
						{
								temp = temp ^ 0xA001;
						}
				 }
	 }


		/* Reverse byte order. */
		temp2 = temp >> 8;
		temp = (temp << 8) | temp2;
		temp &= 0xFFFF;

		return (temp);
}

int main()
{
		//*** variaveis locais
	
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
		// OSCSRC = 3:MOSC is the oscillator source ; PLLSRC = 3: MOSC is the PLL input clock ; + PSYSDIV
		SYSCTL->RSCLKCFG = 0x03300000 ;
		SYSCTL->DSCLKCFG = (SYSCTL->DSCLKCFG &~0x00F00000)+0x00300000;
		SYSCTL->PLLFREQ0 |= ((MFRAC << 10) | (MINT << 0));
		SYSCTL->PLLFREQ1 |= ((Q << 8) |(N << 0));
		SYSCTL->RSCLKCFG |= 0x40000000;       // update PLL clock
		SYSCTL->PLLFREQ0 |= 0x00800000;       // enable PLL
			
		if(SYSCLK < 16000000){
			SYSCTL->MEMTIM0 = (SYSCTL->MEMTIM0 &~0x03EF03EF) + (0x0<<22) + (0x0<<21) + (0x0<<16) + (0x0<<6) + (0x0<<5) + (0x0);
		} else if(SYSCLK == 16000000){
			// FBCHT/EBCHT = 0, FBCE/EBCE = 1, FWS/EWS = 0
			SYSCTL->MEMTIM0 = (SYSCTL->MEMTIM0 &~0x03EF03EF) + (0x0<<22) + (0x1<<21) + (0x0<<16) + (0x0<<6) + (0x1<<5) + (0x0);
		} else if(SYSCLK <= 40000000){
			// FBCHT/EBCHT = 2, FBCE/EBCE = 0, FWS/EWS = 1
			SYSCTL->MEMTIM0 = (SYSCTL->MEMTIM0 &~0x03EF03EF) + (0x2<<22) + (0x0<<21) + (0x1<<16) + (0x2<<6) + (0x0<<5) + (0x1);
    } else if(SYSCLK <= 60000000){
			// FBCHT/EBCHT = 3, FBCE/EBCE = 0, FWS/EWS = 2
			SYSCTL->MEMTIM0 = (SYSCTL->MEMTIM0 &~0x03EF03EF) + (0x3<<22) + (0x0<<21) + (0x2<<16) + (0x3<<6) + (0x0<<5) + (0x2);
		} else if(SYSCLK <= 80000000){
			// FBCHT/EBCHT = 4, FBCE/EBCE = 0, FWS/EWS = 3
			SYSCTL->MEMTIM0 = (SYSCTL->MEMTIM0 &~0x03EF03EF) + (0x4<<22) + (0x0<<21) + (0x3<<16) + (0x4<<6) + (0x0<<5) + (0x3);
		} else if(SYSCLK <= 100000000){
			// FBCHT/EBCHT = 5, FBCE/EBCE = 0, FWS/EWS = 4
			SYSCTL->MEMTIM0 = (SYSCTL->MEMTIM0 &~0x03EF03EF) + (0x5<<22) + (0x0<<21) + (0x4<<16) + (0x5<<6) + (0x0<<5) + (0x4);
		} else if(SYSCLK <= 120000000){
			// FBCHT/EBCHT = 6, FBCE/EBCE = 0, FWS/EWS = 5
			SYSCTL->MEMTIM0 = (SYSCTL->MEMTIM0 &~0x03EF03EF) + (0x6<<22) + (0x0<<21) + (0x5<<16) + (0x6<<6) + (0x0<<5) + (0x5);
		}	
		
		while((SYSCTL->PLLSTAT & 0x00000001) == 0){
		}
		
		SYSCTL->RSCLKCFG |= 0x10000000 | 0x80000000 |  PSYSDIV;

		
		/*****************   PLL (FIM)  *****************/
		SysTick_Config(SYSCLK / 1000);     							// Setup SysTick for 1 msec   

	  // ADC 
		ADC0_InitTimer0ATrigger();

		/* preset */
		memset(TimerDownCnt, 0, sizeof(TimerDownCnt));
		TimerDownCnt[TIMER_DOWN_SERIAL] = 300;
		
		// Modbus
		memset(&Modbus,0,sizeof(Modbus));
		Modbus.Address = 0x05; 
		send_step = 0;
		
		// serial init
	  SER_Initialize();	
	
		SerialData = '0';	
		seq = 0;

		//*** ciclica
		while(1)
		{
				if(Modbus.Request)
				{				
						switch(send_step)
						{
								case 0:
										Modbus.SendD[0] = Modbus.Address;
										Modbus.SendD[1] = Modbus.Function;
										Modbus.SendD[2] = 2;						
										Modbus.DataAdress = Modbus.DataAdr0 * 256 + Modbus.DataAdr1;
										if(Modbus.DataAdress > 10)
										{
												Modbus.Request = 0;
												break;
										}				
										Modbus.SendD[3] = Modbus.Data[Modbus.DataAdress];
										Modbus.SendD[4] = Modbus.Data[Modbus.DataAdress + 1];
										crcValue = crc(Modbus.SendD, 0, 5);
										crc0 = (crcValue & 0xFF00) >> 8;
										Modbus.SendD[5] = (char)crc0;
										crc1 = (crcValue & 0x00FF);
										Modbus.SendD[6] = (char)crc1; 
										sCnt = 0;
										send_step = 1;
								break;
								case 1:
										if(sCnt<7)
										{	
												SER_PutChar(Modbus.SendD[sCnt]);
												sCnt++;
										}
										else
										{
												printf("");
 											  Modbus.Request = 0;
												send_step = 0;
										}
								break;
						}					
				}
				init = 0;
		}
}

void MainTask()
{
		int i;

		for(i=0;i<MAX_NR_AI;i++)
		{
				AI.AIVolt[i] = 1000 * ((float)AI.AIDig[i] * (float)3.3)/4096; // alimetacao 3.3V : 12 bits (4096)
		}
		AI.TempSensor = 147.5 - (0.075* AI.AIVolt[AI_TEMP]);
		
		Modbus.Data[0] = (AI.AIVolt[AI_PE2] & 0xFF00) >> 8;
		Modbus.Data[1] = (AI.AIVolt[AI_PE2] & 0x00FF);

	
		if(!init)
		{
				/* voltage update*/
				if((TimerDownCnt[TIMER_DOWN_SERIAL]==0) && (!Modbus.Request))
				{
						TimerDownCnt[TIMER_DOWN_SERIAL] = 300;
//						printf("PE2: %d [V] | Temp: %d [C] \n\r", (int)AI.AIVolt[AI_PE2], (int) AI.TempSensor);							
				}						
		}
}

