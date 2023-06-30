#ifndef ZLLMAKEUP_TRACKER_H
#define ZLLMAKEUP_TRACKER_H

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

template<typename FloatType>
class Tracker {
public:
    Tracker() = default;

    virtual ~Tracker() = default;

    virtual void prepareToPlay(const juce::dsp::ProcessSpec &spec) = 0;

    virtual void reset() = 0;

    virtual void setMomentarySize(size_t mSize) = 0;

    virtual inline FloatType getBufferPeak() = 0;

    virtual inline FloatType getMomentaryLoudness() = 0;

    virtual inline FloatType getIntegratedLoudness() = 0;

    virtual void process(const juce::AudioBuffer<FloatType> &buffer) = 0;
};

#endif //ZLLMAKEUP_TRACKER_H
