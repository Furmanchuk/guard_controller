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


// void sk_flash_init(void)
// {
//     flash_init_spi();
// }


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
