#pragma once

#include <stdint.h>
#include "config.h"

// Таймер 25+5. Железо не трогает, только считает время.
class Pomodoro {
public:
    void start();
    void stop();
    void update();

    bool running() const { return running_; }
    bool resting() const { return resting_; }

    uint32_t remainingSec() const;
    uint8_t minutes() const;
    uint8_t seconds() const;

private:
    uint32_t phaseLengthSec() const;

    uint32_t phaseStartedAt_ = 0;
    bool running_ = false;
    bool resting_ = false;
};
