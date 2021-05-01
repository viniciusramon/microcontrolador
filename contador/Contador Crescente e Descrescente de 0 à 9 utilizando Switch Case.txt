////Includes////

////Definições////

////Declaração de Variaveis Globais////

int main()
{
		////Declaração de Variavel Locais////
	
		int cnt; //contador
		int estado;
	
		////Inicialização////
	
		cnt = 0;
		estado=0;
	
	
		////Parte Ciclica////
	
		while(1)
		{
			switch(estado)
			{
				case 0: //incrementa
				if(cnt<9)
				{
				cnt++;
				}
				else
				{
					estado=1;
				}
				break;
				
				case 1: //decrementa
					if(cnt>0)
					{
						cnt--;
					}
					else
					{
						estado=0;
					}		
					break;
			}
		}
}
