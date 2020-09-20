#include "gc_utils.h"



uint16_t pin_t_to_uint16_t(pin_t *pin)
{
	uint16_t ret = 0x00;
	ret |= (pin->num[0] << 12) | (pin->num[1] << 8) | (pin->num[2] << 4) |
        pin->num[3];
	return ret;
}


// helper fuction
void set_number(int8_t *number, uint8_t min, uint8_t max, bool is_inc_up)
{
	if (is_inc_up){
		*number = (++(*number) <= max) ? *number : min;
	}else{
		*number = (--(*number) >= min) ? *number : max;
	}
}


// helper fuction
void lcd_cursor_shift(int8_t *pos, uint8_t min, uint8_t max, bool left_shift)
{
	if (left_shift){
		*pos = (--(*pos) >= min) ? *pos : min;
	}else{
		*pos = (++(*pos) <= max) ? *pos : max;
	}
}


int8_t set_date_time(date_format_t *time, date_format_t *date,
    enum time_format format, bool is_inc_up)
{

    uint8_t num;
	switch (format) {
		case TF_HOUR_T:
            set_number(&(time->num[0]), 0, 2, is_inc_up);
            num = time->num[0];
            break;
		case TF_HOUR_U:
            set_number(&(time->num[1]), 0, 9, is_inc_up);
            num = time->num[1];
            break;
		case TF_MINUTE_T:
            set_number(&(time->num[2]), 0, 5, is_inc_up);
            num = time->num[2];
			break;
		case TF_MINUTE_U:
            set_number(&(time->num[3]), 0, 9, is_inc_up);
            num = time->num[3];
			break;
		case TF_SECOND_T:
            set_number(&(time->num[4]), 0, 5, is_inc_up);
            num = time->num[4];
			break;
		case TF_SECOND_U:
            set_number(&(time->num[5]), 0, 9, is_inc_up);
            num = time->num[5];
			break;
		case TF_YEAR_T:
            set_number(&(date->num[0]), 0, 9, is_inc_up);
            num = date->num[0];
			break;
		case TF_YEAR_U:
            set_number(&(date->num[1]), 0, 9, is_inc_up);
            num = date->num[1];
			break;
		case TF_MONTH_T:
            set_number(&(date->num[2]), 0, 1, is_inc_up);
            num = date->num[2];
			break;
		case TF_MONTH_U:
            set_number(&(date->num[3]), 0, 9, is_inc_up);
            num = date->num[3];
			break;
		case TF_DAY_T:
            set_number(&(date->num[4]), 0, 3, is_inc_up);
            num = date->num[4];
			break;
		case TF_DAY_U:
            set_number(&(date->num[5]), 0, 9, is_inc_up);
            num = date->num[5];
			break;
        default: return -1;
	}
    return num;
}
