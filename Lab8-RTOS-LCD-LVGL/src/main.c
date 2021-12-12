/************************************************************************/
/* includes                                                             */
/************************************************************************/

#include <asf.h>
#include <string.h>
#include "ili9341.h"
#include "lvgl.h"
#include "touch/touch.h"
LV_FONT_DECLARE(dseg35);
LV_FONT_DECLARE(dseg50);
LV_FONT_DECLARE(dseg70);

/************************************************************************/
/* LCD / LVGL                                                           */
/************************************************************************/

#define LV_HOR_RES_MAX          (320)
#define LV_VER_RES_MAX          (240)

/*A static or global variable to store the buffers*/
static lv_disp_draw_buf_t disp_buf;

/*Static or global buffer(s). The second buffer is optional*/
static lv_color_t buf_1[LV_HOR_RES_MAX * LV_VER_RES_MAX];
static lv_disp_drv_t disp_drv;          /*A variable to hold the drivers. Must be static or global.*/
static lv_indev_drv_t indev_drv;

static lv_style_t style;

static lv_obj_t *btn_power;
static lv_obj_t *btn_m;
static lv_obj_t *btn_config;
static lv_obj_t *btn_up;
static lv_obj_t *btn_down;

lv_obj_t *labelFloor;
lv_obj_t *labelTemp;
lv_obj_t *labelTime;

typedef struct  {
	uint32_t year;
	uint32_t month;
	uint32_t day;
	uint32_t week;
	uint32_t hour;
	uint32_t minute;
	uint32_t second;
} calendar;

/************************************************************************/
/* RTOS                                                                 */
/************************************************************************/

#define TASK_LCD_STACK_SIZE                (1024*6/sizeof(portSTACK_TYPE))
#define TASK_LCD_STACK_PRIORITY            (tskIDLE_PRIORITY)

extern void vApplicationStackOverflowHook(xTaskHandle *pxTask,  signed char *pcTaskName);
extern void vApplicationIdleHook(void);
extern void vApplicationTickHook(void);
extern void vApplicationMallocFailedHook(void);
extern void xPortSysTickHandler(void);

extern void vApplicationStackOverflowHook(xTaskHandle *pxTask, signed char *pcTaskName) {
	printf("stack overflow %x %s\r\n", pxTask, (portCHAR *)pcTaskName);
	for (;;) {	}
}

extern void vApplicationIdleHook(void) { }

extern void vApplicationTickHook(void) { }

extern void vApplicationMallocFailedHook(void) {
	configASSERT( ( volatile void * ) NULL );
}

/************************************************************************/
/* lvgl                                                                 */
/************************************************************************/

static void event_handler(lv_event_t * e) {
	lv_event_code_t code = lv_event_get_code(e);

	if(code == LV_EVENT_CLICKED) {
		LV_LOG_USER("Clicked");
	}
	else if(code == LV_EVENT_VALUE_CHANGED) {
		LV_LOG_USER("Toggled");
	}
}

void lv_ex_btn_1(void) {
	lv_obj_t * label;

	lv_obj_t * btn1 = lv_btn_create(lv_scr_act());
	lv_obj_add_event_cb(btn1, event_handler, LV_EVENT_ALL, NULL);
	lv_obj_align(btn1, LV_ALIGN_CENTER, 0, -40);

	label = lv_label_create(btn1);
	lv_label_set_text(label, "Corsi");
	lv_obj_center(label);

	lv_obj_t * btn2 = lv_btn_create(lv_scr_act());
	lv_obj_add_event_cb(btn2, event_handler, LV_EVENT_ALL, NULL);
	lv_obj_align(btn2, LV_ALIGN_CENTER, 0, 40);
	lv_obj_add_flag(btn2, LV_OBJ_FLAG_CHECKABLE);
	lv_obj_set_height(btn2, LV_SIZE_CONTENT);

	label = lv_label_create(btn2);
	lv_label_set_text(label, "Toggle");
	lv_obj_center(label);
}

static void event_handler_power(lv_event_t *e){
	lv_event_code_t code = lv_event_get_code(e);

	if (code == LV_EVENT_CLICKED)
	{
		LV_LOG_USER("Clicked");
	}
	else if (code == LV_EVENT_VALUE_CHANGED)
	{
		LV_LOG_USER("Toggled");
	}
}

static void event_handler_m(lv_event_t *e){
	lv_event_code_t code = lv_event_get_code(e);

	if (code == LV_EVENT_CLICKED)
	{
		LV_LOG_USER("Clicked");
	}
	else if (code == LV_EVENT_VALUE_CHANGED)
	{
		LV_LOG_USER("Toggled");
	}
}

static void event_handler_config(lv_event_t *e){
	lv_event_code_t code = lv_event_get_code(e);

	if (code == LV_EVENT_CLICKED)
	{
		LV_LOG_USER("Clicked");
	}
	else if (code == LV_EVENT_VALUE_CHANGED)
	{
		LV_LOG_USER("Toggled");
	}
}

static void event_handler_up(lv_obj_t *obj, lv_event_t *e){
	lv_event_code_t code = lv_event_get_code(e);

	if (code == LV_EVENT_CLICKED)
	{
		char *c = lv_label_get_text(labelFloor);
		int temp = atoi(c);
		lv_label_set_text_fmt(labelFloor, "%02d", temp + 1);
		LV_LOG_USER("Clicked");
	}
	else if (code == LV_EVENT_VALUE_CHANGED)
	{
		LV_LOG_USER("Toggled");
	}
}

static void event_handler_down(lv_obj_t *obj, lv_event_t *e){
	lv_event_code_t code = lv_event_get_code(e);

	if (code == LV_EVENT_CLICKED)
	{
		char *c = lv_label_get_text(labelFloor);
		int temp = atoi(c);
		lv_label_set_text_fmt(labelFloor, "%02d", temp - 1);
		LV_LOG_USER("Clicked");
	}
	else if (code == LV_EVENT_VALUE_CHANGED)
	{
		LV_LOG_USER("Toggled");
	}
}

void lv_power(void){
	// Style
	lv_style_init(&style);
	lv_style_set_bg_color(&style, lv_color_black());
	//lv_style_set_border_width(&style, 2);

	// Config
	btn_power = lv_btn_create(lv_scr_act());
	lv_obj_add_event_cb(btn_power, event_handler_power, LV_EVENT_ALL, NULL);
	lv_obj_align(btn_power, LV_ALIGN_BOTTOM_LEFT, 10, -20);
	lv_obj_set_width(btn_power, 50);
	lv_obj_set_height(btn_power, 40);
	lv_obj_add_style(btn_power, &style, 0);

	// Label
	static lv_obj_t *label_power;
	label_power = lv_label_create(btn_power);
	lv_label_set_text(label_power, "[  " LV_SYMBOL_POWER "");
	lv_obj_center(label_power);
}

void lv_m(void){
	// Config
	btn_m = lv_btn_create(lv_scr_act());
	lv_obj_add_event_cb(btn_m, event_handler_m, LV_EVENT_ALL, NULL);
	lv_obj_align_to(btn_m, btn_power, LV_ALIGN_RIGHT_MID, 40, -12);
	lv_obj_set_width(btn_m, 50);
	lv_obj_set_height(btn_m, 40);
	lv_obj_add_style(btn_m, &style, 0);

	// Label
	static lv_obj_t *label_m;
	label_m = lv_label_create(btn_m);
	lv_label_set_text(label_m, "| M |");
	lv_obj_center(label_m);
}

void lv_config(void){
	// Config
	btn_config = lv_btn_create(lv_scr_act());
	lv_obj_add_event_cb(btn_config, event_handler_config, LV_EVENT_ALL, NULL);
	lv_obj_align_to(btn_config, btn_m, LV_ALIGN_RIGHT_MID, 40, -12);
	lv_obj_set_width(btn_config, 50);
	lv_obj_set_height(btn_config, 40);
	lv_obj_add_style(btn_config, &style, 0);

	// Label
	static lv_obj_t *label_config;
	label_config = lv_label_create(btn_config);
	lv_label_set_text(label_config, "" LV_SYMBOL_SETTINGS "  ]");
	lv_obj_center(label_config);
}

void lv_down(void){
	// Config
	btn_down = lv_btn_create(lv_scr_act());
	lv_obj_add_event_cb(btn_down, event_handler_down, LV_EVENT_ALL, NULL);
	lv_obj_align(btn_down, LV_ALIGN_BOTTOM_RIGHT, -10, -20);
	lv_obj_set_width(btn_down, 50);
	lv_obj_set_height(btn_down, 40);
	lv_obj_add_style(btn_down, &style, 0);

	// Label
	static lv_obj_t *label_down;
	label_down = lv_label_create(btn_down);
	lv_label_set_text(label_down, "" LV_SYMBOL_DOWN "  ]");
	lv_obj_center(label_down);
}

void lv_up(void){
	// Config
	btn_up = lv_btn_create(lv_scr_act());
	lv_obj_add_event_cb(btn_up, event_handler_up, LV_EVENT_ALL, NULL);
	lv_obj_align_to(btn_up, btn_down, LV_ALIGN_LEFT_MID, -70, -12);
	lv_obj_set_width(btn_up, 50);
	lv_obj_set_height(btn_up, 40);
	lv_obj_add_style(btn_up, &style, 0);

	// Label
	static lv_obj_t *label_up;
	label_up = lv_label_create(btn_up);
	lv_label_set_text(label_up, "[  " LV_SYMBOL_UP "");
	lv_obj_center(label_up);
}

void lv_therm(void){
	labelFloor = lv_label_create(lv_scr_act());
	lv_obj_align(labelFloor, LV_ALIGN_LEFT_MID, 35, -20);
	lv_obj_set_style_text_font(labelFloor, &dseg70, LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(labelFloor, lv_color_white(), LV_STATE_DEFAULT);
	lv_label_set_text_fmt(labelFloor, "%02d", 25);

	labelTime = lv_label_create(lv_scr_act());
	lv_obj_align(labelTime, LV_ALIGN_TOP_RIGHT, -35, 10);
	lv_obj_set_style_text_font(labelTime, &dseg35, LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(labelTime, lv_color_white(), LV_STATE_DEFAULT);
	lv_label_set_text_fmt(labelTime, "%02d:%02d", rtc_initial.hour, rtc_initial.minute);

	labelTemp = lv_label_create(lv_scr_act());
	lv_obj_align(labelTemp, LV_ALIGN_RIGHT_MID, -35, -20);
	lv_obj_set_style_text_font(labelTemp, &dseg50, LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(labelTemp, lv_color_white(), LV_STATE_DEFAULT);
	lv_label_set_text_fmt(labelTemp, "%02d", 23);
}

/************************************************************************/
/* TASKS                                                                */
/************************************************************************/

static void task_lcd(void *pvParameters){
	int px, py;

	lv_ex_btn_1();

	for (;;)  {
		lv_tick_inc(50);
		lv_task_handler();
		vTaskDelay(50);
	}
}

void RTC_Handler(void){
	uint32_t ul_status = rtc_get_status(RTC);

	/* seccond tick	*/
	if ((ul_status & RTC_SR_SEC) == RTC_SR_SEC) {
		uint32_t h, m, s;
		rtc_get_time(RTC,&h,&m,&s);
		
		if (s%2 == 0){
			lv_label_set_text_fmt(labelTime, "%02d:%02d", h, m);
		}
		else{
			lv_label_set_text_fmt(labelTime, "%02d %02d", h, m);
		}
		
		flag_sec = 1;
	}
	
	/* Time or date alarm */
	if ((ul_status & RTC_SR_ALARM) == RTC_SR_ALARM){
		flag_rtc = 1;
	}
	
	rtc_clear_status(RTC, RTC_SCCR_SECCLR);
	rtc_clear_status(RTC, RTC_SCCR_ALRCLR);
	rtc_clear_status(RTC, RTC_SCCR_ACKCLR);
	rtc_clear_status(RTC, RTC_SCCR_TIMCLR);
	rtc_clear_status(RTC, RTC_SCCR_CALCLR);
	rtc_clear_status(RTC, RTC_SCCR_TDERRCLR);
}

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

static void task_lcd(void *pvParameters)
{
	int px, py;

	lv_power();
	lv_m();
	lv_config();
	lv_down();
	lv_up();
	lv_therm();

	for (;;){
		lv_tick_inc(50);
		lv_task_handler();
		vTaskDelay(50);
	}
}

/************************************************************************/
/* configs                                                              */
/************************************************************************/

static void configure_lcd(void){
	/**LCD pin configure on SPI*/
	pio_configure_pin(LCD_SPI_MISO_PIO, LCD_SPI_MISO_FLAGS);  //
	pio_configure_pin(LCD_SPI_MOSI_PIO, LCD_SPI_MOSI_FLAGS);
	pio_configure_pin(LCD_SPI_SPCK_PIO, LCD_SPI_SPCK_FLAGS);
	pio_configure_pin(LCD_SPI_NPCS_PIO, LCD_SPI_NPCS_FLAGS);
	pio_configure_pin(LCD_SPI_RESET_PIO, LCD_SPI_RESET_FLAGS);
	pio_configure_pin(LCD_SPI_CDS_PIO, LCD_SPI_CDS_FLAGS);
	
	ili9341_init();
	ili9341_backlight_on();
}

static void configure_console(void){
	const usart_serial_options_t uart_serial_options = {
		.baudrate = USART_SERIAL_EXAMPLE_BAUDRATE,
		.charlength = USART_SERIAL_CHAR_LENGTH,
		.paritytype = USART_SERIAL_PARITY,
		.stopbits = USART_SERIAL_STOP_BIT,
	};

	/* Configure console UART. */
	stdio_serial_init(CONSOLE_UART, &uart_serial_options);

	/* Specify that stdout should not be buffered. */
	setbuf(stdout, NULL);
}

/************************************************************************/
/* port lvgl                                                            */
/************************************************************************/

void my_flush_cb(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p){
	ili9341_set_top_left_limit(area->x1, area->y1);   ili9341_set_bottom_right_limit(area->x2, area->y2);
	ili9341_copy_pixels_to_screen(color_p,  (area->x2 + 1 - area->x1) * (area->y2 + 1 - area->y1));
	
	/* IMPORTANT!!!
	* Inform the graphics library that you are ready with the flushing*/
	lv_disp_flush_ready(disp_drv);
}

void my_input_read(lv_indev_drv_t * drv, lv_indev_data_t*data){
	int px, py, pressed;
	
	if (readPoint(&px, &py))
		data->state = LV_INDEV_STATE_PRESSED;
	else
		data->state = LV_INDEV_STATE_RELEASED; 
	
	data->point.x = px;
	data->point.y = py;
}

void configure_lvgl(void){
	lv_init();
	lv_disp_draw_buf_init(&disp_buf, buf_1, NULL, LV_HOR_RES_MAX * LV_VER_RES_MAX);
	
	lv_disp_drv_init(&disp_drv);            /*Basic initialization*/
	disp_drv.draw_buf = &disp_buf;          /*Set an initialized buffer*/
	disp_drv.flush_cb = my_flush_cb;        /*Set a flush callback to draw to the display*/
	disp_drv.hor_res = LV_HOR_RES_MAX;      /*Set the horizontal resolution in pixels*/
	disp_drv.ver_res = LV_VER_RES_MAX;      /*Set the vertical resolution in pixels*/

	lv_disp_t * disp;
	disp = lv_disp_drv_register(&disp_drv); /*Register the driver and save the created display objects*/
	
	/* Init input on LVGL */
	lv_indev_drv_init(&indev_drv);
	indev_drv.type = LV_INDEV_TYPE_POINTER;
	indev_drv.read_cb = my_input_read;
	lv_indev_t * my_indev = lv_indev_drv_register(&indev_drv);
}

/************************************************************************/
/* main                                                                 */
/************************************************************************/
int main(void){
	/* board and sys init */
	board_init();
	sysclk_init();
	configure_console();

	/* LCd, touch and lvgl init*/
	configure_lcd();
	configure_touch();
	configure_lvgl();
	
	RTC_init(RTC, ID_RTC, rtc_initial, RTC_IER_ALREN | RTC_IER_SECEN);

	/* Create task to control oled */
	if (xTaskCreate(task_lcd, "LCD", TASK_LCD_STACK_SIZE, NULL, TASK_LCD_STACK_PRIORITY, NULL) != pdPASS){
		printf("Failed to create lcd task\r\n");
	}

	/* Start the scheduler. */
	vTaskStartScheduler();

	while (1){
	}
}