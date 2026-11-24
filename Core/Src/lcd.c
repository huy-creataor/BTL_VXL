/*
 * lcd.c
 *
 * I2C 16x2 character LCD driver (PCF8574) and mode-specific UI formatting helpers.
 */

#include "lcd.h"
#include "pin_config.h"
#include <stdio.h>
#include <string.h>

#define LCD_I2C_TIMEOUT_MS  10U
#define LCD_BIT_RS          0x01U
#define LCD_BIT_EN          0x04U
#define LCD_BIT_BACKLIGHT   0x08U

static void lcd_write_nibble(uint8_t nibble, uint8_t rs_flag)
{
    uint8_t data = (uint8_t)((nibble & 0xF0U) | LCD_BIT_BACKLIGHT | (rs_flag ? LCD_BIT_RS : 0x00U));
    uint8_t seq[2];

    seq[0] = (uint8_t)(data | LCD_BIT_EN);
    seq[1] = (uint8_t)(data & (uint8_t)(~LCD_BIT_EN));
    HAL_I2C_Master_Transmit(&hi2c1, LCD_I2C_ADDR, seq, 2U, LCD_I2C_TIMEOUT_MS);
}

static void lcd_send_byte(uint8_t byte, uint8_t rs_flag)
{
    lcd_write_nibble((uint8_t)(byte & 0xF0U), rs_flag);
    lcd_write_nibble((uint8_t)((byte << 4) & 0xF0U), rs_flag);
}

static void lcd_send_cmd(uint8_t cmd)
{
    lcd_send_byte(cmd, 0U);
}

static void lcd_send_data(uint8_t data)
{
    lcd_send_byte(data, 1U);
}

static void lcd_format_env_line(char *buf, size_t buf_size, float temp_c, float hum_pct, uint8_t env_valid)
{
    if (env_valid != 0U) {
        int t_int = (int)(temp_c + 0.5f);
        int h_int = (int)(hum_pct + 0.5f);
        snprintf(buf, buf_size, "T:%2dC   H:%2d%%   ", t_int, h_int);
    } else {
        snprintf(buf, buf_size, "T:--C   H:--%%   ");
    }
}

void lcd_init(void)
{
    HAL_Delay(50U);
    lcd_write_nibble(0x30U, 0U);
    HAL_Delay(5U);
    lcd_write_nibble(0x30U, 0U);
    HAL_Delay(1U);
    lcd_write_nibble(0x30U, 0U);
    HAL_Delay(1U);
    lcd_write_nibble(0x20U, 0U);
    HAL_Delay(1U);

    lcd_send_cmd(0x28U); /* 4-bit, 2 lines, 5x8 font */
    lcd_send_cmd(0x0CU); /* Display ON, cursor OFF */
    lcd_send_cmd(0x06U); /* Entry mode: increment */
    lcd_clear();
}

void lcd_clear(void)
{
    lcd_send_cmd(0x01U);
    HAL_Delay(2U);
}

void lcd_set_cursor(uint8_t row, uint8_t col)
{
    uint8_t addr = (row == 0U) ? (uint8_t)(0x80U + col) : (uint8_t)(0xC0U + col);
    if (col < LCD_COLS) {
        lcd_send_cmd(addr);
    }
}

void lcd_print_string(const char *str)
{
    if (str == NULL) {
        return;
    }
    while (*str != '\0') {
        lcd_send_data((uint8_t)(*str));
        str++;
    }
}

void lcd_print_line(uint8_t row, const char *str)
{
    char line[LCD_COLS + 1U];
    size_t len = (str != NULL) ? strlen(str) : 0U;

    for (uint8_t i = 0U; i < LCD_COLS; i++) {
        line[i] = (i < len) ? str[i] : ' ';
    }
    line[LCD_COLS] = '\0';

    lcd_set_cursor(row, 0U);
    lcd_print_string(line);
}

void lcd_print_int(int32_t value)
{
    char buf[12];
    snprintf(buf, sizeof(buf), "%ld", (long)value);
    lcd_print_string(buf);
}

void lcd_display_auto(uint16_t t1_s, uint16_t t2_s, float temp_c, float hum_pct, uint8_t env_valid)
{
    char line0[LCD_COLS + 1U];
    char line1[LCD_COLS + 1U];

    snprintf(line0, sizeof(line0), "AUTO L1:%02u L2:%02u", (unsigned)t1_s, (unsigned)t2_s);
    lcd_format_env_line(line1, sizeof(line1), temp_c, hum_pct, env_valid);

    lcd_print_line(0U, line0);
    lcd_print_line(1U, line1);
}

void lcd_display_manual(const char *phase_label, float temp_c, float hum_pct, uint8_t env_valid)
{
    char line0[LCD_COLS + 1U];
    char line1[LCD_COLS + 1U];

    snprintf(line0, sizeof(line0), "MAN: %-11s", (phase_label != NULL) ? phase_label : "---");
    lcd_format_env_line(line1, sizeof(line1), temp_c, hum_pct, env_valid);

    lcd_print_line(0U, line0);
    lcd_print_line(1U, line1);
}

void lcd_display_tuning(const char *color_label, uint16_t value_s, uint16_t red_s, uint16_t green_s, uint16_t amber_s)
{
    char line0[LCD_COLS + 1U];
    char line1[LCD_COLS + 1U];
    const char *ok_str = (red_s == (uint16_t)(green_s + amber_s)) ? "OK" : "ER";

    snprintf(line0, sizeof(line0), "TUN %-5s Val:%02u", (color_label != NULL) ? color_label : "?", (unsigned)value_s);
    snprintf(line1, sizeof(line1), "R%02u G%02u A%02u %s", (unsigned)red_s, (unsigned)green_s, (unsigned)amber_s, ok_str);

    lcd_print_line(0U, line0);
    lcd_print_line(1U, line1);
}

void lcd_display_error(const char *msg)
{
    lcd_print_line(0U, "ERROR:");
    lcd_print_line(1U, (msg != NULL) ? msg : "Invalid config");
}
