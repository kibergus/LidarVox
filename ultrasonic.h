#pragma once

#include <ch.h>
#include <hal.h>

#include <functional>

// HC-SR04 ultrasonic range sensor support
class Ultrasonic {
public:
    // HC-SR04 needs a triger pulse to start measuremens.
    // Manufacturer suggests to use over 60ms cycle to avoid
    // interference from previous pulse echo. We will setup
    // a PWM to generate these pulses for us.
    // The sensor reports distance by length of the generated pulse.
    // Luckily there is a hardware for that too. Is is called Input Capture.
    //
    // This will consume two timers which are valuable resource, but
    // we have enough of them for the project. And this is a great cause
    // to tackle two very widely used perepherials.

    // NOTE: This class is almost entierly configuration of hardware
    // so there is no much point in making it configurable.
    explicit Ultrasonic(std::function<void(int range)> callback);

private:
    static void icuWidthCallback(ICUDriver *icup);

    // Unfortunately, we can't pass any parameters to the callback, so here is a hack with a static variable
    static std::function<void(int range)> callback_;
    const ICUConfig icuConfig_;
    const PWMConfig pwmConfig_;
};

