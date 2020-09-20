#include <stdint.h>
#include <stdbool.h>

#include "gc_types.h"


/**
 *Convert pin_t to uint16_t format
 */
uint16_t pin_t_to_uint16_t(pin_t *pin);


void set_number(int8_t *number, uint8_t min, uint8_t max, bool is_inc_up);


void lcd_cursor_shift(int8_t *pos, uint8_t min, uint8_t max, bool left_shift);


int8_t set_date_time(date_format_t *time, date_format_t *date,
    enum time_format format, bool is_inc_up);
