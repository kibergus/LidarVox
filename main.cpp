//      === LidarVox v0.0 by kibergus ===
//
// This is a application that demonstrates how to work with various STM32 perepherials
// It's was developed mainly for educational use but you can try to play music with it.
//
// This code is licensed under WTFPL

// Some useful links:
// CPU datasheet
//      http://www.st.com/content/ccc/resource/technical/document/datasheet/ef/92/76/6d/bb/c2/4f/f7/DM00037051.pdf/files/DM00037051.pdf/jcr:content/translations/en.DM00037051.pdf
// STM32F407 discovery board datasheet:
//      http://www.st.com/content/ccc/resource/technical/document/user_manual/70/fe/4a/3f/e7/e1/4f/7d/DM00039084.pdf/files/DM00039084.pdf/jcr:content/translations/en.DM00039084.pdf
// LidarLite v3 datasheet:
//      https://static.garmin.com/pumac/LIDAR_Lite_v3_Operation_Manual_and_Technical_Specifications.pdf

#include "samples/samples.h"
#include "lidar.h"
#include "ultrasonic.h"
#include "sampler.h"

#include <ch.h>
#include <hal.h>

// Sound will be played if there lidar reading are closer than this
const int MAX_RANGE = 200;
const int MAX_MODULATION_RANGE = 100;

void initI2C() {
    static const I2CConfig config {
        OPMODE_I2C,
        // NOTE: CS43L22 which sits on the same bus supports only 100kHz
        // but we don't use it and it is not enabled.
        400000,                         //I2C @ 400kHz
        STD_DUTY_CYCLE,
    };
    i2cStart(&I2CD1, &config);

    // NOTE: Discovery board has external DAC CS43L22 connected to I2C bus.
    // Pins B6 and B9 are already configured for I2C in board.h file.
    // The cleaner and better way to setup pins is to edit board.h file.
    // This way they will be configured correctly and safely right from the
    // chip startup. but if you want to configure them on the run, this is what you do:

    // palSetPadMode(GPIOB, 6, PAL_MODE_ALTERNATE(4) | PAL_STM32_OTYPE_OPENDRAIN);
    // palSetPadMode(GPIOB, 9, PAL_MODE_ALTERNATE(4) | PAL_STM32_OTYPE_OPENDRAIN);

    // I2C devies have addresses and can share same bus, so CS43L22 would not break
    // communication with the lidar.
}

void initDac() {
    // DAC output is redirected to a PA4
    palSetPadMode(GPIOA, 4, PAL_MODE_INPUT_ANALOG);

    // NOTE: Discovery has external DAC and amplifier connected to I2S bus
    // It can be configured to take signal from internall DAC too.
    // This demo does not do that. Signal will be output to a PA4, but not
    // to a jack header
}

// This function is called from interrput handler
void onPedalPressed(void* sampler) {
    port_lock_from_isr();

    static int sample = 0;

    sample = (sample + 1) % samplesSize;
    reinterpret_cast<Sampler*>(sampler)->setSampleI(samples[sample].data, samples[sample].size);

    port_unlock_from_isr();
}

// Global variables are evil. But compiler optimizes out local one too good,
// making debug a little bit hard. So here is a kludge that allows you to see
// current measurements in gdb at any time.
volatile int range;

int main(void) {
    halInit();
    chSysInit();

    initI2C();
    Lidar lidar(&I2CD1);

    initDac();
    Sampler sampler(&DACD1, &GPTD6);

    // We could detect button press by periodic calls to palReadPad
    // but there is a better way: STM32 can trigger interrupts when
    // detects an edge. This method has less latency, does not consume
    // CPU cycles and can even work when CPU core sleeping.
    auto line = PAL_LINE(GPIOA, 0);
    palEnableLineEvent(line, PAL_EVENT_MODE_RISING_EDGE);
    palSetLineCallback(line, onPedalPressed, &sampler);

    Ultrasonic ultrasonic(
        [&sampler](int range) {
            if (range > MAX_MODULATION_RANGE || range == 0) {
                sampler.stopModulation();
            } else {
                sampler.modulate(200000 / (1 + range));
            }
        });

    chSysLock();
    sampler.setSampleI(samples[0].data, samples[0].size);
    sampler.setModulationI(samples[0].data, samples[0].size);
    chSysUnlock();

    for(;;) {
        range = lidar.getRange();
        if (range > MAX_RANGE) {
            sampler.stop();
        } else {
            sampler.play(20000 / (1 + range));
        }
    }
}
