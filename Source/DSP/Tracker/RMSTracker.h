#ifndef ZLLMAKEUP_RMSTRACKER_H
#define ZLLMAKEUP_RMSTRACKER_H

#include "Tracker.h"
#include "SecondOrderIIRFilter.h"

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
        return juce::Decibels::gainToDecibels(meanSquare) * static_cast<FloatType>(0.5);
    }

    void process(const juce::AudioBuffer<FloatType> &buffer) override;

private:

    bool kWeight = false;
    size_t size = 0, numBuffer = 0;
    FloatType peak = 0, mLoudness = 0, iLoudness = 0;
    std::deque<FloatType> loudness;
    SecondOrderIIRFilter<FloatType> preFilter;
    SecondOrderIIRFilter<FloatType> revisedLowFrequencyBCurveFilter;
    juce::AudioBuffer<FloatType> bufferCopy;
};


#endif //ZLLMAKEUP_RMSTRACKER_H
