#include "gc_periph_config.h"

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/pwr.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/cm3/cortex.h>
#include <libopencm3/stm32/gpio.h>
#include <stdint.h>


void clock_init(void)
{
	// Set clock to 168 MHz (external 8 MHz generator input from on-board ST-Link
	// multiplied with PLL
	// For more detailed description, see example "168mhz_extclk.c" in 06_clock_system

	rcc_osc_bypass_enable(RCC_HSE);		// bypass load capacitors used for crystals
	rcc_osc_on(RCC_HSE);				// enable High-Speed External clock (HSE)
	while (!rcc_is_osc_ready(RCC_HSE));	// trap until external clock is detected as stable

	rcc_osc_off(RCC_PLL);		// Disable PLL before configuring it

	// * Set PLL multiplication factor as specified at page 226 RM
	// PLLM = 4		-- 8/4 = 2 MHz input to PLL mul stage
	// PLLN = 168   -- F<main> = 2 * 168 = 336 MHz
	// PLLP = 2		-- F<genout> = 336 / 2 = 168 MHz for our CPU and AHB
	// PLLQ = 7		-- F<Qdomain> = 336 / 7 = 48 MHz exactly
	rcc_set_main_pll_hse(4, 168, 2, 7, 0);		// with input from HSE to PLL
	rcc_css_disable();		// Disable clock security system
	rcc_osc_on(RCC_PLL);				// Enable PLL
	while (!rcc_is_osc_ready(RCC_PLL)); // Wait for PLL out clock to stabilize

	// Set all prescalers.
	rcc_set_hpre(RCC_CFGR_HPRE_DIV_NONE);	// AHB = 168 / 1 = 168 MHz
    rcc_set_ppre1(RCC_CFGR_PPRE_DIV_4);		// APB1 = FAHB / 4 = 168 / 4 = 42 MHz  (<= 42 MHz)
    rcc_set_ppre2(RCC_CFGR_PPRE_DIV_2);		// APB2 = FAHB / 2 = 168 / 2 = 84 MHz  (<= 84 MHz)

	// Enable caches. Flash is slow (around 30 MHz) and CPU is fast (168 MHz)
	flash_dcache_enable();
	flash_icache_enable();

	flash_set_ws(FLASH_ACR_LATENCY_7WS); // IMPORTANT! We must increase flash wait states (latency)


    rcc_set_sysclk_source(RCC_CFGR_SW_PLL);		// Select PLL as AHB bus (and CPU clock) source
	rcc_wait_for_sysclk_status(RCC_PLL);		// Wait for clock domain to be changed to PLL input

	// set by hand since we've not used rcc_clock_setup_pll
	rcc_ahb_frequency = 168000000ul;
	rcc_apb1_frequency = rcc_ahb_frequency / 4;
	rcc_apb2_frequency = rcc_ahb_frequency / 2;
	rcc_osc_off(RCC_HSI);		// Disable internal 16 MHz RC oscillator (HSI)
}


// ToDo: make separate API for SPI
static void flash_init_spi(void)
{
    // Setup GPIO

    // Our SST25VF016B memory chip has maximum clock frequency of 80 MHz, so set speed to high
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOB);
    rcc_periph_clock_enable(RCC_GPIOD);
    // Pins directly assigned to SPI peripheral
    gpio_set_output_options(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, 1 << 5);
    gpio_set_af(GPIOA, GPIO_AF5, 1 << 5);
    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, 1 << 5);

    gpio_set_output_options(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, (1 << 5) | (1 << 4));
    gpio_set_af(GPIOB, GPIO_AF5, (1 << 5) | (1 << 4));
    gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, (1 << 5) | (1 << 4));
    // CS Pin we drive manually
    gpio_set_output_options(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, 1 << 7);
    gpio_mode_setup(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, 1 << 7);
    gpio_set(GPIOD, 1 << 7);

    rcc_periph_clock_enable(RCC_SPI1);
    // Disable SPI1 before configuring
    spi_disable(SPI1);		// not required here, SPI is disabled after Reset
    // SPI1 belongs to APB2 (86 MHz frequency)
    // We have /2, /4, /8, /16, /32, /64, /128 and /256 prescalers
    // Our SPI flash can work up to 50 MHz ... 80 MHz clock (depending on part marking)
    // But to be able to capture data with logic analyzer, which has maximum frequency of 24 MHz,
    // set spi baudrate to /32, which gives us 86/32 = 2.6 MHz SCLK frequency
    spi_set_baudrate_prescaler(SPI1, SPI_CR1_BR_FPCLK_DIV_2);
    // Our MCU is master and flash chip is slave
    spi_set_master_mode(SPI1);
    // We work in full duplex (simultaneous transmit and receive)
    spi_set_full_duplex_mode(SPI1);
    // Data frame format is 8 bit, not 16
    spi_set_dff_8bit(SPI1);
    // No CRC 03H, fcalculation is required
    spi_disable_crc(SPI1);
    // Our flash chip requires Most Significant Bit first (datasheet p. 5, Figure 3. SPI Protocol)
    spi_send_msb_first(SPI1);
    // Flash chip can work in Mode 0 (polarity 0, phase 0) and Mode 3 (polarity 1, phase 1)
    // But the chip inputs data on rising edge and outputs data on falling edge, so
    // MCU is better to read data on rising edge and output on falling edge -- select mode 3
    spi_set_clock_polarity_1(SPI1);
    spi_set_clock_phase_1(SPI1);
    // Set hardware control of NSS pin. Because disabling it can cause master to become slave
    // depending on NSS input state.
    // Normally NSS will be hard-wired to slave. But in our case we have other pin connected to
    // slave CS and should drive it manually
    spi_enable_ss_output(SPI1);

    spi_enable(SPI1);
}



void periph_clock_init(void)
{
    rcc_periph_clock_enable(RCC_GPIOE);		// lcd is connected to port E
	rcc_periph_clock_enable(RCC_GPIOD);		// DISCOVERY LEDS are connected to Port E
}
