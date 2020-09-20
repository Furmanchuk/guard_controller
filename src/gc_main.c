
#include "lcd_hd44780.h"
#include "pin.h"
#include "tick.h"
#include "macro.h"
#include "rtc.h"
#include "gc_periph_config.h"
#include "gc_utils.h"

#include <libprintf/printf.h>
#include <libopencm3/cm3/cortex.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/pwr.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/rtc.h>
#include <libopencm3/stm32/spi.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>


#include "semihosting.h"

static struct sk_lcd lcd = {
	.pin_group_data = &sk_io_lcd_data,
	.pin_rs = &sk_io_lcd_rs,
	.pin_en = &sk_io_lcd_en,
	.pin_rw = &sk_io_lcd_rw,
	.pin_bkl = &sk_io_lcd_bkl,
	.set_backlight_func = NULL,
	.delay_func_us = NULL,
	.delay_func_ms = &sk_tick_delay_ms,
	.is4bitinterface = true,
	.charmap_func = &sk_lcd_charmap_rus_cp1251
};


static rtc_date date = {
	.hour 		= 17,
	.minute 	= 48,
	.second 	= 00,
	.notation 	= 0, // 24-hour format
	.year 		= 20,
	.week_day 	= 1,
	.month 		= 9,
	.day 		= 14
};



static states current_state = M_PUT_ON_GUEARD;
static menu_func_t current_menu;

static const uint16_t pin_code = 0x1234;
static const uint16_t s_pin = 0x9876;


//static bool IsArming;

static void lcd_putstring(struct sk_lcd *lcd, const char *str)	// dummy example
{
	char *ptr = str;
	while (*ptr != '\0') {
		sk_lcd_putchar(lcd, *ptr);
		ptr++;
	}
}

static void lcd_print_menu(char *str1, char *str2)
{
	sk_lcd_cmd_clear(&lcd);
	sk_lcd_cmd_setaddr(&lcd, 0x00, false);
	char buffer[20];
	snprintf(buffer, sizeof(buffer), "%s", str1);
	lcd_putstring(&lcd, buffer);

	if (str2 == NULL)
		return;
	sk_lcd_cmd_setaddr(&lcd, 0x40, false);
	snprintf(buffer, sizeof(buffer), "%s", str2);
	lcd_putstring(&lcd, buffer);
}


bool get_signal(sk_button *menu_bth)
{
	sk_tick_delay_ms(130);
	if(sk_pin_read(sk_io_btn_up)){
		sk_pin_toggle(sk_io_led_orange);
		*menu_bth = BTN_UP;
		return true;
	}
	if(sk_pin_read(sk_io_btn_down)){
		sk_pin_toggle(sk_io_led_green);
		*menu_bth =  BTN_DOWN;
		return true;
	}
	if(sk_pin_read(sk_io_btn_mid)){
		// sk_pin_toggle(sk_io_led_blue);
		*menu_bth =  BTN_MID;
		return true;
	}
	if(sk_pin_read(sk_io_btn_left)){
		sk_pin_toggle(sk_io_led_blue);
		*menu_bth =  BTN_LEFT;
		return true;
	}
	if(sk_pin_read(sk_io_btn_right)){
		sk_pin_toggle(sk_io_led_red);
		*menu_bth =  BTN_RIGHT;
		return true;
	}
	return false;
}


bool pin_checking(const uint16_t true_PIN, const uint16_t entered_PIN,  uint8_t *choose)
{
	char buffer[20];
	if (true_PIN != entered_PIN){
		sk_lcd_cmd_clear(&lcd);
		sk_lcd_cmd_setaddr(&lcd, 0x00, false);
		lcd_putstring(&lcd, "Wrong PIN!");
		(*choose)--;

		// // !!!!!!!
		// if (!(*choose)){
		// 	return false;
		// }

		sk_lcd_cmd_setaddr(&lcd, 0x40, false);
		snprintf(buffer, sizeof(buffer), "%d %s", *choose, "attempts left!");
		lcd_putstring(&lcd, buffer);
		sk_tick_delay_ms(2000);
		return false;
	}
	else{
		lcd_print_menu("Success", NULL);
		sk_tick_delay_ms(2000);
		return true;
	}
}


// helper fuction
void lcd_print_number(struct sk_lcd *lcd, uint8_t number, uint8_t pos, uint8_t addr)
{
	uint8_t offset = 48;
	sk_lcd_putchar(lcd, (char) (offset + number));
	sk_lcd_cmd_setaddr(lcd, addr, false);
}


//helper function
void lcd_enter_pin(struct sk_lcd *lcd, pin_t *pin, uint8_t *num_pos, sk_button btn)
{
	void pin_cursor_shift(struct sk_lcd *lcd, int8_t *pos, bool left_shift)
	{
		sk_lcd_putchar(lcd, (char)42);
		sk_lcd_cmd_setaddr(lcd, 0x40 + *num_pos, false);
		lcd_cursor_shift(num_pos, 0, 3, left_shift);
		sk_lcd_cmd_setaddr(lcd, 0x40 + *num_pos, false);
	}

	int8_t cursor_pos = *num_pos;
	uint8_t addr;
	switch (btn){
	case BTN_UP:
		set_number(&(pin->num[cursor_pos]), 0, 9, true);
		addr = 0x40 + cursor_pos;
		lcd_print_number(lcd, pin->num[cursor_pos], cursor_pos, addr);
		break;
	case BTN_DOWN:
		set_number(&(pin->num[cursor_pos]), 0, 9, false);
		addr = 0x40 + cursor_pos;
		lcd_print_number(lcd, pin->num[cursor_pos], cursor_pos, addr);
		break;
	case BTN_LEFT:
		pin_cursor_shift(lcd, num_pos, true);
		break;
	case BTN_RIGHT:
		pin_cursor_shift(lcd, num_pos, false);
		break;
	}
}


bool PIN_guard_menu(void)
{
	lcd_print_menu("Enter PIN:", NULL);
	sk_lcd_cmd_onoffctl(&lcd, true, true, true);
	sk_lcd_cmd_setaddr(&lcd, 0x40, false);
	int8_t num_pos = 0;
	pin_t enter_pin = {0};
	bool pin_check;
	sk_button menu_bth = BTN_UP;
	uint8_t choose_cnt = 3;
	while((menu_bth != BTN_MID) | choose_cnt){
		if (get_signal(&menu_bth)){
			lcd_enter_pin(&lcd, &enter_pin, &num_pos, menu_bth);
			if (menu_bth == BTN_MID){
				uint16_t _pin = pin_t_to_uint16_t(&enter_pin);
				pin_check = pin_checking(pin_code, _pin, &choose_cnt);

				if (!pin_check & (choose_cnt > 0))
				{
					num_pos = 0;
					enter_pin.pin = 0;
					lcd_print_menu("Enter PIN:", NULL);
					sk_lcd_cmd_setaddr(&lcd, 0x40, false);
				}else if (pin_check){
					break;
				}else{
					lcd_print_menu("Alarm!", NULL);
					sk_tick_delay_ms(2000);
					break;
				}
			}
		}
	}
	sk_lcd_cmd_onoffctl(&lcd, true, false, false);
	print_menu(current_state);
	return pin_check;
}

void print_status(sensor_status status)
{
	switch (status) {
	case SS_DS:
		// ToDo: read_sensor_status();
		lcd_print_menu("Door sensor", "Status __");
		break;
	case SS_GS:
		// ToDo: read_sensor_status();
		lcd_print_menu("Gas sensor", "Status __");
		break;
	}
}


void sensor_status_menu(void)
{
	sensor_status new_state, current_state;
	current_state = SS_DS;
	sk_button btn = BTN_MID;
	print_status(current_state);
	while(btn != BTN_LEFT){
		if (get_signal(&btn) & (btn != BTN_MID) & (btn != BTN_RIGHT)){
		new_state = FMS_sensor_status[current_state][btn];
		print_status(new_state);
		current_state = new_state;
		}
	}
	print_menu(current_state);
}

void display_log(void)
{
	sk_button btn = BTN_MID;
	lcd_print_menu("In log", NULL);
	while(btn != BTN_LEFT){
		if (get_signal(&btn)){
			switch (btn){
			case BTN_UP:
				lcd_print_menu("Press", "UP");
				break;
			case BTN_DOWN:
				lcd_print_menu("Press", "DOWN");
				break;
			}
		}
	}
	print_menu(current_state);
}

void read_time_date(date_format_t *time, date_format_t *date)
{
	BCD bcd_data;
	bcd_data = rtc_get_BCD_hour();
	time->num[0] = bcd_data.tens;
	time->num[1] = bcd_data.units;
	bcd_data = rtc_get_BCD_minute();
	time->num[2] = bcd_data.tens;
	time->num[3] = bcd_data.units;
	bcd_data = rtc_get_BCD_second();
	time->num[4] = bcd_data.tens;
	time->num[5] = bcd_data.units;
	bcd_data = rtc_get_BCD_year();
	date->num[0] = bcd_data.tens;
	date->num[1] = bcd_data.units;
	bcd_data = rtc_get_BCD_month();
	date->num[2] = bcd_data.tens;
	date->num[3] = bcd_data.units;
	bcd_data = rtc_get_BCD_day();
	date->num[4] = bcd_data.tens;
	date->num[5] = bcd_data.units;
}

void lcd_print_time(date_format_t *time, date_format_t *date)
{
	char buffer[20];
	sk_lcd_cmd_setaddr(&lcd, 0x00, false);
	snprintf(buffer, sizeof(buffer), "%d%d:%d%d:%d%d", time->num[0], time->num[1],
	time->num[2], time->num[3], time->num[4], time->num[5]);
	lcd_putstring(&lcd, buffer);
	sk_lcd_cmd_setaddr(&lcd, 0x40, false);
	snprintf(buffer, sizeof(buffer), "%d%d:%d%d:%d%d", date->num[0], date->num[1],
	date->num[2], date->num[3], date->num[4], date->num[5]);
	lcd_putstring(&lcd, buffer);
}


//
void write_time_data_to_rtc(date_format_t *time, date_format_t *date)
{
	rtc_date date_time;
	rtc_get_date(&date_time);

	date_time.hour 		= time->num[0] * 10 + time->num[1];
	date_time.minute 	= time->num[2] * 10 + time->num[3];
	date_time.second 	= time->num[4] * 10 + time->num[5];

	date_time.year 		= date->num[0] * 10 + date->num[1];
	date_time.month 	= date->num[2] * 10 + date->num[3];
	date_time.day 		= date->num[4] * 10 + date->num[5];
	RTC_init(&date_time, RCC_LSI);
}

// ToDo refactoring
void set_time_date_menu(void)
{
	date_format_t time = {0};
	date_format_t date = {0};
	read_time_date(&time, &date);
	sk_lcd_cmd_clear(&lcd);
	lcd_print_time(&time, &date);
	sk_lcd_cmd_onoffctl(&lcd, true, true, true);
	sk_lcd_cmd_setaddr(&lcd, 0x00, false);
	int8_t num_pos = 0, num;

	uint8_t lcd_addr = 0;
	sk_button btn = BTN_UP;
	while((btn != BTN_MID)){
		if (get_signal(&btn)){
			switch (btn){
			case BTN_UP:
				num = set_date_time(&time, &date, num_pos, true);
				if (num != -1){
					lcd_print_number(&lcd, num, num_pos, lcd_addr);
				}
				break;
			case BTN_DOWN:
				num = set_date_time(&time, &date, num_pos, false);
				if (num != -1){
					lcd_print_number(&lcd, num, num_pos, lcd_addr);
				}
				break;
			case BTN_LEFT:
				lcd_cursor_shift(&num_pos, 0, 15, true);
				lcd_addr = (num_pos > 7) ? (0x40 + num_pos - 8) : num_pos;
				sk_lcd_cmd_setaddr(&lcd, lcd_addr, false);
				break;
			case BTN_RIGHT:
				lcd_cursor_shift(&num_pos, 0, 15, false);
				lcd_addr = (num_pos > 7) ? (0x40 + num_pos - 8) : num_pos;
				sk_lcd_cmd_setaddr(&lcd, lcd_addr, false);
				break;
			case BTN_MID:
				write_time_data_to_rtc(&time, &date);
				print_menu(current_state);
				break;
			}
		}
	}
	sk_lcd_cmd_onoffctl(&lcd, true, false, false);
	print_menu(current_state);
	print_sevice_menu(current_state);
}


void print_sevice_menu(service_menu_state state)
{
	switch (state) {
	case SM_ADD_PIN:
		lcd_print_menu("Add new PIN", NULL);
		break;
	case SM_REMOVE_PIN:
		// read_sensor_status();
		lcd_print_menu("Remove PIN", NULL);
		break;
	case SM_CHANGE_SPIN:
		// read_sensor_status();
		lcd_print_menu("Change service", "PIN");
		break;
	}
}

void service_menu(void)
{
	if (!PIN_guard_menu()){
		return;
	}
	service_menu_state new_state, current_state;
	current_state = SM_ADD_PIN;
	sk_button btn = BTN_MID;
	print_sevice_menu(current_state);
	while(btn != BTN_LEFT){
		if (get_signal(&btn)){
		new_state = FMS_service_menu[current_state][btn];
		print_sevice_menu(new_state);
		current_state = new_state;
		}
	}
	print_menu(current_state);
}

void print_menu(states menu_state)
{
	switch (menu_state) {
	case M_PUT_ON_GUEARD:
		lcd_print_menu("Put on guard", NULL);
		current_menu = &PIN_guard_menu;
		break;
		case M_SENSOR_STAT:
		lcd_print_menu("Sensors status", NULL);
		current_menu = &sensor_status_menu;
		break;
	case M_DISPLAY_LOG:
		lcd_print_menu("Display log", NULL);
		current_menu = &display_log;
		break;
	case M_TIME_DATE:
		lcd_print_menu("Set time & data", NULL);
		current_menu = &set_time_date_menu;
		break;
	case M_SERVICE:
		lcd_print_menu("Service menu", NULL);
		current_menu = &service_menu;
		break;
	}
}


void internal_init(void)
{
	periph_clock_init();
	glsk_pins_init(false);
	sk_tick_init(16000000ul / 10000ul, ((2 << 2 | 0) << 4));

	RTC_init(&date, RCC_LSI);
	sk_tick_delay_ms(1000);
	for (int i = 0; i < 5; i++)
		sk_lcd_init(&lcd);
	sk_lcd_set_backlight(&lcd, 200);
}

int main(void)
{
	internal_init();

	sk_button current_signal ;
	states new_state;

	print_menu(current_state);
	while (1) {
		if (get_signal(&current_signal) /*& (current_signal == BTN_LEFT)*/){
			new_state = FMS_menu[current_state][current_signal];

			if ((current_signal == BTN_MID) | (current_signal == BTN_RIGHT)){
				current_menu();
			}
			else{

				print_menu(new_state);
				current_state = new_state;
			}
		}
	}
}
