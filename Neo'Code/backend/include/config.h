#pragma once

#include <driver/rmt.h>

#define LED_GPIO            GPIO_NUM_17
#define LED_RMT_CHANNEL     RMT_CHANNEL_0

#define LAYERS_PER_DIGIT    10
#define DIGIT_COUNT         6
#define LED_COUNT           144
#define DEFAULT_BRIGHTNESS  40

// Тёплый белый, примерно 2700 K.
#define WARM_WHITE_R        255
#define WARM_WHITE_G        147
#define WARM_WHITE_B        41

// Длительности Помодоро в секундах.
// Для отладки поставь 25 и 5, чтобы цикл проходил за полминуты.
#define POMODORO_WORK_SEC   (25)
#define POMODORO_REST_SEC   (5)

#define SERIAL_BAUD         115200
