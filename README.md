# LidarVox
LidarVox is a simple synthizer controlled by waiving your hands in the air. The project has following aims:

* Demonstrate how to work with different perepherials which STM32 has:
    * EXTI for buttons
    * PWM generation for triggering HC-SR04
    * ICU for reading response of HC-SR04
    * I2C for communicating with LidarLite v3
    * DAC + GPT for waveform generation
* Make a demo that would interest not only bearded embedded enthusiasts, but even a three year old child.
* Have fun.

# How to build LidarVox hardware

Connect LidarLite v3 to PB6 (SCL) and PB9(SDA) pins. *Put capacitors on power line*. Without them interference from LidarLite would make HC-SR04 to behave very odd.
Connect HC-SR04's trigger to PD12 and echo to PC6.
Connect external *decoupled* 5v power supply to discovery 5v bus. Discovery has protection diode and 5v from USB drops to 4.5. This is not enough for HC-SR04 and it starts to work unreliably in the presense of LidarLite. If your power supply is not decopuled you can fry laptop connected to discovery!
Connect a pedal to PA0. You can use built-in button, but it more convinient to switch samples with a larger one.
Connect gnd and PA4 to an amplifier. This project does not use CS43L22, so built-in mini-jack is not utilized.

# How to build LidarVox firmware
Checkout the repository and [ChibiOS_18.2.0](https://sourceforge.net/projects/chibios/files/ChibiOS%20GPL3/Version%2018.2.0/)

Connect your discovery board and launch openocd

    # For new version of dev board
    openocd -f board/stm32f4discovery.cfg -f interface/stlink-v2-1.cfg
    # For old version of dev board
    openocd -f board/stm32f4discovery.cfg -f interface/stlink-v2.cfg

    CHIBIOS="path to ChibiOS root" make

In the launched gdb session type `load` to flash the firmware.
