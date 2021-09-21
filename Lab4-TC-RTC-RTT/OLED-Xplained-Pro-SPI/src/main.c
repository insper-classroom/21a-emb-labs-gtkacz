#include "asf.h"
#include <time.h>
#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"

/************************************************************************/
/* defines                                                              */
/************************************************************************/

/**
 *  Informacoes para o RTC
 *  poderia ser extraida do __DATE__ e __TIME__
 *  ou ser atualizado pelo PC.
 */
typedef struct  {
  uint32_t year;
  uint32_t month;
  uint32_t day;
  uint32_t week;
  uint32_t hour;
  uint32_t minute;
  uint32_t second;
} calendar;

#define LED1_PIO      PIOA
#define LED1_PIO_ID   ID_PIOA
#define LED1_IDX      0
#define LED1_IDX_MASK (1 << LED1_IDX)

#define LED2_PIO      PIOC
#define LED2_PIO_ID   ID_PIOC
#define LED2_IDX      30
#define LED2_IDX_MASK (1 << LED2_IDX)

#define LED3_PIO      PIOB
#define LED3_PIO_ID   ID_PIOB
#define LED3_IDX      2
#define LED3_IDX_MASK (1 << LED3_IDX)

#define BUT1_PIO      PIOD
#define BUT1_PIO_ID   ID_PIOD
#define BUT1_IDX      28
#define BUT1_IDX_MASK (1 << BUT1_IDX)

/*  Interrupt Edge detection is active. */
#define PIO_IT_EDGE             (1u << 6)

/************************************************************************/
/* variaveis globais                                                    */
/************************************************************************/
/* flag */
volatile char but1_flag = 0;
volatile char flag_tc = 0;
volatile Bool f_rtt_alarme = false;
volatile char flag_rtc = 0;

/************************************************************************/
/* prototype                                                            */
/************************************************************************/
void io_init(void);
void LED1_init(int estado);
void LED2_init(int estado);
void LED3_init(int estado);
void TC_init(Tc * TC, int ID_TC, int TC_CHANNEL, int freq);
void pisca_led1(int n, int t);
void pisca_led2(int n, int t);
void pisca_led3(int n, int t);
void but1_callback(void);
void pin_toggle(Pio *pio, uint32_t mask);
static void RTT_init(uint16_t pllPreScale, uint32_t IrqNPulses);
void RTC_init(Rtc *rtc, uint32_t id_rtc, calendar t, uint32_t irq_type);

/************************************************************************/
/* Handlers                                                             */
/************************************************************************/

void TC1_Handler(void){
	volatile uint32_t ul_dummy;

	ul_dummy = tc_get_status(TC0, 1);

	/* Avoid compiler warning */
	UNUSED(ul_dummy);

	flag_tc = 1;
}

void RTT_Handler(void)
{
	uint32_t ul_status;

	/* Get RTT status - ACK */
	ul_status = rtt_get_status(RTT);

	/* IRQ due to Time has changed */
	if ((ul_status & RTT_SR_RTTINC) == RTT_SR_RTTINC) {
		//f_rtt_alarme = false;
		pin_toggle(LED2_PIO, LED2_IDX_MASK);    // BLINK Led
		
	}

	/* IRQ due to Alarm */
	if ((ul_status & RTT_SR_ALMS) == RTT_SR_ALMS) {
		// pin_toggle(LED_PIO, LED_IDX_MASK);    // BLINK Led
		f_rtt_alarme = true;                  // flag RTT alarme
	}
}

/**
* \brief Interrupt handler for the RTC. Refresh the display.
*/
void RTC_Handler(void)
{
	uint32_t ul_status = rtc_get_status(RTC);
	
	/* Time or date alarm */
	if ((ul_status & RTC_SR_ALARM) == RTC_SR_ALARM) {
		flag_rtc = 1;
	}
	
	rtc_clear_status(RTC, RTC_SCCR_SECCLR);
	rtc_clear_status(RTC, RTC_SCCR_ALRCLR);
	rtc_clear_status(RTC, RTC_SCCR_ACKCLR);
	rtc_clear_status(RTC, RTC_SCCR_TIMCLR);
	rtc_clear_status(RTC, RTC_SCCR_CALCLR);
	rtc_clear_status(RTC, RTC_SCCR_TDERRCLR);
}

/************************************************************************/
/* fun��es                                                              */
/************************************************************************/
void but1_callback(void)
{
	if (!pio_get(BUT1_PIO, PIO_INPUT, BUT1_IDX_MASK)) {
		but1_flag = 1;
	}
	
}

void pisca_led1(int n, int t){
  for (int i=0;i<n;i++){
    pio_clear(LED1_PIO, LED1_IDX_MASK);
    delay_ms(t);
    pio_set(LED1_PIO, LED1_IDX_MASK);
    delay_ms(t);
  }
}

void pisca_led2(int n, int t){
  for (int i=0;i<n;i++){
    pio_clear(LED2_PIO, LED2_IDX_MASK);
    delay_ms(t);
    pio_set(LED2_PIO, LED2_IDX_MASK);
    delay_ms(t);
  }
}

void pisca_led3(int n, int t){
  for (int i=0;i<n;i++){
    pio_clear(LED3_PIO, LED3_IDX_MASK);
    delay_ms(t);
    pio_set(LED3_PIO, LED3_IDX_MASK);
    delay_ms(t);
  }
}

void LED1_init(int estado){
	pmc_enable_periph_clk(LED1_PIO_ID);
	pio_set_output(LED1_PIO, LED1_IDX_MASK, estado, 0, 0);
};

void LED2_init(int estado){
	pmc_enable_periph_clk(LED2_PIO_ID);
	pio_set_output(LED2_PIO, LED2_IDX_MASK, estado, 0, 0);
};

void LED3_init(int estado){
	pmc_enable_periph_clk(LED3_PIO_ID);
	pio_set_output(LED3_PIO, LED3_IDX_MASK, estado, 0, 0);
};


void TC_init(Tc * TC, int ID_TC, int TC_CHANNEL, int freq){
	uint32_t ul_div;
	uint32_t ul_tcclks;
	uint32_t ul_sysclk = sysclk_get_cpu_hz();

	uint32_t channel = 1;

	pmc_enable_periph_clk(ID_TC);

	tc_find_mck_divisor(freq, ul_sysclk, &ul_div, &ul_tcclks, ul_sysclk);
	tc_init(TC, TC_CHANNEL, ul_tcclks | TC_CMR_CPCTRG);
	tc_write_rc(TC, TC_CHANNEL, (ul_sysclk / ul_div) / freq);

	NVIC_EnableIRQ((IRQn_Type) ID_TC);
	tc_enable_interrupt(TC, TC_CHANNEL, TC_IER_CPCS);

	/* Inicializa o canal 0 do TC */
	tc_start(TC, TC_CHANNEL);
}

void pin_toggle(Pio *pio, uint32_t mask){
	if(pio_get_output_data_status(pio, mask))
	pio_clear(pio, mask);
	else
	pio_set(pio,mask);
}

static float get_time_rtt(){
	uint ul_previous_time = rtt_read_timer_value(RTT);
}

static void RTT_init(uint16_t pllPreScale, uint32_t IrqNPulses)
{
	uint32_t ul_previous_time;

	/* Configure RTT for a 1 second tick interrupt */
	rtt_sel_source(RTT, false);
	rtt_init(RTT, pllPreScale);
	
	ul_previous_time = rtt_read_timer_value(RTT);
	while (ul_previous_time == rtt_read_timer_value(RTT));
	
	rtt_write_alarm_time(RTT, IrqNPulses+ul_previous_time);

	/* Enable RTT interrupt */
	NVIC_DisableIRQ(RTT_IRQn);
	NVIC_ClearPendingIRQ(RTT_IRQn);
	NVIC_SetPriority(RTT_IRQn, 4);
	NVIC_EnableIRQ(RTT_IRQn);
	rtt_enable_interrupt(RTT, RTT_MR_ALMIEN | RTT_MR_RTTINCIEN);
}

/**
* Configura o RTC para funcionar com interrupcao de alarme
*/
void RTC_init(Rtc *rtc, uint32_t id_rtc, calendar t, uint32_t irq_type){
	/* Configura o PMC */
	pmc_enable_periph_clk(ID_RTC);

	/* Default RTC configuration, 24-hour mode */
	rtc_set_hour_mode(rtc, 0);

	/* Configura data e hora manualmente */
	rtc_set_date(rtc, t.year, t.month, t.day, t.week);
	rtc_set_time(rtc, t.hour, t.minute, t.second);

	/* Configure RTC interrupts */
	NVIC_DisableIRQ(id_rtc);
	NVIC_ClearPendingIRQ(id_rtc);
	NVIC_SetPriority(id_rtc, 4);
	NVIC_EnableIRQ(id_rtc);

	/* Ativa interrupcao via alarme */
	rtc_enable_interrupt(rtc,  irq_type);
}

// Inicializa botao 1 do OLED com interrupcao
void io_init(void)
{
	pmc_enable_periph_clk(LED1_PIO_ID);
	pio_configure(LED1_PIO, PIO_OUTPUT_0, LED1_IDX_MASK, PIO_DEFAULT);
	
	pmc_enable_periph_clk(LED2_PIO_ID);
	pio_configure(LED2_PIO, PIO_OUTPUT_0, LED2_IDX_MASK, PIO_DEFAULT);
	
	pmc_enable_periph_clk(LED3_PIO_ID);
	pio_configure(LED3_PIO, PIO_OUTPUT_0, LED3_IDX_MASK, PIO_DEFAULT);

	pmc_enable_periph_clk(BUT1_PIO_ID);
	
	pio_configure(BUT1_PIO, PIO_INPUT, BUT1_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_debounce_filter(BUT1_PIO, BUT1_IDX_MASK, 60);

	pio_handler_set(BUT1_PIO,
	BUT1_PIO_ID,
	BUT1_IDX_MASK,
	PIO_IT_EDGE,
	but1_callback);

	pio_enable_interrupt(BUT1_PIO, BUT1_IDX_MASK);

	// Configura NVIC para receber interrupcoes do PIO do botao
	// com prioridade 4 (quanto mais pr�ximo de 0 maior)
	NVIC_EnableIRQ(BUT1_PIO_ID);
	NVIC_SetPriority(BUT1_PIO_ID, 4); // Prioridade 4
}

int main (void){
	board_init();
	sysclk_init();
	io_init();

	// Init OLED
	gfx_mono_ssd1306_init();
  

	/* Disable the watchdog */
	WDT->WDT_MR = WDT_MR_WDDIS;

	LED1_init(1);
	LED2_init(1);
	LED3_init(1);

	/** Configura timer TC0, canal 1 */
	TC_init(TC0, ID_TC1, 1, 4);
	
	// Inicializa RTT com IRQ no alarme.
	f_rtt_alarme = true;

	/* configura hora do RTC */
	time_t t = time(NULL);
	struct tm *ptm = localtime(&t);
	calendar rtc_initial = {tm.tm_year, 3, 19, 12, 15, 45 ,1};
	RTC_init(RTC, ID_RTC, rtc_initial, RTC_IER_ALREN);
	rtc_set_date_alarm(RTC, 1, rtc_initial.month, 1, rtc_initial.day);
	rtc_set_time_alarm(RTC, 1, rtc_initial.hour, 1, rtc_initial.minute, 1, rtc_initial.second + 20);
	RTC_init(RTC, ID_RTC, rtc_initial, RTC_IER_ALREN | RTC_IER_SECEN);

  /* Insert application code here, after the board has been initialized. */
	while(1){
		if(flag_tc){
			pisca_led1(1,10);
			flag_tc = 0;
		}
		if (f_rtt_alarme){
      
		  uint16_t pllPreScale = (int) (((float) 32768) / 4.0);
		  uint32_t irqRTTvalue = 16;
      
		  RTT_init(pllPreScale, irqRTTvalue);         
      
		  f_rtt_alarme = false;
		}
		if (but1_flag){
			/** Configura RTC */
			calendar rtc_initial_button = {2018, 3, 19, 12, 15, 45 ,1};
			RTC_init(RTC, ID_RTC, rtc_initial_button, RTC_IER_ALREN);
			rtc_set_date_alarm(RTC, 1, rtc_initial_button.month, 1, rtc_initial_button.day);
			rtc_set_time_alarm(RTC, 1, rtc_initial_button.hour, 1, rtc_initial_button.minute, 1, rtc_initial_button.second + 20);
			but1_flag = 0;
		}
		if (flag_rtc){
			pisca_led3(1, 100);
			flag_rtc = 0;
		}  
		pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);
	}
}
