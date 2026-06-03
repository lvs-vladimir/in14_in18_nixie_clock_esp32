#ifndef WS2812_DRIVER_H
#define WS2812_DRIVER_H

#include <Arduino.h>
#include <driver/rmt.h>

static inline uint8_t qadd8(uint8_t a, uint8_t b) {
  uint16_t sum = (uint16_t)a + b;
  return sum > 255 ? 255 : sum;
}

struct CHSV {
  uint8_t h, s, v;
  CHSV() : h(0), s(0), v(0) {}
  CHSV(uint8_t h_, uint8_t s_, uint8_t v_) : h(h_), s(s_), v(v_) {}
};

struct CRGB {
  uint8_t r, g, b;
  CRGB() : r(0), g(0), b(0) {}
  CRGB(uint8_t r_, uint8_t g_, uint8_t b_) : r(r_), g(g_), b(b_) {}
  CRGB(const CHSV &hsv);
  CRGB& operator=(const CRGB &o) { r = o.r; g = o.g; b = o.b; return *this; }
  CRGB& operator=(const CHSV &hsv) { *this = CRGB(hsv); return *this; }
  CRGB& operator+=(const CRGB &o) { r = qadd8(r, o.r); g = qadd8(g, o.g); b = qadd8(b, o.b); return *this; }
  CRGB& operator+=(const CHSV &hsv) { *this += CRGB(hsv); return *this; }
  void nscale8(uint8_t s) {
    r = (r * s) >> 8;
    g = (g * s) >> 8;
    b = (b * s) >> 8;
  }
  static const CRGB Black, White, Green;
};

inline CRGB::CRGB(const CHSV &hsv) {
  if (hsv.s == 0) { r = g = b = hsv.v; return; }
  uint8_t region = hsv.h / 43;
  uint8_t f = (hsv.h % 43) * 6;
  uint8_t p = (hsv.v * (255 - hsv.s)) >> 8;
  uint8_t q = (hsv.v * (255 - ((int)hsv.s * f >> 8))) >> 8;
  uint8_t t = (hsv.v * (255 - ((int)hsv.s * (255 - f) >> 8))) >> 8;
  switch (region) {
    case 0: r = hsv.v; g = t; b = p; break;
    case 1: r = q; g = hsv.v; b = p; break;
    case 2: r = p; g = hsv.v; b = t; break;
    case 3: r = p; g = q; b = hsv.v; break;
    case 4: r = t; g = p; b = hsv.v; break;
    default: r = hsv.v; g = p; b = q; break;
  }
}

const CRGB CRGB::Black(0, 0, 0);
const CRGB CRGB::White(255, 255, 255);
const CRGB CRGB::Green(0, 255, 0);

static inline uint8_t random8() { return (uint8_t)random(256); }
static inline uint8_t random8(uint8_t lim) { return (lim == 0) ? 0 : (uint8_t)random(lim); }

static inline uint8_t sin8(uint8_t x) {
  static const uint8_t t[256] = {
    128,131,134,137,140,143,146,149,152,155,158,161,164,167,170,173,
    176,179,182,185,188,191,193,196,199,201,204,207,209,212,214,217,
    219,222,224,226,229,231,233,235,237,239,241,243,245,246,248,250,
    251,253,254,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,254,253,252,251,250,249,248,247,246,244,243,241,240,
    238,236,235,233,231,229,227,225,223,220,218,216,213,211,208,206,
    203,200,198,195,192,189,186,183,180,177,174,171,168,165,162,159,
    156,153,150,147,144,141,138,135,132,129,126,123,120,117,114,111,
    108,105,102,99, 96, 93, 90, 87, 84, 81, 78, 75, 72, 69, 66, 63,
    60, 57, 54, 52, 49, 46, 43, 41, 38, 35, 33, 30, 28, 25, 23, 20,
    18, 16, 13, 11, 9,  7,  5,  3,  1,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  3,  4,  5,  6,
    8,  9,  10, 12, 13, 15, 16, 18, 19, 21, 23, 24, 26, 28, 30, 32,
    34, 36, 38, 40, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60, 62, 64,
    66, 68, 70, 72, 74, 76, 78, 80, 82, 83, 85, 87, 89, 91, 93, 95,
  };
  return t[x];
}

static inline CRGB HeatColor(uint8_t t) {
  if (t < 128) return CRGB(t * 2, 0, 0);
  if (t < 192) return CRGB(255, (t - 128) * 4, 0);
  return CRGB(255, 255, (t - 192) * 4);
}

static inline void fill_solid(CRGB *leds, int n, const CRGB &color) {
  for (int i = 0; i < n; i++) leds[i] = color;
}

static inline void fill_rainbow(CRGB *leds, int n, uint8_t sh, uint8_t dh) {
  for (int i = 0; i < n; i++) {
    leds[i] = CRGB(CHSV(sh, 255, 255));
    sh += dh;
  }
}

static inline void fadeToBlackBy(CRGB *leds, int n, uint8_t amount) {
  uint8_t s = 255 - amount;
  for (int i = 0; i < n; i++) leds[i].nscale8(s);
}

static rmt_channel_t rmt_ch = RMT_CHANNEL_0;
static bool rmt_inited = false;
static uint8_t gBrightness = 255;

static inline void ws2812_set_brightness(uint8_t b) { gBrightness = b; }

static inline void ws2812_show(CRGB *leds, int count, int pin) {
  if (!rmt_inited) {
    rmt_config_t cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.rmt_mode = RMT_MODE_TX;
    cfg.channel = rmt_ch;
    cfg.gpio_num = (gpio_num_t)pin;
    cfg.clk_div = 4;
    cfg.mem_block_num = 3;
    cfg.tx_config.loop_en = false;
    cfg.tx_config.carrier_en = false;
    cfg.tx_config.idle_output_en = true;
    cfg.tx_config.idle_level = RMT_IDLE_LEVEL_LOW;
    rmt_config(&cfg);
    rmt_driver_install(rmt_ch, 0, 0);
    rmt_inited = true;
  }
  static rmt_item32_t buf[6 * 24]; // max 6 LEDs
  int idx = 0;
  for (int i = 0; i < count; i++) {
    uint8_t r = ((uint16_t)leds[i].r * gBrightness) >> 8;
    uint8_t g = ((uint16_t)leds[i].g * gBrightness) >> 8;
    uint8_t b = ((uint16_t)leds[i].b * gBrightness) >> 8;
    uint32_t pixel = ((uint32_t)g << 16) | ((uint32_t)r << 8) | b;
    for (int bit = 23; bit >= 0; bit--) {
      if (pixel & (1UL << bit)) {
        buf[idx].level0 = 1;
        buf[idx].duration0 = 16;
        buf[idx].level1 = 0;
        buf[idx].duration1 = 10;
      } else {
        buf[idx].level0 = 1;
        buf[idx].duration0 = 8;
        buf[idx].level1 = 0;
        buf[idx].duration1 = 18;
      }
      idx++;
    }
  }
  rmt_write_items(rmt_ch, buf, idx, true);
  delayMicroseconds(300);
}

#endif
