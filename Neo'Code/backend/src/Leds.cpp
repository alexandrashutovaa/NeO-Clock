// Драйвер WS2812B на периферии RMT.
// Основан на examples/peripherals/rmt/led_strip из ESP-IDF v4.4.

#include "Leds.h"

#include <esp_attr.h>
#include <esp_log.h>

static const char *TAG = "leds";

// Тайминги протокола WS2812B, наносекунды.
static const uint32_t WS2812_T0H_NS = 350;
static const uint32_t WS2812_T0L_NS = 1000;
static const uint32_t WS2812_T1H_NS = 1000;
static const uint32_t WS2812_T1L_NS = 350;
static const uint32_t WS2812_RESET_US = 280;

// Длительности в тиках счётчика RMT. Считаются в begin() от реальной
// частоты счётчика, поэтому от clk_div не зависят.
static uint32_t t0h_ticks = 0;
static uint32_t t0l_ticks = 0;
static uint32_t t1h_ticks = 0;
static uint32_t t1l_ticks = 0;

// Транслятор: RMT дёргает его порциями, превращая наши байты в посылки.
// IRAM_ATTR обязателен - вызывается из обработчика прерывания.
static void IRAM_ATTR ws2812_rmt_adapter(const void *src, rmt_item32_t *dest,
                                         size_t src_size, size_t wanted_num,
                                         size_t *translated_size, size_t *item_num) {
    if (src == nullptr || dest == nullptr) {
        *translated_size = 0;
        *item_num = 0;
        return;
    }

    const rmt_item32_t bit0 = {{{t0h_ticks, 1, t0l_ticks, 0}}};
    const rmt_item32_t bit1 = {{{t1h_ticks, 1, t1l_ticks, 0}}};

    size_t size = 0;
    size_t num = 0;
    const uint8_t *psrc = static_cast<const uint8_t *>(src);
    rmt_item32_t *pdest = dest;

    while (size < src_size && num + 8 <= wanted_num) {
        for (int i = 0; i < 8; ++i) {
            *pdest = (*psrc & (1 << (7 - i))) ? bit1 : bit0;
            ++pdest;
        }
        ++size;
        num += 8;
        ++psrc;
    }

    *translated_size = size;
    *item_num = num;
}

bool Leds::begin() {
    rmt_config_t rmt = RMT_DEFAULT_CONFIG_TX(LED_GPIO, LED_RMT_CHANNEL);
    rmt.clk_div = 2;

    esp_err_t err = rmt_config(&rmt);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "rmt_config: %s", esp_err_to_name(err));
        return false;
    }

    err = rmt_driver_install(rmt.channel, 0, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "rmt_driver_install: %s", esp_err_to_name(err));
        return false;
    }

    uint32_t counter_clk_hz = 0;
    err = rmt_get_counter_clock(rmt.channel, &counter_clk_hz);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "rmt_get_counter_clock: %s", esp_err_to_name(err));
        return false;
    }

    const float ticks_per_ns = static_cast<float>(counter_clk_hz) / 1e9f;
    t0h_ticks = static_cast<uint32_t>(ticks_per_ns * WS2812_T0H_NS);
    t0l_ticks = static_cast<uint32_t>(ticks_per_ns * WS2812_T0L_NS);
    t1h_ticks = static_cast<uint32_t>(ticks_per_ns * WS2812_T1H_NS);
    t1l_ticks = static_cast<uint32_t>(ticks_per_ns * WS2812_T1L_NS);

    err = rmt_translator_init(rmt.channel, ws2812_rmt_adapter);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "rmt_translator_init: %s", esp_err_to_name(err));
        return false;
    }

    ESP_LOGI(TAG, "rmt ready: %u Hz, ticks %u/%u/%u/%u",
             counter_clk_hz, t0h_ticks, t0l_ticks, t1h_ticks, t1l_ticks);

    ready_ = true;
    clear();
    return show();
}

void Leds::set(uint16_t index, uint8_t r, uint8_t g, uint8_t b) {
    if (index >= LED_COUNT) {
        return;
    }
    buffer_[index][0] = r;
    buffer_[index][1] = g;
    buffer_[index][2] = b;
}

void Leds::clear() {
    for (uint16_t i = 0; i < LED_COUNT; ++i) {
        buffer_[i][0] = buffer_[i][1] = buffer_[i][2] = 0;
    }
}

void Leds::fill(uint8_t r, uint8_t g, uint8_t b) {
    for (uint16_t i = 0; i < LED_COUNT; ++i) {
        set(i, r, g, b);
    }
}

bool Leds::show() {
    if (!ready_) {
        return false;
    }

    // WS2812B ждёт порядок G, R, B. Яркость применяем здесь,
    // чтобы буфер хранил исходный цвет без потерь.
    uint8_t wire[LED_COUNT * 3];
    for (uint16_t i = 0; i < LED_COUNT; ++i) {
        wire[i * 3 + 0] = static_cast<uint8_t>(buffer_[i][1] * brightness_ / 255);
        wire[i * 3 + 1] = static_cast<uint8_t>(buffer_[i][0] * brightness_ / 255);
        wire[i * 3 + 2] = static_cast<uint8_t>(buffer_[i][2] * brightness_ / 255);
    }

    esp_err_t err = rmt_write_sample(LED_RMT_CHANNEL, wire, sizeof(wire), true);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "rmt_write_sample: %s", esp_err_to_name(err));
        return false;
    }

    // Пауза сброса: по ней лента понимает, что кадр закончен.
    ets_delay_us(WS2812_RESET_US);
    return true;
}
