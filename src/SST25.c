/*TODO: Add module description */
/**
 * Lib for work with SST25VF016B SPI flash
*/



#include "errors.h"
#include "pin.h"
#include "SST25.h"
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/gpio.h>
#include <stdbool.h>
#include <libopencm3/stm32/rcc.h>

#define SPI_FLASH SPI1

// BUSY bit of Software Status Register
#define BUSY_BIT (1 << 0)

/** Device Operation Instruction
 * Instruction| Description                 | Op Code Cycle|
 * RDSR         Read-Status-Register Code       0x05
 * Read         Read memory at 25 MHz           0x03
 * JEDEC-ID     JEDEC ID read                   0x9F
 * RDID         Read-ID                         0x90
 * Byte-program To program one data byte        0x02
 * EWSR         Enable-Write-Status-Register    0x50
 * WRSR         Write Status-Register           0x01
 * Chip-Erase   Erase Full Memoru Array         0x60
 * WREN         Write-enable                    0x06
 * WRDI         Write-disable                   0x04
 */


// Write byte to SPI TX buffer
static void flash_spi_write_byte(uint8_t data)
{
    spi_send(SPI_FLASH, data);
    spi_read(SPI_FLASH);
}


// Read byre from SPI RX buffer
static uint8_t flash_spi_read_byte(void)
{
    spi_send(SPI_FLASH, 0x00);
    return spi_read(SPI_FLASH);
}


static inline void ce_on(void)
{
    sk_pin_set(sk_io_spiflash_ce, false);
}


static inline void ce_off(void)
{
    sk_pin_set(sk_io_spiflash_ce, true);
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

void sk_flash_write_enable(void)
{
    ce_on();
    flash_spi_write_byte(0x06);
    ce_off();
}


void sk_flash_write_disable(void)
{
    ce_on();
    flash_spi_write_byte(0x04);
    ce_off();
}


// Read software status register SST25VF016B
uint8_t sk_flash_get_status(void)
{
    ce_on(); // CE# set to low
    flash_spi_write_byte(0x05);
    uint8_t flash_status = flash_spi_read_byte();
    ce_off();
    return flash_status;
}


void sk_flash_write_byte(uint32_t addr, uint8_t data)
{
    while((sk_flash_get_status() & BUSY_BIT) != 0);
    ce_on();
    flash_spi_write_byte(0x02);
    flash_spi_write_byte((uint8_t) (addr >> 16));
    flash_spi_write_byte((uint8_t) (addr >> 8));
    flash_spi_write_byte((uint8_t) addr);
    flash_spi_write_byte(data);
    ce_off();
}


void sk_flash_read_bytes(uint32_t addr, uint8_t *data, uint32_t size)
{
    // ? protection for this opertator
    while ((sk_flash_get_status() & BUSY_BIT) != 0);
    ce_on();
    flash_spi_write_byte(0x03);
    flash_spi_write_byte((uint8_t) (addr >> 16));
    flash_spi_write_byte((uint8_t) (addr >> 8));
    flash_spi_write_byte((uint8_t) addr);
    for (uint32_t i = 0; i < size; i++){
        data[i] = flash_spi_read_byte();
    }
    ce_off();
}


void sk_flash_arase(void)
{
    ce_on();
    flash_spi_write_byte(0x50);
    ce_off();
    ce_on();
    flash_spi_write_byte(0x01);
    // Clear software status register
    flash_spi_write_byte(0x00);
    ce_off();
    sk_flash_write_enable();
    while((sk_flash_get_status() & BUSY_BIT) != 0);
    ce_on();
    flash_spi_write_byte(0x60);
    ce_off();
}


void sk_flash_init(void)
{
    flash_init_spi();
}


uint16_t sk_flash_read_id(void)
{
    ce_on();
    flash_spi_write_byte(0x90);
    flash_spi_write_byte(0x00);
    flash_spi_write_byte(0x00);
    flash_spi_write_byte(0x00);
    uint16_t id = flash_spi_read_byte() << 8;
    id |= flash_spi_read_byte();
    ce_off();
    return id;
}


void sk_flesh_jedec_read_id(uint16_t *device_id, uint8_t *manufacturer)
{
    ce_on();
    flash_spi_write_byte(0x9F);
    *manufacturer = flash_spi_read_byte();
    *device_id = flash_spi_read_byte() << 8;
    *device_id |= flash_spi_read_byte();
    ce_on();
}
