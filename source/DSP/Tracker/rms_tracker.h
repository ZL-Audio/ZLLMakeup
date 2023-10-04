#ifndef ZLLMAKEUP_RMS_TRACKER_H
#define ZLLMAKEUP_RMS_TRACKER_H

#include "second_order_IIR_filter.h"
#include "tracker.h"

template<typename FloatType>
class RMSTracker : Tracker<FloatType> {
public:
    explicit RMSTracker(bool useKWeight = false);

    ~RMSTracker() override;

    void prepareToPlay(const juce::dsp::ProcessSpec &spec) override;

    void reset() override;

    void setMomentarySize(size_t mSize) override;

    void setKWeight(bool f);

    inline FloatType getBufferPeak() override {
        return juce::Decibels::gainToDecibels(peak);
    }

    inline FloatType getMomentaryLoudness() override {
        FloatType meanSquare = 0;
        if (loudness.size() > 0) {
            meanSquare = mLoudness / static_cast<FloatType>(loudness.size());
        }
        return juce::Decibels::gainToDecibels(meanSquare) * static_cast<FloatType>(0.5);
    }

    inline FloatType getIntegratedLoudness() override {
        FloatType meanSquare = 0;
        if (numBuffer > 0) {
            meanSquare = iLoudness / static_cast<FloatType>(numBuffer);
        }
        return secondPerBuffer * juce::Decibels::gainToDecibels(meanSquare) *
               static_cast<FloatType>(0.5);
    }

    inline FloatType getIntegratedTotalLoudness() override {
        return getIntegratedLoudness() * static_cast<FloatType>(numBuffer);
    }

    void process(const juce::AudioBuffer<FloatType> &buffer) override;

private:

    bool kWeight = false;
    size_t size = 0, numBuffer = 0;
    FloatType peak = 0, mLoudness = 0, iLoudness = 0;
    FloatType secondPerBuffer = 0.01f;
    std::deque<FloatType> loudness;
    SecondOrderIIRFilter<FloatType> preFilter;
    SecondOrderIIRFilter<FloatType> revisedLowFrequencyBCurveFilter;
    juce::AudioBuffer<FloatType> bufferCopy;
};


#endif //ZLLMAKEUP_RMS_TRACKER_H
