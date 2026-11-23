/*
 * lcd.h
 *
 * I2C 16x2 character LCD driver and mode-specific UI formatting helpers.
 *
 * ARCHITECTURAL RULE: Pure C only — NO HAL headers here.
 */

#ifndef INC_LCD_H_
#define INC_LCD_H_

#include <stdint.h>

#define LCD_ROWS    2U
#define LCD_COLS    16U

/**
 * @brief Initialize the I2C LCD controller (4-bit mode, backlight on).
 */
void lcd_init(void);

/**
 * @brief Clear the entire LCD display.
 */
void lcd_clear(void);

/**
 * @brief Move the LCD cursor to the specified row and column.
 * @param row  Row index (0 .. LCD_ROWS - 1).
 * @param col  Column index (0 .. LCD_COLS - 1).
 */
void lcd_set_cursor(uint8_t row, uint8_t col);

/**
 * @brief Print a null-terminated string starting at the current cursor.
 */
void lcd_print_string(const char *str);

/**
 * @brief Print and pad a string to fill an entire 16-character LCD row.
 * @param row  Row index (0 or 1).
 * @param str  Null-terminated string (truncated or space-padded to 16 chars).
 */
void lcd_print_line(uint8_t row, const char *str);

/**
 * @brief Print a signed integer at the current cursor position.
 */
void lcd_print_int(int32_t value);

/**
 * @brief Render the AUTOMATIC mode screen (lane timers + DHT20 temp/humidity).
 */
void lcd_display_auto(uint16_t t1_s, uint16_t t2_s, float temp_c, float hum_pct, uint8_t env_valid);

/**
 * @brief Render the MANUAL mode screen (current phase label + DHT20 temp/humidity).
 */
void lcd_display_manual(const char *phase_label, float temp_c, float hum_pct, uint8_t env_valid);

/**
 * @brief Render the TUNING mode screen (selected color, edited value, validity).
 */
void lcd_display_tuning(const char *color_label, uint16_t value_s, uint16_t red_s, uint16_t green_s, uint16_t amber_s);

/**
 * @brief Show a transient error or validation message on the LCD.
 */
void lcd_display_error(const char *msg);

#endif /* INC_LCD_H_ */
