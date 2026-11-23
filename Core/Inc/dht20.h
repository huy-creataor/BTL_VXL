/*
 * dht20.h
 *
 * Non-blocking I2C driver for the DHT20 temperature & humidity sensor.
 * Runs as a periodic state-machine task on the scheduler without HAL_Delay.
 *
 * ARCHITECTURAL RULE: Pure C only — NO HAL headers here.
 */

#ifndef INC_DHT20_H_
#define INC_DHT20_H_

#include <stdint.h>

typedef struct {
    float temperature;      /* Degrees Celsius */
    float humidity;         /* Relative humidity (%) */
    uint8_t is_valid;       /* 1 if last reading passed status & CRC check */
} DHT20_Data_t;

/**
 * @brief Initialize DHT20 driver state (and check sensor calibration status).
 */
void dht20_init(void);

/**
 * @brief Non-blocking DHT20 state-machine task executed by the scheduler
 *        (e.g., trigger measurement -> wait >= 80 ms via scheduler ticks -> read & parse).
 */
void dht20_task(void);

/**
 * @brief Retrieve the latest parsed DHT20 reading.
 * @param out_data  Pointer to output DHT20_Data_t struct.
 */
void dht20_get_data(DHT20_Data_t *out_data);

/**
 * @brief Get the latest temperature in degrees Celsius.
 */
float dht20_get_temperature(void);

/**
 * @brief Get the latest relative humidity in percent.
 */
float dht20_get_humidity(void);

#endif /* INC_DHT20_H_ */
