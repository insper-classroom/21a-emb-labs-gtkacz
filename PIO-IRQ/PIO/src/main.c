#include "asf.h"
#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"

/************************************************************************/
/* defines                                                              */
/************************************************************************/

// LED
#define LED_PIO      PIOC
#define LED_PIO_ID   ID_PIOC
#define LED_IDX      8
#define LED_IDX_MASK (1 << LED_IDX)

// Botoes

// Botao 1
#define BUT1_PIO      PIOD
#define BUT1_PIO_ID   ID_PIOD
#define BUT1_IDX      28
#define BUT1_IDX_MASK (1 << BUT1_IDX)
// Botao 2
#define BUT2_PIO      PIOC
#define BUT2_PIO_ID   ID_PIOC
#define BUT2_IDX      31
#define BUT2_IDX_MASK (1 << BUT2_IDX)
// Botao 3
#define BUT3_PIO      PIOA
#define BUT3_PIO_ID   ID_PIOA
#define BUT3_IDX      19
#define BUT3_IDX_MASK (1 << BUT3_IDX)

/*  Interrupt Edge detection is active. */
#define PIO_IT_EDGE             (1u << 6)

/************************************************************************/
/* variaveis globais                                                    */
/************************************************************************/
int contador = 0;
/* flag */
volatile char but1_flag;
volatile char but2_flag;
volatile char but3_flag;
volatile int frq = 300;

/************************************************************************/
/* prototype                                                            */
/************************************************************************/
void io_init(void);
void pisca_led(int n, int t);
void para_led(void);
void but1_callback(void);
void but2_callback(void);
void but3_callback(void);

/************************************************************************/
/* funções                                                              */
/************************************************************************/
void but1_callback(void)
{
	if (!pio_get(BUT1_PIO, PIO_INPUT, BUT1_IDX_MASK)) {
		frq -= 100;
		but1_flag = 1;
		}
	
}

void but2_callback(void)
{
	if (!pio_get(BUT2_PIO, PIO_INPUT, BUT2_IDX_MASK)) {
		but2_flag = 1;
	}
	
}

void but3_callback(void)
{
	if (!pio_get(BUT3_PIO, PIO_INPUT, BUT3_IDX_MASK)) {
		frq += 100;
		but3_flag = 1;
	}
	
}

//para o led de piscar
void para_led(void){
	pio_clear(LED_PIO, LED_IDX_MASK);
}

// pisca led N vez no periodo T
void pisca_led(int n, int t){
	// condicao para t <= 0, se nao houver tal condicao
	// o led fica aceso ininterruptamente
	if (t <= 0){
		for (int i=0;i<n;i++){
			if (but2_flag){
				para_led();
				but2_flag = 0;
				return;
			}
			pio_clear(LED_PIO, LED_IDX_MASK);
			delay_ms(100);
			pio_set(LED_PIO, LED_IDX_MASK);
			delay_ms(100);
		}
	}
	else {
		for (int i=0;i<n;i++){
			if (but2_flag){
				para_led();
				but2_flag = 0;
				return;
			}
			pio_clear(LED_PIO, LED_IDX_MASK);
			delay_ms(t);
			pio_set(LED_PIO, LED_IDX_MASK);
			delay_ms(t);
		}
	}
	
}

// Inicializa botao 1 do OLED com interrupcao
void io_init(void)
{

	// Configura led
	pmc_enable_periph_clk(LED_PIO_ID);
	pio_configure(LED_PIO, PIO_OUTPUT_0, LED_IDX_MASK, PIO_DEFAULT);

	// Inicializa clock do periférico PIO responsavel pelo botao
	pmc_enable_periph_clk(BUT1_PIO_ID);
	
	// ------ BOTAO 1------
	pio_configure(BUT1_PIO, PIO_INPUT, BUT1_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_debounce_filter(BUT1_PIO, BUT1_IDX_MASK, 60);

	// Configura interrupção no pino referente ao botao e associa
	// função de callback caso uma interrupção for gerada
	// a função de callback é a: but_callback()
	pio_handler_set(BUT1_PIO,
	BUT1_PIO_ID,
	BUT1_IDX_MASK,
	PIO_IT_EDGE,
	but1_callback);

	// Ativa interrupção
	pio_enable_interrupt(BUT1_PIO, BUT1_IDX_MASK);

	// Configura NVIC para receber interrupcoes do PIO do botao
	// com prioridade 4 (quanto mais próximo de 0 maior)
	NVIC_EnableIRQ(BUT1_PIO_ID);
	NVIC_SetPriority(BUT1_PIO_ID, 4); // Prioridade 4
	
	// ------ BOTAO 2------
	pio_configure(BUT2_PIO, PIO_INPUT, BUT2_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_debounce_filter(BUT2_PIO, BUT2_IDX_MASK, 60);

	// Configura interrupção no pino referente ao botao e associa
	// função de callback caso uma interrupção for gerada
	// a função de callback é a: but_callback()
	pio_handler_set(BUT2_PIO,
	BUT2_PIO_ID,
	BUT2_IDX_MASK,
	PIO_IT_EDGE,
	but2_callback);

	// Ativa interrupção
	pio_enable_interrupt(BUT2_PIO, BUT2_IDX_MASK);

	// Configura NVIC para receber interrupcoes do PIO do botao
	// com prioridade 4 (quanto mais próximo de 0 maior)
	NVIC_EnableIRQ(BUT2_PIO_ID);
	NVIC_SetPriority(BUT2_PIO_ID, 3); // Prioridade 4
	
	// ------ BOTAO 3------
	pio_configure(BUT3_PIO, PIO_INPUT, BUT3_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_debounce_filter(BUT3_PIO, BUT3_IDX_MASK, 60);

	// Configura interrupção no pino referente ao botao e associa
	// função de callback caso uma interrupção for gerada
	// a função de callback é a: but_callback()
	pio_handler_set(BUT3_PIO,
	BUT3_PIO_ID,
	BUT3_IDX_MASK,
	PIO_IT_EDGE,
	but3_callback);

	// Ativa interrupção
	pio_enable_interrupt(BUT3_PIO, BUT3_IDX_MASK);

	// Configura NVIC para receber interrupcoes do PIO do botao
	// com prioridade 4 (quanto mais próximo de 0 maior)
	NVIC_EnableIRQ(BUT3_PIO_ID);
	NVIC_SetPriority(BUT3_PIO_ID, 4); // Prioridade 4
}

int main (void)
{
	board_init();
	sysclk_init();
	delay_init();
	
	// configura botao com interrupcao
	io_init();
	
	// Init OLED
	gfx_mono_ssd1306_init();
	
  /* Insert application code here, after the board has been initialized. */
	while(1) {
		if (but1_flag){
			if (frq <= 0){
				char format0[20];
				sprintf(format0, "%d ms", 100);
				gfx_mono_draw_string(format0, 5,16, &sysfont);
			}
			else {
				char format0[20];
				sprintf(format0, "%d ms", frq);
				gfx_mono_draw_string(format0, 5,16, &sysfont);
			}
			
			pisca_led(5, frq);
			but1_flag = 0;
			
		} else if (but2_flag){
			para_led();
			but2_flag = 0;
		} else if (but3_flag){
			if (frq <= 0){
				char format0[20];
				sprintf(format0, "%d ms", 100);
				gfx_mono_draw_string(format0, 5,16, &sysfont);
			}
			else {
				char format0[20];
				sprintf(format0, "%d ms", frq);
				gfx_mono_draw_string(format0, 5,16, &sysfont);
			}
			
			pisca_led(5, frq);
			but3_flag = 0;
		}
		
		// Entra em sleep mode
		// Código 'trava' aqui até ser 'acordado'
		pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);
	}
}
