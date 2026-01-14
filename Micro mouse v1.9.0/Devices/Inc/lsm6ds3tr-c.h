/*
 * LSM6DS3TR.h
 *
 *  Created on: Sep 3, 2025
 *      Author: psybe
 */

#ifndef INC_LSM6DS3TR_H_
#define INC_LSM6DS3TR_H_
#include "main.h" // HAL 헤더 포함

// I2C Device Address (7-bit address 0x6A << 1 = 0xD4)
// 만약 SDO/SA0 핀이 VDD에 연결되어 있다면 0xD6을 사용하세요.
#define LSM6DS3_ADDR        0xD4

// Register Map
#define REG_WHO_AM_I        0x0F
#define REG_CTRL1_XL        0x10 // Accelerometer Control
#define REG_CTRL2_G         0x11 // Gyroscope Control
#define REG_OUTX_L_G        0x22 // Gyro Output Start
#define REG_OUTX_L_XL       0x28 // Accel Output Start

// Configuration Values
#define WHO_AM_I_ID         0x6A // Expected ID

// Sensitivity (FS setting에 따라 다름)
// Accel: +-2g, Gyro: +-2000dps 기준
#define ACCEL_SENSITIVITY   0.061f // mg/LSB (FS = +-2g)
#define GYRO_SENSITIVITY    70.0f  // mdps/LSB (FS = +-2000dps)

typedef struct {
    int16_t Accel_X_Raw;
    int16_t Accel_Y_Raw;
    int16_t Accel_Z_Raw;
    int16_t Gyro_X_Raw;
    int16_t Gyro_Y_Raw;
    int16_t Gyro_Z_Raw;

    float Accel_X; // Unit: g
    float Accel_Y;
    float Accel_Z;
    float Gyro_X;  // Unit: dps
    float Gyro_Y;
    float Gyro_Z;
} LSM6DS3_Data_t;

// Function Prototypes
uint8_t LSM6DS3_Init(I2C_HandleTypeDef *hi2c);
void LSM6DS3_ReadAccel(I2C_HandleTypeDef *hi2c, LSM6DS3_Data_t *data);
void LSM6DS3_ReadGyro(I2C_HandleTypeDef *hi2c, LSM6DS3_Data_t *data);
void LSM6DS3_ReadAll(I2C_HandleTypeDef *hi2c, LSM6DS3_Data_t *data);

#endif /* INC_LSM6DS3TR_H_ */
