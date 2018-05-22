#include "samples.h"

// This is a set of available samples. Just uncompressed const dacsample_t[].
// STM32 has unified address space which covers both RAM and flash. So const arrays
// can be used directly from flash memory without copying to RAM.
// STM32F407VG has 1Mb of flash which gives us quite a lot of space for samples. 
#include "sine.inl"
#include "meander.inl"
#include "saw.inl"
#include "voice.inl"

#define SAMPLE(buf) Sample{buf, sizeof(buf) / sizeof(dacsample_t)}

const Sample samples[] = {
    SAMPLE(dac_buffer_sine),
    SAMPLE(dac_buffer_meander),
    SAMPLE(dac_buffer_saw),
    SAMPLE(dac_buffer_voice),
};

const size_t samplesSize = sizeof(samples) / sizeof(Sample);
