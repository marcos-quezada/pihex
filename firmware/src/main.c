#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include <stdint.h>
#include <stdio.h>
#include "tusb.h"
#include "bsp/board_api.h"

#define PIN_SCK  18
#define PIN_MOSI 19
#define PIN_DC   16
#define PIN_CS   17
#define PIN_BL   20
#define PIN_LED_R 6

#define SCREEN_W 320
#define SCREEN_H 240

static inline void cs_select(void)   { gpio_put(PIN_CS, 0); }
static inline void cs_deselect(void) { gpio_put(PIN_CS, 1); }

static void st7789_write_cmd(uint8_t cmd) {
  cs_select();
  gpio_put(PIN_DC, 0); // command
  spi_write_blocking(spi0, &cmd, 1);
  cs_deselect();
}

static void st7789_write_data(const uint8_t *data, size_t len) {
  cs_select();
  gpio_put(PIN_DC, 1); // data
  spi_write_blocking(spi0, data, len);
  cs_deselect();
}

static void st7789_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
  uint8_t caset[4] = { x0 >> 8, x0 & 0xFF, x1 >> 8, x1 & 0xFF };
  uint8_t raset[4] = { y0 >> 8, y0 & 0xFF, y1 >> 8, y1 & 0xFF };

  st7789_write_cmd(0x2A); // CASET
  st7789_write_data(caset, 4);
  st7789_write_cmd(0x2B); // RASET
  st7789_write_data(raset, 4);
  st7789_write_cmd(0x2C); // RAMWR
}

static void st7789_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color565) {
  st7789_set_window(x, y, x + w - 1, y + h - 1);

  uint8_t px[2] = { color565 >> 8, color565 & 0xFF };
  cs_select();
  gpio_put(PIN_DC, 1);
  for (uint32_t i = 0; i < (uint32_t)w * h; i++) {
    spi_write_blocking(spi0, px, 2);
  }
  cs_deselect();
}

static inline uint16_t rgb888_to_565(uint32_t rgb) {
  uint8_t r = (rgb >> 16) & 0xFF;
  uint8_t g = (rgb >> 8) & 0xFF;
  uint8_t b = rgb & 0xFF;
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

#define CMD_BUF_SIZE 64
static char cmd_buf[CMD_BUF_SIZE];
static size_t cmd_len = 0;

static void process_command(const char *line) {
  unsigned x, y, w, h, color;
  if (sscanf(line, "FILL %u %u %u %u %06x", &x, &y, &w, &h, &color) == 5) {
    st7789_fill_rect((uint8_t)x, (uint16_t)y, (uint16_t)w, (uint16_t)h,
                      rgb888_to_565(color));
  }
}

static void cdc_task(void) {
  if (!tud_cdc_available()) return;

  gpio_put(PIN_LED_R, 1); // debug: latch on as soon as any data arrives

  char buf[64];
  uint32_t count = tud_cdc_read(buf, sizeof(buf));

  for (uint32_t i = 0; i < count; i++) {
    char c = buf[i];
    if (c == '\n' || c == '\r') {
      if (cmd_len > 0) {
        cmd_buf[cmd_len] = '\0';
        process_command(cmd_buf);
        cmd_len = 0;
      }
    } else if (cmd_len < CMD_BUF_SIZE -1) {
      cmd_buf[cmd_len++] = c;
    }
  }
}

static void st7789_init(void) {
  gpio_init(PIN_CS);
  gpio_set_dir(PIN_CS, GPIO_OUT);
  gpio_put(PIN_CS, 1);

  gpio_init(PIN_DC);
  gpio_set_dir(PIN_DC, GPIO_OUT);

  spi_init(spi0, 62500000);
  gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
  gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

  st7789_write_cmd(0x01); // SWRESET
  sleep_ms(150);
  st7789_write_cmd(0x11); // SLPOUT
  sleep_ms(150);

  uint8_t colmod = 0x05; // RGB565
  st7789_write_cmd(0x3A);
  st7789_write_data(&colmod, 1);

  uint8_t madctl = 0x70; // COL_ORDER | SWAP_XY | SCAN_ORDER
  st7789_write_cmd(0x36);
  st7789_write_data(&madctl, 1);

  st7789_write_cmd(0x21); // INVON -- common requirement for this panel family
  st7789_write_cmd(0x29); // DISPON
  sleep_ms(50);
}

static void backlight_init(void) {
  gpio_set_function(PIN_BL, GPIO_FUNC_PWM);
  uint slice = pwm_gpio_to_slice_num(PIN_BL);
  pwm_set_wrap(slice, 255);
  pwm_set_gpio_level(PIN_BL, 180); // fixed brightness for now
  pwm_set_enabled(slice, true);
}

int main(void) {
  board_init();
  tusb_init();
  stdio_init_all();

  st7789_init();
  backlight_init();

  gpio_init(PIN_LED_R);
  gpio_set_dir(PIN_LED_R, GPIO_OUT);
  gpio_put(PIN_LED_R, 0);

  st7789_fill_rect(0, 0, SCREEN_W, SCREEN_H, 0x0000);         // clear to black

  while (true){
    tud_task();
    cdc_task;
  }
}
