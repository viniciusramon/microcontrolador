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

#define 					S_MAX_NR_EIXOS				5

// Eixos
#define 					S_EIXO_T1			0
#define 					S_EIXO_T2			1
#define 					S_EIXO_P1			2
#define 					S_EIXO_P2			3
#define 					S_EIXO_PL			4



typedef struct{
		long					Cnt;		// timer up counter
		char					Enable;	// habilita
} timerUp_typ;

typedef struct{
			int			xmin;
			int			xmax;
			int			ymin;
			int			ymax;
} scal_typ;

typedef struct{
			scal_typ		scal;
			int 				sp0;
			int					sp1;
			char				mover;
			int					apos;
			int 				avel;
			int 				avelmedia;
			int					adig;
} eixo_typ;

/*----------------------------------------------------------------------------
		Global Variables
 *----------------------------------------------------------------------------*/
char							step;
timerUp_typ				TIMER_UP[S_MAX_NR_TIMER_UP];
eixo_typ					eixo[S_MAX_NR_EIXOS];

/*----------------------------------------------------------------------------
    SysTick_Handler
 *----------------------------------------------------------------------------*/
void SysTick_Handler(void) {
		int i;
		for(i=0; i < S_MAX_NR_TIMER_UP; i++)
		{
				if(TIMER_UP[i].Enable)
				{
						TIMER_UP[i].Cnt ++;
				}
		}
}

int Posicao(eixo_typ eixo)
{
		int calc;
		calc =  ((eixo.adig - eixo.scal.xmin) * ((eixo.scal.ymax - eixo.scal.ymin)/(eixo.scal.xmax-eixo.scal.xmin))) + eixo.scal.ymin;
		return calc;
}


/*----------------------------------------------------------------------------
    MAIN
 *----------------------------------------------------------------------------*/
int main()
{
		int i;
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

	
		/*********************** CYCLIC ***********************/
		while(1)
		{
				for(i=0; i< S_MAX_NR_EIXOS; i++)
				{
						eixo[i].apos = Posicao(eixo[i]);
				}
			
		} // while(1)		
}


