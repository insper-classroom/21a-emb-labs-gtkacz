#include <asf.h>
#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"

// TRIG (Saida)
#define TRIG_PIO      PIOA
#define TRIG_PIO_ID   ID_PIOA
#define TRIG_IDX      6
#define TRIG_IDX_MASK (1 << TRIG_IDX)

// ECHO (entrada)
#define ECHO_PIO      PIOA
#define ECHO_PIO_ID   ID_PIOA
#define ECHO_IDX      3
#define ECHO_IDX_MASK (1 << ECHO_IDX)

/************************************************************************/
volatile uint32_t U = 0;
/************************************************************************/

/************************************************************************/
/* Prototypes                                                           */
/************************************************************************/
static void RTT_init(uint16_t pllPreScale, uint32_t IrqNPulses);


/************************************************************************/
/* handler / callbacks                                                  */
/************************************************************************/
void echo_callback(void){
	uint16_t pllPreScale = (int) (((float) 32768) / 8000);
	uint32_t irqRTTvalue = 8000; // define quantidade de tempo

	if (pio_get(ECHO_PIO, PIO_INPUT, ECHO_IDX_MASK)){
		RTT_init(pllPreScale, irqRTTvalue);
	}
	else{
		U = rtt_read_timer_value(RTT);
	}
}

void RTT_Handler(void){
	uint32_t ul_status;

	/* Get RTT status - ACK */
	ul_status = rtt_get_status(RTT);
}

void io_init(void){
	// Initialize the board clock
	sysclk_init();

	WDT->WDT_MR = WDT_MR_WDDIS;
O
	pmc_enable_periph_clk(ECHO_PIO_ID);
	pio_configure(ECHO_PIO, PIO_INPUT, ECHO_IDX_MASK, PIO_PULLUP);
	pio_set_debounce_filter(ECHO_PIO, ECHO_IDX_MASK, 60);
	pio_get_interrupt_status(ECHO_PIO);

	pio_enable_interrupt(ECHO_PIO, ECHO_IDX_MASK);

	pio_handler_set(ECHO_PIO,
	ECHO_PIO_ID,
	ECHO_IDX_MASK,
	PIO_IT_EDGE,
	echo_callback);

	NVIC_EnableIRQ(ECHO_PIO_ID);
	NVIC_SetPriority(ECHO_PIO_ID, 4);

	pmc_enable_periph_clk(TRIG_PIO_ID);
	pio_configure(TRIG_PIO, PIO_OUTPUT_0, TRIG_IDX_MASK, PIO_PULLUP);
}

static void RTT_init(uint16_t pllPreScale, uint32_t IrqNPulses)
{
	uint32_t ul_previous_time;

	/* Configure RTT for a 1 second tick interrupt */
	rtt_sel_source(RTT, false);
	rtt_init(RTT, pllPreScale);
	
	ul_previous_time = rtt_read_timer_value(RTT);
	while (ul_previous_time == rtt_read_timer_value(RTT));
}


/************************************************************************/
/* funcoes                                                              */
/************************************************************************/
void trig_pulse(){
	pio_set(TRIG_PIO, TRIG_IDX_MASK);
	delay_us(10);
	pio_clear(TRIG_PIO, TRIG_IDX_MASK);
}

/************************************************************************/
/* main                                                                 */
/************************************************************************/
int main (void)
{
	io_init();
	board_init();
	sysclk_init();
	delay_init();
	WDT->WDT_MR = WDT_MR_WDDIS;

	gfx_mono_ssd1306_init();
	float buffer [100];

	while(1) {
		trig_pulse();
		float t = (float)U/8000;
		float d = (340.0 * t * 100.0)/2.0;
		sprintf(buffer, "%f cm", d);
		gfx_mono_draw_string(buffer, 0, 0, &sysfont);
		//pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);
	}
}
