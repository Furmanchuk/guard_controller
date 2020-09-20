/**
 * lib for SST25VF016B 16 Mbit (2M x *) serial flash memory
 * support for RTC STM32F40xx
 */

#include <stdint.h>


/**
 * Read the status register of the flash.
 * @return: value of the status register
 *
 * Note:
 * Аor detailed information datasheet for flash
 */
uint8_t sk_flash_get_status(void);


/**
 * Read JEDEC ID identifies the device and the manufacturer
 */
void sk_flesh_jedec_read_id(uint16_t *device_id, uint8_t *manufacturer);


/**
 * Reads the manufacturer ID and device ID
 *
 * Note:
 * ID Sequence 0xBF41
 * Manufacturer ID is the first halgbyte BFH
 * Device ID is the second halgbyte 41H
 */
uint16_t sk_flash_read_id(void);


/**
 * Write enable (WREN) the flash
 */
void sk_flash_write_enable(void);


/**
 * Write disable (WRDI) the flash
 */
void sk_flash_write_disable(void);


/**
 * Program one byte to the serial flash
 * add: destination address 000000H - 1FFFFFH
 * data: byte to be programmed
 * Note:
 * Before write data to flash need allows WREN
 */
void sk_flash_write_byte(uint32_t addr, uint8_t data);


/**
 * Program one byte to the serial flash
 * add: destination address 000000H - 1FFFFFH
 * data: byre array for read
 * size: array size
 * Note:
 * Maximum byte that can be read is 128 bytes
 */
void sk_flash_read_bytes(uint32_t addr, uint8_t *data, uint32_t size);


/**
 * Erases entire serial flash
 */
void sk_flash_arase(void);
