#include "sampler.h"

Sampler::Sampler(DACDriver* driver, GPTDriver* gptDriver)
    : dacConfig_{
        .init = 2047U,
        .datamode = DAC_DHRM_12BIT_RIGHT,
        .cr = 0
    }
    , dacGroupConfig_{
        .num_channels = 1U,
        .end_cb = onEnd,
        .error_cb = onError,
        .trigger = DAC_TRG(0)
    }
    , gptConfig_{
        // 10MHz tick. We will adjust pace by setting number of ticks before overflow
        .frequency    = 10000000U,
        // No need for callback. This thing will work in hardware
        .callback     = NULL,
        // MMS = 010 = TRGO on Update Event
        .cr2          = TIM_CR2_MMS_1,
        .dier         = 0U
    }
    , driver_(driver)
    , gptDriver_(gptDriver)
    , isActive_(false)
    , isModulated_(false)
    , modulationStep_(0)
{
    self_ = this;

    dacStart(driver_, &dacConfig_);

    // NOTE: DAC outputs analog level corresponding to register settings.
    // But it does not advance through buffer itsafe. The driver internally
    // uses DMA to feed the buffer to the register. But we also need a pacemaker
    // which will tell the DMA when to adwance to the next value. Here we use a GPT
    // (general purpose timer) which on owerflow event triggers the DMA
    gptStart(gptDriver_, &gptConfig_);
    gptStartContinuous(gptDriver_, 20U);
}

void Sampler::stop()
{
    dacStopConversion(driver_);
    isActive_ = false;
}

void Sampler::play(gptcnt_t pace)
{
    if (!buffer_)
        return;

    if (!isActive_) {
        dacStartConversion(driver_, &dacGroupConfig_, buffer_, BUFFER_SIZE);
        isActive_ = true;
    }

    sample_.setPace(pace);
    gptChangeInterval(gptDriver_, pace);
}

void Sampler::stopModulation()
{
    isModulated_ = false;
}

void Sampler::modulate(uint32_t pace)
{
    isModulated_ = true;
    modulation_.setPace(pace);
}

void Sampler::setSampleI(const dacsample_t* sample, size_t size)
{
    sample_ = Sample(sample, size);

    if (isActive_) {
        dacStopConversionI(driver_);
        dacStartConversionI(driver_, &dacGroupConfig_, buffer_, BUFFER_SIZE);
    }
}

void Sampler::setModulationI(const dacsample_t* sample, size_t size) {
    modulation_ = Sample(sample, size);
}

void Sampler::onEnd(DACDriver* /*dacp*/, dacsample_t *buffer, size_t n) {
    self_->onEnd(buffer, n);
}

void Sampler::onEnd(dacsample_t *buffer, size_t n) {
    // We want to apply modulation to our samples. So we have to process
    // them before sending to DAC. Driver calls this calback and tells us
    // when we need to prepare buffers for it.
    // Buffers allow our code to work in less strict realtime mode.
    // We can prepare them anytime while previous buffer is beeng processed.

    static const dacsample_t MUL_MAX = 4096;

    for (size_t i = 0; i < n; ++i) {
        dacsample_t scale = isModulated_ ? modulation_.get() : MUL_MAX;
        buffer[i] = sample_.get() * scale / MUL_MAX;
        sample_.next();

        modulation_.next();

        if (++modulationStep_ % (modulation_.pace() / sample_.pace()) == 0) {
            modulation_.next();
            modulationStep_ = 0;
        }
    }
}

void Sampler::onError(DACDriver*, dacerror_t) {
    chSysHalt("DAC failure");
}

Sampler* Sampler::self_;
