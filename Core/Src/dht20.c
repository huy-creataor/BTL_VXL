/*
 * dht20.c
 *
 * Non-blocking I2C driver for the DHT20 temperature & humidity sensor.
 * Runs as a periodic state-machine task on the scheduler without HAL_Delay.
 */

#include "dht20.h"
#include "pin_config.h"
#include "global.h"
#include <stddef.h>

#define DHT20_I2C_TIMEOUT_MS    10U
#define DHT20_WAIT_TICKS        2U      /* 2 * 50 ms = 100 ms (>= 80 ms) */
#define DHT20_IDLE_TICKS        38U     /* ~2 s total cycle at 50 ms task period */

typedef enum {
    DHT20_STATE_CHECK_STATUS = 0,
    DHT20_STATE_TRIGGER,
    DHT20_STATE_WAIT_MEASUREMENT,
    DHT20_STATE_READ_DATA,
    DHT20_STATE_IDLE
} DHT20_State_t;

static DHT20_State_t dht20_state = DHT20_STATE_CHECK_STATUS;
static uint8_t dht20_tick_cnt = 0U;
static DHT20_Data_t dht20_data = { 0.0f, 0.0f, 0U };

static uint8_t dht20_crc8(const uint8_t *data, uint8_t len)
{
    uint8_t crc = 0xFFU;
    for (uint8_t i = 0U; i < len; i++) {
        crc ^= data[i];
        for (uint8_t bit = 0U; bit < 8U; bit++) {
            if ((crc & 0x80U) != 0U) {
                crc = (uint8_t)((crc << 1) ^ 0x31U);
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

static void dht20_reset_reg(uint8_t reg)
{
    uint8_t cmd[3] = { reg, 0x00U, 0x00U };
    HAL_I2C_Master_Transmit(&hi2c1, DHT20_I2C_ADDR, cmd, 3U, DHT20_I2C_TIMEOUT_MS);
}

void dht20_init(void)
{
    dht20_state = DHT20_STATE_CHECK_STATUS;
    dht20_tick_cnt = 0U;
    dht20_data.temperature = 0.0f;
    dht20_data.humidity = 0.0f;
    dht20_data.is_valid = 0U;
    env_data_valid = 0U;
}

void dht20_task(void)
{
    uint8_t status_cmd = 0x71U;
    uint8_t status_byte = 0U;
    uint8_t trig_cmd[3] = { 0xACU, 0x33U, 0x00U };
    uint8_t rx_buf[7] = { 0 };

    switch (dht20_state) {
        case DHT20_STATE_CHECK_STATUS:
            if (HAL_I2C_Master_Transmit(&hi2c1, DHT20_I2C_ADDR, &status_cmd, 1U, DHT20_I2C_TIMEOUT_MS) == HAL_OK &&
                HAL_I2C_Master_Receive(&hi2c1, DHT20_I2C_ADDR, &status_byte, 1U, DHT20_I2C_TIMEOUT_MS) == HAL_OK) {
                if ((status_byte & 0x18U) != 0x18U) {
                    dht20_reset_reg(0x1BU);
                    dht20_reset_reg(0x1CU);
                    dht20_reset_reg(0x1EU);
                }
                dht20_state = DHT20_STATE_TRIGGER;
            }
            break;

        case DHT20_STATE_TRIGGER:
            if (HAL_I2C_Master_Transmit(&hi2c1, DHT20_I2C_ADDR, trig_cmd, 3U, DHT20_I2C_TIMEOUT_MS) == HAL_OK) {
                dht20_tick_cnt = 0U;
                dht20_state = DHT20_STATE_WAIT_MEASUREMENT;
            } else {
                dht20_state = DHT20_STATE_CHECK_STATUS;
            }
            break;

        case DHT20_STATE_WAIT_MEASUREMENT:
            dht20_tick_cnt++;
            if (dht20_tick_cnt >= DHT20_WAIT_TICKS) {
                dht20_state = DHT20_STATE_READ_DATA;
            }
            break;

        case DHT20_STATE_READ_DATA:
            if (HAL_I2C_Master_Receive(&hi2c1, DHT20_I2C_ADDR, rx_buf, 7U, DHT20_I2C_TIMEOUT_MS) == HAL_OK) {
                if ((rx_buf[0] & 0x80U) == 0U && dht20_crc8(rx_buf, 6U) == rx_buf[6]) {
                    uint32_t raw_hum = ((uint32_t)rx_buf[1] << 12) |
                                       ((uint32_t)rx_buf[2] << 4) |
                                       ((uint32_t)(rx_buf[3] >> 4));
                    uint32_t raw_temp = (((uint32_t)(rx_buf[3] & 0x0FU)) << 16) |
                                        ((uint32_t)rx_buf[4] << 8) |
                                        ((uint32_t)rx_buf[5]);

                    dht20_data.humidity = ((float)raw_hum * 100.0f) / 1048576.0f;
                    dht20_data.temperature = (((float)raw_temp * 200.0f) / 1048576.0f) - 50.0f;
                    dht20_data.is_valid = 1U;

                    env_temperature = dht20_data.temperature;
                    env_humidity = dht20_data.humidity;
                    env_data_valid = 1U;
                } else {
                    dht20_data.is_valid = 0U;
                    env_data_valid = 0U;
                }
            } else {
                dht20_data.is_valid = 0U;
                env_data_valid = 0U;
            }
            dht20_tick_cnt = 0U;
            dht20_state = DHT20_STATE_IDLE;
            break;

        case DHT20_STATE_IDLE:
        default:
            dht20_tick_cnt++;
            if (dht20_tick_cnt >= DHT20_IDLE_TICKS) {
                dht20_state = DHT20_STATE_TRIGGER;
            }
            break;
    }
}

void dht20_get_data(DHT20_Data_t *out_data)
{
    if (out_data != NULL) {
        *out_data = dht20_data;
    }
}

float dht20_get_temperature(void)
{
    return dht20_data.temperature;
}

float dht20_get_humidity(void)
{
    return dht20_data.humidity;
}
