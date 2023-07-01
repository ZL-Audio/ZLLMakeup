#ifndef ZLLMAKEUP_CONTROLLER_H
#define ZLLMAKEUP_CONTROLLER_H

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "dsp_defines.h"
#include "FixedAudioBuffer.h"
#include "Tracker//RMSTracker.h"

template<typename FloatType>
class Controller {
public:
    explicit Controller(juce::AudioProcessor *processor,
                        juce::AudioProcessorValueTreeState &parameters);

    void prepareToPlay(juce::dsp::ProcessSpec spec);

    void processBlock(juce::AudioBuffer<FloatType> &buffer);

    void setSegmentToReset(FloatType v);

    void setSegment(FloatType v);

    void setLookahead(FloatType v);

    void setWindow(FloatType v);

    void setGain(FloatType v);

    void setBound(FloatType v);

    void setStrength(FloatType v);

    void setModeID(int ID);

    void setMeasurementID(int ID);

    void setCeil(bool f);

    FloatType getGain();

    bool getIsPlaying();

private:
    juce::AudioProcessor *m_processor;
    juce::AudioProcessorValueTreeState *apvts;

    std::atomic<FloatType> gain, lookahead, bound, strength, segment, window;
    std::atomic<int> modeID;
    std::atomic<bool> ceil;

    juce::dsp::DelayLine<FloatType> delayLineDSP;
    juce::dsp::Gain<FloatType> gainDSP;
    std::atomic<bool> isPlaying = false, isSegmentReset = false;
    int64_t lastBufferSize = 0, lastBufferTime = 0;

    FixedAudioBuffer<FloatType> fixedAudioBuffer;
    RMSTracker<FloatType> mainSubTracker, auxSubTracker;
    RMSTracker<FloatType> mainTracker, auxTracker;
};

template<typename FloatType>
class ControllerAttach : public juce::AudioProcessorValueTreeState::Listener,
                         private juce::Timer {
public:
    explicit ControllerAttach(Controller<FloatType> &gainController,
                              juce::AudioProcessorValueTreeState &parameters);

    ~ControllerAttach() override;

    void timerCallback() override;

    void addListeners();

    void parameterChanged(const juce::String &parameterID, float newValue) override;

private:
    Controller<FloatType> *controller;
    juce::AudioProcessorValueTreeState *apvts;
    std::atomic<int> modeID;
};

#endif //ZLLMAKEUP_CONTROLLER_H
