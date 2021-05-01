/*----------------------------------------------------------------------------
		
 *----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
		Includes
 *----------------------------------------------------------------------------*/
 #include <TM4C129.h>

/*----------------------------------------------------------------------------
		Defines
 *----------------------------------------------------------------------------*/
#define PN0				(1UL << 0)						

#define PJ0				(1UL << 0)						
#define PJ1				(1UL << 1)

#define	PortN			GPION->DATA
#define	PortF			GPIOF_AHB->DATA

#define 					S_MAX_NR_TIMER_UP			10

typedef struct{
		char				S1;		// primeiro sensor - ativacao
		char				S2;		// segundo  sensor - comparacao
		char				L1;		// lampada indicador de velocidade excedida
} IO_typ;

typedef struct{
		int 				VMin;			// velocidade minima
		int					VMax;			// velocidade máxima
		int					tempo;		// tempo para calculo da velocidade
		int					VAtual;		// velocidade calculada atual
} veloc_typ;

typedef struct{
		IO_typ				IO;			// entradas e saidas
		veloc_typ 		Veloc;	// parametros de velocidade
} radar_typ;

typedef struct{
		long					Cnt;		// timer up counter
		char					Enable;	// habilita
} timerUp_typ;

/*----------------------------------------------------------------------------
		Global Variables
 *----------------------------------------------------------------------------*/
char					step;
radar_typ			RADAR;
timerUp_typ		TIMER_UP[S_MAX_NR_TIMER_UP];


/*----------------------------------------------------------------------------
    SysTick_Handler
 *----------------------------------------------------------------------------*/
void SysTick_Handler(void) {
		int i;
		for(i=0; i < 11; i++)
		{
				if(TIMER_UP[i].Enable)
				{
						TIMER_UP[i].Cnt ++;
				}
		}
}

long ConvVelTempo (long vel)
{
		return (10/ ((vel/ 3.6))) * 1000;
}

/*----------------------------------------------------------------------------
    MAIN
 *----------------------------------------------------------------------------*/
int main()
{
		// Local variables
		long tempoVMax;
		long tempoVMin;
	
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

		RADAR.Veloc.VMax = 72;
		RADAR.Veloc.VMin = 10;
	
		
		/*********************** CYCLIC ***********************/
		while(1)
		{
			tempoVMax =  ConvVelTempo(RADAR.Veloc.VMax);
			tempoVMin =  ConvVelTempo(RADAR.Veloc.VMin);

			// read inputs
			RADAR.IO.S1  = (((GPIOJ_AHB->DATA) & PJ0))?0:1;		
			RADAR.IO.S2  = (((GPIOJ_AHB->DATA) & PJ1))?0:1;			
		
				switch(step)
				{
						case 0:
								if(RADAR.IO.S1)
								{
										TIMER_UP[0].Enable = 1;
										TIMER_UP[0].Cnt = 0;
										step =1;
								}
						break;
								
						case 1:
								if(RADAR.IO.S2)
								{
										if(TIMER_UP[0].Cnt <= tempoVMax)
										{
												GPION->DATA |= PN0;
												TIMER_UP[0].Cnt = 0;
												step = 2;
										}
								}
								if(TIMER_UP[0].Cnt > tempoVMin)
								{
										step = 0;
								}
						break;

						case 2:
								if(TIMER_UP[0].Cnt >= 1000)
								{
										GPION->DATA &= ~PN0;
										step = 0;
								}
						break;
				} // switch(step)	

		} // while(1)		
}




