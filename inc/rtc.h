/**
 * rtc API is an abstraction over RTC periphey it provides
 * support for RTC STM32F40xx
 */
#include "macro.h"

#include <stdint.h>
#include <libopencm3/stm32/rcc.h>


/**
 * RTC time and date type
 */
struct rtc_date{
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
    /**
     * AM/PM notation
     * 0: AM or 24-hour format
     * 1: PM
     */
    uint8_t notation;
    /** Year: 0-99*/
	uint8_t year;
    /** Week day units
     * 000: forbidden
     * 001: Monday
     * ...
     * 111: Sunday
    */
    uint8_t week_day;
	uint8_t month;
	uint8_t day;
};


typedef struct rtc_date rtc_date;

 /**
  * Binary-Coded Decimal
  */
struct BCD{
	uint8_t tens;
	uint8_t units;
};


typedef struct BCD BCD;


/**
 * Initialize RTC for STM32F40xx
 * @date: RTC time and date type (:c:type:`rtc_date`)
 * @rcc_osc: clock source for RTC:
 * 00: - no clock
 * 01: - LSE oscillator clock used as the RTC clock -> RCC_LSE
 * 10: - LSI oscillator clock used as the RTC clock -> RCC_LSI
 * 11: - HSE oscillator clock divided by a programmable prescaler
 *		 used as the RTC clock -> RCC_HSE
 */
void RTC_init(rtc_date *date, enum rcc_osc rtc_osc);


/**
 * Set time and date to RTC time register and RTC date register
 * @date: RTC time and date type (:c:type:`rtc_date`)
*/
void rtc_set_date(rtc_date *date);


//ToDo: Add description
BCD rtc_get_BCD_hour(void);


BCD rtc_get_BCD_minute(void);


BCD rtc_get_BCD_second(void);


uint8_t rtc_get_notation(void);


BCD rtc_get_BCD_year(void);


uint8_t rtc_get_BCD_week_day(void);


BCD rtc_get_BCD_month(void);


BCD rtc_get_BCD_day(void);



/**
 * Read time and date from RTC time register and RTC date register
 * to :c:type:`rtc_date`
 * @date: RTC time and date type (:c:type:`rtc_date`)
*/
void rtc_get_date(rtc_date *date);
