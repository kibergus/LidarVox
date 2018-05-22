#include "ultrasonic.h"

Ultrasonic::Ultrasonic(std::function<void(int range)> callback)
    : icuConfig_ {
        ICU_INPUT_ACTIVE_HIGH,
        1000000,       // 1MHz ICU clock frequency
        icuWidthCallback,
        NULL,
        NULL,
        ICU_CHANNEL_1,
        0
    }
    , pwmConfig_ {
        100000,        // 100kHz PWM clock frequency
        6000,          // PWM period
        NULL,
        {
            {PWM_OUTPUT_ACTIVE_HIGH, NULL},
            {PWM_OUTPUT_DISABLED, NULL},
            {PWM_OUTPUT_DISABLED, NULL},
            {PWM_OUTPUT_DISABLED, NULL}
        },
        // HW dependent part
        0,
        0
    }
{
    callback_ = callback;

    // Enable ICU
    icuStart(&ICUD3, &icuConfig_);
    palSetPadMode(GPIOC, 6, PAL_MODE_ALTERNATE(2));
    icuStartCapture(&ICUD3);
    icuEnableNotifications(&ICUD3);

    // Enable PWM
    pwmStart(&PWMD4, &pwmConfig_);
    palSetPadMode(GPIOD, 12, PAL_MODE_ALTERNATE(2));
    pwmEnableChannel(&PWMD4, 0, 10);
}

// This global variable helps debug a lot
volatile int ultrasonicRange = 0;

void Ultrasonic::icuWidthCallback(ICUDriver *icup) {
    icucnt_t width = icu_lld_get_width(icup);

    // The sensor is unstable when powered at 4.5v.
    // This exponential smoothing fights it a bit.
    // This code is here to show how easily a smoothing can be done.
    // It would be better if smoothedWidth would a class property.
    static const SMOOTH_FACTOR = 4;
    static icucnt_t smoothedWidth;
    smoothedWidth = (smoothedWidth * (SMOOTH_FACTOR - 1) + width) / SMOOTH_FACTOR;
    
    static const icucnt_t soundSpeed = 340;
    
    ultrasonicRange = width * soundSpeed * 100 /* centimeters */ / 1000000 /* microseconds */ / 2;
    if (callback_)
        callback_(ultrasonicRange);
}

std::function<void(int range)> Ultrasonic::callback_;
