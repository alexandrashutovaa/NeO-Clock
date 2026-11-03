#include <Arduino.h>

#include "Leds.h"
#include "Pomodoro.h"

namespace {

Leds leds;
Pomodoro pomodoro;

constexpr uint32_t PRINT_INTERVAL_MS = 1000;

uint32_t lastPrintAt = 0;

}  // namespace

void setup() {
    Serial.begin(SERIAL_BAUD);
    delay(300);

    if (!leds.begin()) {
        Serial.println("Leds::begin failed");
        return;
    }

    pomodoro.start();
    Serial.println("pomodoro started");
    Serial.println("TEST 123 ABC abc");
}

void loop() {
    pomodoro.update();

    const uint32_t now = millis();
    if (now - lastPrintAt < PRINT_INTERVAL_MS) {
        return;
    }
    lastPrintAt = now;

    // Пока нет DigitMap, остаток фазы показываем полоской:
    // чем меньше осталось, тем меньше горит.
    const uint32_t total = pomodoro.resting() ? POMODORO_REST_SEC : POMODORO_WORK_SEC;
    const uint16_t lit = pomodoro.remainingSec() * Leds::count() / total;

    leds.clear();
    for (uint16_t i = 0; i < lit; ++i) {
        leds.set(i, WARM_WHITE_R, WARM_WHITE_G, WARM_WHITE_B);
    }
    leds.show();

    Serial.printf("%s %02u:%02u, leds on %u\n",
                  pomodoro.resting() ? "resting " : "working",
                  pomodoro.minutes(), pomodoro.seconds(), lit);
}
