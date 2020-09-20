
#include "rtc.h"
#include <libopencm3/stm32/rtc.h>
#include <libopencm3/stm32/pwr.h>

static inline uint8_t get_bcd_tens(uint8_t value)
{
	return value / 10;
}


static inline uint8_t get_bcd_units(uint8_t value)
{
	return value % 10;
}


// Convert from BDC format to decimal
static inline uint8_t bcd_to_dec(BCD number)
{
    return number.tens * 10 + number.units;
}


void rtc_set_date(rtc_date *date)
{
   // set time
    uint32_t tr = 0;
    // set hours
    tr |= ((get_bcd_tens(date->hour) & RTC_TR_HT_MASK) << RTC_TR_HT_SHIFT);
    tr |= ((get_bcd_units(date->hour) & RTC_TR_HU_MASK) << RTC_TR_HU_SHIFT);
	// set minutes
	tr |= ((get_bcd_tens(date->minute) & RTC_TR_MNT_MASK) << RTC_TR_MNT_SHIFT);
	tr |= ((get_bcd_units(date->minute) & RTC_TR_MNU_MASK) << RTC_TR_MNU_SHIFT);
	// set seconds
	tr |= ((get_bcd_tens(date->second)  & RTC_TR_ST_MASK) << RTC_TR_ST_SHIFT);
	tr |= ((get_bcd_units(date->second) & RTC_TR_SU_MASK) << RTC_TR_SU_SHIFT);
    if (!date->notation)
        tr &= ~RTC_TR_PM;
    else
        tr |= RTC_TR_PM;
	RTC_TR = tr;
   // set date
    uint32_t dr = 0;// = RTC_DR;
    // set year
    dr |= ((get_bcd_tens(date->year) & RTC_DR_YT_MASK) << RTC_DR_YT_SHIFT);
	dr |= ((get_bcd_units(date->year) & RTC_DR_YU_MASK) << RTC_DR_YU_SHIFT);
    // set week day
	dr |= ((date->week_day & RTC_DR_WDU_MASK) << RTC_DR_WDU_SHIFT);
	// set month
    dr |= ((get_bcd_tens(date->month) & RTC_DR_MT_MASK) << RTC_DR_MT_SHIFT);
	dr |= ((get_bcd_units(date->month) & RTC_DR_MU_MASK) << RTC_DR_MU_SHIFT);
	// set day
	dr |= ((get_bcd_tens(date->day) & RTC_DR_DT_MASK) << RTC_DR_DT_SHIFT);
	dr |= ((get_bcd_units(date->day) & RTC_DR_DU_MASK) << RTC_DR_DU_SHIFT);

	RTC_DR = dr;
}


BCD rtc_get_BCD_hour(void)
{
	BCD hour;
	hour.tens = ((RTC_TR & (RTC_TR_HT_MASK << RTC_TR_HT_SHIFT)) >> 20);
	hour.units = ((RTC_TR & (RTC_TR_HU_MASK << RTC_TR_HU_SHIFT)) >> 16);
	return hour;
}


BCD rtc_get_BCD_minute(void)
{
	BCD minute;
	minute.tens = ((RTC_TR & (RTC_TR_MNT_MASK << RTC_TR_MNT_SHIFT)) >> 12);
	minute.units = ((RTC_TR & (RTC_TR_MNU_MASK << RTC_TR_MNU_SHIFT)) >> 8);
	return minute;
}


BCD rtc_get_BCD_second(void)
{
	BCD second;
	second.tens = ((RTC_TR & (RTC_TR_ST_MASK << RTC_TR_ST_SHIFT)) >> 4);
	second.units = ((RTC_TR & (RTC_TR_SU_MASK << RTC_TR_SU_SHIFT)) >> 0);
	return second;
}


uint8_t rtc_get_notation(void)
{
	return (RTC_TR & RTC_TR_PM);
}


BCD rtc_get_BCD_year(void)
{
	BCD year;
	year.tens = ((RTC_DR & (RTC_DR_YT_MASK << RTC_DR_YT_SHIFT)) >> 20);
	year.units = ((RTC_DR & (RTC_DR_YU_MASK << RTC_DR_YU_SHIFT)) >> 16);
	return year;
}


uint8_t rtc_get_BCD_week_day(void)
{
	return ((RTC_DR & (RTC_DR_WDU_MASK << RTC_DR_WDU_SHIFT)) >> 15);
}


BCD rtc_get_BCD_month(void)
{
	BCD month;
	month.tens = ((RTC_DR & (RTC_DR_MT_MASK << RTC_DR_MT_SHIFT)) >> 12);
	month.units = ((RTC_DR & (RTC_DR_MU_MASK << RTC_DR_MU_SHIFT)) >> 8);
	return month;
}


BCD rtc_get_BCD_day(void)
{
	BCD day;
	day.tens = ((RTC_DR & (RTC_DR_DT_MASK << RTC_DR_DT_SHIFT)) >> 4);
	day.units = ((RTC_DR & (RTC_DR_DU_MASK << RTC_DR_DU_SHIFT)) >> 0);
	return day;
}


void rtc_get_date(rtc_date *date)
{
	BCD bcd;
	// get hours
	bcd = rtc_get_BCD_hour();
	date->hour = bcd_to_dec(bcd);
    // get minutes
	bcd = rtc_get_BCD_minute();
	date->minute = bcd_to_dec(bcd);
    // get second
	bcd = rtc_get_BCD_second();
	date->second = bcd_to_dec(bcd);
    // get notation
    date->notation = rtc_get_notation();
    // get year
    bcd = rtc_get_BCD_year();
	date->year = bcd_to_dec(bcd);
    // get week day
    date->week_day = rtc_get_BCD_week_day();
    // get month
	bcd = rtc_get_BCD_month();
	date->month = bcd_to_dec(bcd);
    // get day
	bcd = rtc_get_BCD_day();
	date->day= bcd_to_dec(bcd);
}


static void rcc_bdcr_clk_sel(uint8_t clock_sorce)
{
    uint32_t reg = RCC_BDCR;
    reg &= ~(RCC_BDCR_RTCSEL_MASK << RCC_BDCR_RTCSEL_SHIFT);
    reg |= clock_sorce << RCC_BDCR_RTCSEL_SHIFT;
    RCC_BDCR = reg;
}


static void rtc_set_clock_sorce(enum rcc_osc rtc_osc)
{
    rcc_osc_on(rtc_osc);
    rcc_wait_for_osc_ready(rtc_osc);
    switch(rtc_osc) {
    case RCC_LSE:
        // rcc_osc_on(RCC_LSE);
        // rcc_wait_for_osc_ready(RCC_LSE);
        // External low-speed oscillator enable
        RCC_BDCR |= RCC_BDCR_LSEON;
        // Wait for oscillator ready
        while (!(RCC_BDCR & RCC_BDCR_LSERDY));
        // Select clock sorce for RTC
        rcc_bdcr_clk_sel(RCC_BDCR_RTCSEL_LSE);
        break;
    // LSI oscillator clock used as the RTC clock
    case RCC_LSI:
        // rcc_osc_on(RCC_LSI);
        // rcc_wait_for_osc_ready(RCC_LSI);
        // Internal low-speed oscillator enable
        RCC_CSR |= RCC_CSR_LSION;
        // Wait for oscillator ready
        while (!(RCC_CSR & RCC_CSR_LSIRDY));
        // Select clock sorce for RTC
        rcc_bdcr_clk_sel(RCC_BDCR_RTCSEL_LSI);
        break;
    case RCC_HSE:
        // rcc_osc_on(RCC_LSE);
        // rcc_wait_for_osc_ready(RCC_LSE);
        // HSE clock enable
        RCC_CR |= RCC_CR_HSEON;
        // Wait for oscillator ready
        while (!(RCC_CR & RCC_CR_HSERDY));
        // Select clock sorce for RTC
        rcc_bdcr_clk_sel(RCC_BDCR_RTCSEL_HSE);
        break;
    default:
        rcc_bdcr_clk_sel(RCC_BDCR_RTCSEL_NONE);
    }
}


void RTC_init(rtc_date *date, enum rcc_osc rtc_osc)
{
    // Powet interface clock enable
	rcc_periph_clock_enable(RCC_PWR);

	// RTC clock enable
	// if ((RCC_BDCR & RCC_BDCR_RTCEN))
	// 	return;


    // Disable Backup Domain Write Protection.
	// This allows backup domain registers to be changed.
	// These registers are write protected after a reset.
	pwr_disable_backup_domain_write_protect();
    // Backup domain reset,
	RCC_BDCR |= RCC_BDCR_BDRST;
	RCC_BDCR &= ~RCC_BDCR_BDRST;

    rtc_set_clock_sorce(rtc_osc);
    /* Enable the RTC clock */
	rcc_periph_clock_enable(RCC_RTC); // ->RCC_BDCR |= RCC_BDCR_RTCEN;
    // Unlock write access to the RTC registers
    rtc_unlock();
    // Set INIT bit to 1 in the RTC_ISR register to enter initialization mode
	RTC_ISR |= RTC_ISR_INIT;
    // Waiting for initialization flag to be set
	//sk_pin_set(sk_io_led_green, true);
	while(!(RTC_ISR & RTC_ISR_INITF));
    // Set asynchronous prescaler(PREDIV_A) to 127+1 and synchronous
	// prescaler(PREDIV_S) set to 250+1 to get frequency of 1 kHz
	rtc_set_prescaler((uint32_t)(249), (uint32_t)(127));
    rtc_set_date(date);
    // Exit the initialization mode by clearing the INIT bit.
	RTC_ISR &= ~(RTC_ISR_INITS);
	// Lock write access to the RTC registers
	rtc_lock();
    pwr_enable_backup_domain_write_protect();
    /* Wait for the synchronisation. */
	rtc_wait_for_synchro();
}
