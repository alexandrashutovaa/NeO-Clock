#include "Pomodoro.h"

#include <Arduino.h>

uint32_t Pomodoro::phaseLengthSec() const {
    return resting_ ? POMODORO_REST_SEC : POMODORO_WORK_SEC;
}

void Pomodoro::start() {
    running_ = true;
    resting_ = false;
    phaseStartedAt_ = millis();
}

void Pomodoro::stop() {
    running_ = false;
}

void Pomodoro::update() {
    if (!running_) {
        return;
    }

    const uint32_t elapsed = (millis() - phaseStartedAt_) / 1000;
    if (elapsed >= phaseLengthSec()) {
        resting_ = !resting_;
        phaseStartedAt_ = millis();
    }
}

uint32_t Pomodoro::remainingSec() const {
    if (!running_) {
        return 0;
    }

    const uint32_t elapsed = (millis() - phaseStartedAt_) / 1000;
    const uint32_t total = phaseLengthSec();
    return elapsed >= total ? 0 : total - elapsed;
}

uint8_t Pomodoro::minutes() const {
    return remainingSec() / 60;
}

uint8_t Pomodoro::seconds() const {
    return remainingSec() % 60;
}
