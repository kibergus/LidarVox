#pragma once

#include <ch.h>
#include <hal.h>

struct Sample {
    const dacsample_t* data;
    size_t size;
};

extern const Sample samples[];
extern const size_t samplesSize;
