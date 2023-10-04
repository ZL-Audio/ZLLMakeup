#ifndef ZLLMAKEUP_FIXEDAUDIOBUFFER_H
#define ZLLMAKEUP_FIXEDAUDIOBUFFER_H

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "dsp_defines.h"
#include "FIFOAudioBuffer.h"

template<typename FloatType>
class FixedAudioBuffer {
public:
    juce::AudioBuffer<FloatType> subBuffer;

    explicit FixedAudioBuffer(
            int subBufferSize = static_cast<int>(ZLDsp::segment::defaultV));

    void clear();

    void setSubBufferSize(int subBufferSize);

    void prepareToPlay(juce::dsp::ProcessSpec spec);

    void pushBuffer(juce::AudioBuffer<FloatType> &buffer);

    void popSubBuffer();

    void pushSubBuffer();

    void popBuffer(juce::AudioBuffer<FloatType> &buffer);

    juce::AudioBuffer<FloatType> getSubBufferChannels(int channelOffset, int numChannels);

    inline auto isSubReady() {
        return inputBuffer.getNumReady() >= subBuffer.getNumSamples();
    }

    inline auto getMainSpec() { return mainSpec; }

    inline auto getSubSpec() { return subSpec; }

    inline auto getLatencySamples() { return subSpec.maximumBlockSize; }

private:
    FIFOAudioBuffer<FloatType> inputBuffer, outputBuffer;
    juce::dsp::ProcessSpec subSpec, mainSpec;
};

#endif //ZLLMAKEUP_FIXEDAUDIOBUFFER_H
