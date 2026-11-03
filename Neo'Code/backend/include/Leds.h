#pragma once

#include <stdint.h>
#include "config.h"

// Нижний слой: адресация физических светодиодов в цепочке.
// О цифрах и времени не знает ничего.
class Leds {
public:
    bool begin();

    void set(uint16_t index, uint8_t r, uint8_t g, uint8_t b);
    void clear();
    void fill(uint8_t r, uint8_t g, uint8_t b);
    bool show();

    void setBrightness(uint8_t value) { brightness_ = value; }
    uint8_t brightness() const { return brightness_; }

    static constexpr uint16_t count() { return LED_COUNT; }

private:
    uint8_t buffer_[LED_COUNT][3] = {};
    uint8_t brightness_ = DEFAULT_BRIGHTNESS;
    bool    ready_ = false;
};
