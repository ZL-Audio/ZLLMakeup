#ifndef ZLLMAKEUP_CONTROLLER_H
#define ZLLMAKEUP_CONTROLLER_H

#include "FixedBuffer/fixed_audio_buffer.h"
#include "Tracker/rms_tracker.h"
#include "dsp_definitions.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

template<typename FloatType>
class Controller {
public:
    explicit Controller(juce::AudioProcessor *processor,
                        juce::AudioProcessorValueTreeState &parameters);

    void prepareToPlay(juce::dsp::ProcessSpec spec);

    void processBlock(juce::AudioBuffer<FloatType> &buffer);

    void setSegment(FloatType v);

    void setLookahead(FloatType v, bool useLock = true);

    void setWindow(FloatType v);

    void setGain(FloatType v);

    void setBound(FloatType v);

    void setStrength(FloatType v);

    void setSensitivity(FloatType v);

    void setModeID(int ID);

    void setMeasurementID(int ID);

    void setAccurate(bool f);

    void setCeil(bool f);

    void setSideout(bool f);

    FloatType getGain();

    bool getIsPlaying();

private:
    juce::AudioProcessor *processorRef;

    std::atomic<FloatType> gain, lookahead, bound, strength, segment, window, sensitivity;
    std::atomic<FloatType> externSensitivity;
    std::atomic<int> modeID;
    std::atomic<bool> ceil, accurate, sideout;

    juce::dsp::DelayLine<FloatType> delayLineDSP;
    juce::dsp::Gain<FloatType> gainDSP;
    std::atomic<bool> isPlaying = false;
    int64_t lastBufferSize = 0, lastBufferTime = 0;

    fixedBuffer::FixedAudioBuffer<FloatType> fixedAudioBuffer;
    RMSTracker<FloatType> mainSubTracker, auxSubTracker;
    RMSTracker<FloatType> mainTracker, auxTracker;

    void toSetLookAhead();
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
