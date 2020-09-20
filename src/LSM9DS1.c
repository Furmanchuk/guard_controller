#include "LSM9DS1.h"
#include "macro.h"
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/i2c.h>
#include <stdint.h>
#include <stddef.h>
#include <math.h>

// LSM9DS1 magneto register
#define MAGN_ADRR       0x1E

#define WHO_AM_I_M      0x0F
#define CTRL_REG1_M     0x20
#define CTRL_REG2_M     0x21
#define CTRL_REG2_M     0x21
#define CTRL_REG3_M     0x22
#define CTRL_REG4_M     0x23

#define OUT_X_L_M       0x28
#define OUT_X_H_M       0x29
#define OUT_Y_L_M       0x2A
#define OUT_Y_H_M       0x2B
#define OUT_Z_L_M       0x2C
#define OUT_Z_H_M       0x2D


uint8_t set_op_mode(xyz_op_mode mode)
{
    uint8_t ret = 0;
    switch (mode) {
    case XYZ_LOW_POWER:
        ret = 0x00;
        break;
    case XYZ_MIDIUM_PERF:
        ret = 0x01;
        break;
    case XYZ_HIGHT_PERF:
        ret = 0x02;
        break;
    case XYZ_ULTRA_HIGH_PERF:
        ret = 0x03;
        break;
    }
    return ret;
}


//  Output data rate configuration
//  DO2|    DO1|    DO0|    ODR [Hz]
//  0  |    0  |    0  |    0.625
//  0  |    0  |    1  |    1.25
//  0  |    1  |    0  |    2.5
//  0  |    1  |    1  |    5
//  1  |    0  |    0  |    10
//  1  |    0  |    1  |    20
//  1  |    1  |    0  |    40
//  1  |    1  |    1  |    80
void set_ctrl_reg1_m(uint32_t i2c, bool temp_comp, uint8_t data_rate,
    xyz_op_mode mode, bool fast_ord, bool st)
{
    uint8_t reg = 0;
    // temperature compesation
    if (temp_comp){
        reg |= (1 << 7);
    }
    // X, Y axex operativ mode
    reg |= set_op_mode(mode) << 6;
    // Output data rate
    reg |= (data_rate << 4);
    // FAST_ODR enables data rates higher than 80 Hz
    if (fast_ord){
        reg |= (1 << 1);
    }
    // Self-test enable
    if (st){
        reg |= (1 << 0);
    }

	uint8_t cmd_set_contigious[] = {
		CTRL_REG1_M,	// register 22h | (0 << 7)
		0x00
	};
    cmd_set_contigious[1] = reg;

	i2c_transfer7(i2c, MAGN_ADRR, cmd_set_contigious,
        sk_arr_len(cmd_set_contigious), 0, 0);
}


void set_ctrl_reg2_m(uint32_t i2c, uint8_t full_scale, bool reboot,
    bool soft_rst)
{
    uint8_t reg = 0;
    // Full-scale configuration
    reg |= (full_scale << 6);
    // Reboot memory content
    if (reboot){
        reg |= (1 << 3);
    }
    // Configuration registers and user register reset function.
    if (soft_rst){
        reg |= (1 << 2);
    }

	uint8_t cmd_set_contigious[] = {
		CTRL_REG2_M,
		0x00
	};
    cmd_set_contigious[1] = reg;

	i2c_transfer7(i2c, MAGN_ADRR, cmd_set_contigious,
        sk_arr_len(cmd_set_contigious), 0, 0);
}


void set_ctrl_reg3_m(uint32_t i2c, bool i2c_disable, bool lp, bool sim,
    uint8_t md)
{
    uint8_t reg = 0;
    // Disable i2c interface
    if (i2c_disable){
        reg |= (1 << 7);
    }
    // Low-powet mode configuration
    if (lp){
        reg |= (1 << 5);
    }
    // SPI serial interface mode selection
    if (sim){
        reg |= (1 << 2);
    }
    // Operarion mode selection
    reg |= (md << 0);

	uint8_t cmd_set_contigious[] = {
		CTRL_REG3_M,
		0x00
	};
    cmd_set_contigious[1] = reg;

	i2c_transfer7(i2c, MAGN_ADRR, cmd_set_contigious,
        sk_arr_len(cmd_set_contigious), 0, 0);
}


void set_ctrl_reg4_m(uint32_t i2c, xyz_op_mode omz, bool ble)
{
    uint8_t reg = 0;
    // Operarion mode selection
    reg |= set_op_mode(omz) << 6;;
    // SPI serial interface mode selection
    if (ble){
        reg |= (1 << 1);
    }

	uint8_t cmd_set_contigious[] = {
		CTRL_REG4_M,
		0x00
	};
    cmd_set_contigious[1] = reg;

	i2c_transfer7(i2c, MAGN_ADRR, cmd_set_contigious,
        sk_arr_len(cmd_set_contigious), 0, 0);
}


int16_t mag_read_x(uint32_t i2c)
{
	int16_t x = 0;
	uint8_t var[2] = {0};
	uint8_t cmd_read_x = OUT_X_L_M | (1 << 7);		// auto increment
	i2c_transfer7(i2c, MAGN_ADRR, &cmd_read_x, 1, var, sk_arr_len(var));
	x = *(int16_t *)(var);
    return x;
}


int16_t mag_read_y(uint32_t i2c)
{
	int16_t y = 0;
	uint8_t var[2] = {0};
	uint8_t cmd_read_y = OUT_Y_L_M | (1 << 7);		// auto increment
	i2c_transfer7(i2c, MAGN_ADRR, &cmd_read_y, 1, var, sk_arr_len(var));
	y = *(int16_t *)(var);
    return y;
}


int16_t mag_read_z(uint32_t i2c)
{
	int16_t z = 0;
	uint8_t var[2] = {0};
	uint8_t cmd_read_z = OUT_Z_L_M | (1 << 7);		// auto increment
	i2c_transfer7(i2c, MAGN_ADRR, &cmd_read_z, 1, var, sk_arr_len(var));
	z = *(int16_t *)(var);
    return z;
}

// PB6 - I2C1 SCL : Alternate function AF4 : (4.7 k pullup on STM32F4DISCOVERY board)
// PB9 - I2C1 SDA : Alternate function AF4 : (4.7 k pullup on STM32F4DISCOVERY board)
//
static void i2c_init(uint32_t i2c)
{
	// Setup GPIO
	rcc_periph_clock_enable(RCC_GPIOB);
	gpio_set_output_options(GPIOB, GPIO_OTYPE_OD, GPIO_OSPEED_2MHZ, (1 << 6) | (1 << 9));
	gpio_set_af(GPIOB, GPIO_AF4, (1 << 6) | (1 << 9));
	gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, (1 << 6) | (1 << 9));

	rcc_periph_clock_enable((i2c == I2C1) ? RCC_I2C1 : RCC_I2C2);
	i2c_peripheral_disable(i2c);

	// set input clock frequency (which comes from APB1)
	i2c_set_clock_frequency(i2c, rcc_apb1_frequency / 1000000);

	i2c_set_standard_mode(i2c);		// set communication frequency to Sm (100 kHz)
	//i2c_set_dutycycle(i2c, I2C_CCR_DUTY_DIV2);		// Tlow / Thigh = 2 (relates only to Fm)

	// CCR = F_PCLK1 / (2 * F_i2c) = 42 MHz / (2 * 100 kHz) = 42e6 / (2 * 100e3) = 210
	i2c_set_ccr(i2c, (rcc_apb1_frequency / (2ul * 100000ul)));	// 100 kHz communication speed

	// Trise = 1 + Tmax / T_PCLK1 = 1 + F_PCLK1 * Tmax, where Tmax is given is I2C specs
	// for 100 kHz, Tmaz = 1000 ns = 1000e-9 s. => Trise = 1 + F_PCLK / 1e6
	i2c_set_trise(i2c, (1ul + rcc_apb1_frequency/1000000ul));

	i2c_peripheral_enable(i2c);
}


static float veclen(float x, float y, float z)
{
	return sqrtf(x*x + y*y + z*z);
}


void init_magn(uint32_t i2c)
{
    i2c_init(i2c);
}
