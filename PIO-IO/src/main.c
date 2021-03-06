/**
 * 5 semestre - Eng. da Computação - Insper
 * Rafael Corsi - rafael.corsi@insper.edu.br
 *
 * Projeto 0 para a placa SAME70-XPLD
 *
 * Objetivo :
 *  - Introduzir ASF e HAL
 *  - Configuracao de clock
 *  - Configuracao pino In/Out
 *
 * Material :
 *  - Kit: ATMEL SAME70-XPLD - ARM CORTEX M7
 */

/************************************************************************/
/* includes                                                             */
/************************************************************************/

#include "asf.h"

#define LED_PIO           PIOC                 // periferico que controla o LED
#define ID_PIOC        12                  // ID do periférico PIOC (controla LED)
#define LED_PIO_IDX       8                    // ID do LED no PIO
#define LED_PIO_IDX_MASK  (1 << LED_PIO_IDX)   // Mascara para CONTROLARMOS o LED
#define LED_PIO_ID  ID_PIOC  // ID do periférico PIOC (controla LED)

// Configuracoes do botao
#define BUT_PIO          PIOA
#define BUT_PIO_ID		 10
#define BUT_PIO_IDX      11
#define BUT_PIO_IDX_MASK (1u << BUT_PIO_IDX)



/************************************************************************/
/* constants                                                            */
/************************************************************************/

/************************************************************************/
/* variaveis globais                                                    */
/************************************************************************/

/************************************************************************/
/* prototypes                                                           */
/************************************************************************/

void init(void);

/************************************************************************/
/* interrupcoes                                                         */
/************************************************************************/

/************************************************************************/
/* funcoes                                                              */
/************************************************************************/

// Função de inicialização do uC
void init(void){
	// Initialize the board clock
	sysclk_init();

	// Desativa WatchDog Timer
	WDT->WDT_MR = WDT_MR_WDDIS;
	
	// Ativa o PIO na qual o LED foi conectado
	// para que possamos controlar o LED.
	pmc_enable_periph_clk(LED_PIO_ID);
	
	//Inicializa PC8 como saída
	pio_set_output(LED_PIO, LED_PIO_IDX_MASK, 0, 0, 0);
	
	// Inicializa PIO do botao
	pmc_enable_periph_clk(BUT_PIO_ID);
	
	// configura pino ligado ao botão como entrada com um pull-up.
	pio_set_input(BUT_PIO, BUT_PIO_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	
	// Ativa o Pull-up, resistor alimentado para vcc
	pio_pull_up(BUT_PIO, BUT_PIO_IDX_MASK, 1);
	
}



/************************************************************************/
/* Main                                                                 */
/************************************************************************/

void blink(int n){
	for (int i = 0; i < n; i++){
		_pio_set(PIOC, LED_PIO_IDX_MASK);
		delay_ms(1000);
		_pio_clear(PIOC, LED_PIO_IDX_MASK);
		delay_ms(1000);
	}
}

// Funcao principal chamada na inicalizacao do uC.
int main(void){
	init();

	// super loop
	// aplicacoes embarcadas não devem sair do while(1
	int pressed = 0;
	
	while (1){
		int input = pio_get(BUT_PIO, PIO_INPUT, BUT_PIO_IDX_MASK);
		int output = pio_get(BUT_PIO, PIO_OUTPUT_0, BUT_PIO_IDX_MASK);
		
		if ((output == 0) && (input == 0)){
			pressed = 1;
		}
		else{
			if(pressed){
				blink(5);
				pressed = 0;
			}
			else{
				_pio_set(PIOC, LED_PIO_IDX_MASK);
			}
		}
	}
	return 0;
}
