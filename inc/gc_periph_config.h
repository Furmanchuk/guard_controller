
/**
 * Initialize clock
 */
void clock_init(void);


/**
 * Initialize RCC peripheral clock
 */
void periph_clock_init(void);


/**
 * Initialize for SPI peripheral
 *
 * Note:
 * Used SPI1 STM32F40xx
 * SPI pinouts
 * SCK - PA5
 * SO(MOSI) - PB5
 * SI(MISO) - PB4
 * CS(SS) - PD7 (controled by software )
 */
void sk_flash_init(void);
