#pragma once

#include <ch.h>
#include <hal.h>

class Sampler {
public:

    Sampler(DACDriver* driver, GPTDriver* gptDriver);

    void stop();

    void play(gptcnt_t pace);

    void stopModulation();

    void modulate(uint32_t pace);

    // I suffix means that this function can be called only from a locked state
    // This is needed for calling it from interrupts. It is a design choice
    // from chibios. See docks for details:
    // http://chibios.sourceforge.net/html/concepts.html#system_states
    void setSampleI(const dacsample_t* sample, size_t size);

    void setModulationI(const dacsample_t* sample, size_t size);

private:
    class Sample {
    public:
        explicit Sample(const dacsample_t* data = nullptr, size_t size = 0)
            : data_(data)
            , size_(size)
            , cursor_(0)
        {}

        dacsample_t get()
        {
            return data_[cursor_];
        }

        void next() {
            cursor_ = (cursor_ + 1) % size_;
        }

        gptcnt_t pace() {
            return pace_;
        }

        void setPace(gptcnt_t pace) {
            pace_ = pace;
        }

    private:
        const dacsample_t* data_;
        size_t size_;
        size_t cursor_;
        uint32_t pace_;
    };

    // Unfortunately there is no way to pass a pointer in-to the callback
    // so here is an ungly hack with global variable 
    static Sampler* self_;

    static void onEnd(DACDriver* /*dacp*/, dacsample_t *buffer, size_t n);

    static void onError(DACDriver*, dacerror_t);

    void onEnd(dacsample_t *buffer, size_t n);

    const DACConfig dacConfig_;
    const DACConversionGroup dacGroupConfig_;
    const GPTConfig gptConfig_;

    DACDriver* driver_;
    GPTDriver* gptDriver_;

    bool isActive_;
    bool isModulated_;

    Sample sample_;
    Sample modulation_;

    gptcnt_t modulationStep_;

    static const size_t BUFFER_SIZE = 256;
    dacsample_t buffer_[BUFFER_SIZE];
};
