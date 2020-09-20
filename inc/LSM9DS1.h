/**
 * Lib for work with LSM9DS1 iNEMO inertial module:
 * 3D accelerometer, 3D gyroscope, 3D magnetometer
 *
 * Note:
 * Only 3D magnetometer implemented
 * I2C interface available only
 */

 #include <stdint.h>
 #include <stdbool.h>

/**
 * X, Y and Z axes operative mode selection
 */
enum xyz_op_mode{
    /** Low-power mode */
    XYZ_LOW_POWER,
    /** Medium-performance mode */
    XYZ_MIDIUM_PERF,
    /** High-performance mode */
    XYZ_HIGHT_PERF,
    /** Ultra-high performance mode */
    XYZ_ULTRA_HIGH_PERF
};


typedef enum xyz_op_mode xyz_op_mode;


/**
 * Seting CTRL_REG1_M register function
 * i2c: STM32F40xx I2C interface
 * temp_comp: Temperature compensation enable. Default value: 0
 * data_rate: Output data rate selection. Default value: 100
 * mode: X and Y axes operative mode selection(:c:type:`XYZ_op_mode`)
 * fast_ord: FAST_ODR enables data rates higher than 80 Hz
 * st: Self-test enable
 */
void set_ctrl_reg1_m(uint32_t i2c, bool temp_comp, uint8_t data_rate,
    xyz_op_mode mode, bool fast_ord, bool st);


/**
 * Seting CTRL_REG2_M register function
 * i2c: STM32F40xx I2C interface
 * full_scale: Full-scale configuration. Default value: 00
 * reboot: Reboot memory content. Default value: 0
 * soft_rst: Configuration registers and user register reset function.
 * (0: default value; 1: reset operation)
 *
 * Note:
 *  Full-scale selection
 * |FS1 |FS0 |Full scale    |
 * |0   |0   |+-  4 gauss   |
 * |0   |1   |+-  8 gauss   |
 * |1   |0   |+- 12 gauss   |
 * |1   |1   |+- 16 gauss   |
 */
void set_ctrl_reg2_m(uint32_t i2c, uint8_t full_scale, bool reboot,
    bool soft_rst);


/**
 * Seting CTRL_REG3_M register function
 * i2c: STM32F40xx I2C interface
 * i2c_disable: Disable I2C interface. Default value: 0
 * lp: Low-power mode configuration. Default value: 0
 * sim: SPI Serial Interface mode selection. Default value: 0
 * (0: SPI only write operations enabled; 1: SPI read and write operations enable)
 * md: Operating mode selection.
 *
 * Note:
 * System operating mode selection
 * |MD1 |MD0 |MODE                      |
 * |0   |0   |Continuous-conversion mode|
 * |0   |1   |Single-conversion mode    |
 * |1   |0   |Power-down mode           |
 * |1   |1   |Power-down mode           |
 */
void set_ctrl_reg3_m(uint32_t i2c, bool i2c_disable, bool lp, bool sim,
    uint8_t md);


/**
 * Seting CTRL_REG4_M register function
 * i2c: STM32F40xx I2C interface
 * Z-axis operative mode selection (:c:type:`XYZ_op_mode`).
 * Default value: 00
 * ble: SPI Serial Interface mode selection. Default value: 0
 * (0: SPI only write operations enabled; 1: SPI read and write operations enable)
 * md: Operating mode selection.
 */
void set_ctrl_reg4_m(uint32_t i2c, xyz_op_mode omz, bool ble);


/**
 * Magnetometer X-axis data output.
 * i2c: STM32F40xx I2C interface
 * @return: The value of the magnetic field
 */
int16_t mag_read_x(uint32_t i2c);


/**
 * Magnetometer Y-axis data output.
 * i2c: STM32F40xx I2C interface
 * @return: The value of the magnetic field
 */
int16_t mag_read_y(uint32_t i2c);


/**
 * Magnetometer Z-axis data output.
 * i2c: STM32F40xx I2C interface
 * @return: The value of the magnetic field
 */
int16_t mag_read_z(uint32_t i2c);


void init_magn(uint32_t i2c);
