
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
static inline uint8_t bcd_to_dec(uint8_t tens, uint8_t units)
{
    return tens * 10 + units;
}


void rtc_set_date(rtc_date *date)
{
   // set time
    uint32_t tr = RTC_TR;
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


void rtc_get_date(rtc_date *date)
{
    uint8_t tens = 0, units = 0;
    // get hours
    tens = ((RTC_TR & (RTC_TR_HT_MASK << RTC_TR_HT_SHIFT)) >> 20);
	units = ((RTC_TR & (RTC_TR_HU_MASK << RTC_TR_HU_SHIFT)) >> 16);
	date->hour = bcd_to_dec(tens, units);
    // get minutes
	tens = ((RTC_TR & (RTC_TR_MNT_MASK << RTC_TR_MNT_SHIFT)) >> 12);
	units = ((RTC_TR & (RTC_TR_MNU_MASK << RTC_TR_MNU_SHIFT)) >> 8);
	date->minute = bcd_to_dec(tens, units);
    // get second
	tens = ((RTC_TR & (RTC_TR_ST_MASK << RTC_TR_ST_SHIFT)) >> 4);
	units = ((RTC_TR & (RTC_TR_SU_MASK << RTC_TR_SU_SHIFT)) >> 0);
	date->second = bcd_to_dec(tens, units);
    // get notation
    date->notation = (RTC_TR & RTC_TR_PM);
    // get year
    tens = ((RTC_DR & (RTC_DR_YT_MASK << RTC_DR_YT_SHIFT)) >> 20);
	units = ((RTC_DR & (RTC_DR_YU_MASK << RTC_DR_YU_SHIFT)) >> 16);
	date->year = bcd_to_dec(tens, units);
    // get week day
    // units = ((RTC_DR & (RTC_DR_WDU_MASK << RTC_DR_WDU_SHIFT)) >> 15);
    // date->week_day = units;
    // get month
	tens = ((RTC_DR & (RTC_DR_MT_MASK << RTC_DR_MT_SHIFT)) >> 12);
	units = ((RTC_DR & (RTC_DR_MU_MASK << RTC_DR_MU_SHIFT)) >> 8);
	date->month = bcd_to_dec(tens, units);
    // get day
	tens = ((RTC_DR & (RTC_DR_DT_MASK << RTC_DR_DT_SHIFT)) >> 4);
	units = ((RTC_DR & (RTC_DR_DU_MASK << RTC_DR_DU_SHIFT)) >> 0);
	date->day= bcd_to_dec(tens, units);
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
