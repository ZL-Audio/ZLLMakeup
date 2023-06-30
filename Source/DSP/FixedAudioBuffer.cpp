#include "FixedAudioBuffer.h"

template<typename FloatType>
FixedAudioBuffer<FloatType>::FixedAudioBuffer(int subBufferSize) :
        inputBuffer(2, 441), outputBuffer(2, 441),
        subSpec{44100, 441, 2},
        mainSpec{44100, 441, 2} {
    setSubBufferSize(subBufferSize);
}

template<typename FloatType>
void FixedAudioBuffer<FloatType>::clear() {
    inputBuffer.clear();
    outputBuffer.clear();
    subBuffer.clear();
}

template<typename FloatType>
void FixedAudioBuffer<FloatType>::setSubBufferSize(int subBufferSize) {
    clear();
    // init internal spec
    subSpec = mainSpec;
    subSpec.maximumBlockSize = static_cast<juce::uint32>(subBufferSize);
    // resize subBuffer, inputBuffer and outputBuffer
    subBuffer.setSize(static_cast<int>(subSpec.numChannels),
                      static_cast<int>(subSpec.maximumBlockSize));
    inputBuffer.setSize(static_cast<int>(mainSpec.numChannels),
                        static_cast<int>(mainSpec.maximumBlockSize) + subBufferSize);
    outputBuffer.setSize(static_cast<int>(mainSpec.numChannels),
                         static_cast<int>(mainSpec.maximumBlockSize) + subBufferSize);
    // put latency samples
    juce::AudioBuffer<FloatType> zeroBuffer(inputBuffer.getNumChannels(), subBufferSize);
    for (int channel = 0; channel < zeroBuffer.getNumChannels(); ++channel) {
        auto *channelData = zeroBuffer.getWritePointer(channel);
        for (int index = 0; index < subBufferSize; ++index) {
            channelData[index] = 0;
        }
    }
    inputBuffer.push(zeroBuffer);
}

template<typename FloatType>
void FixedAudioBuffer<FloatType>::prepareToPlay(juce::dsp::ProcessSpec spec) {
    mainSpec = spec;
}

template<typename FloatType>
void FixedAudioBuffer<FloatType>::pushBuffer(juce::AudioBuffer<FloatType> &buffer) {
    inputBuffer.push(buffer);
}

template<typename FloatType>
void FixedAudioBuffer<FloatType>::popSubBuffer() {
    inputBuffer.pop(subBuffer);
}

template<typename FloatType>
void FixedAudioBuffer<FloatType>::pushSubBuffer() {
    outputBuffer.push(subBuffer);
}

template<typename FloatType>
void FixedAudioBuffer<FloatType>::popBuffer(juce::AudioBuffer<FloatType> &buffer) {
    outputBuffer.pop(buffer);
}

template<typename FloatType>
juce::AudioBuffer<FloatType> FixedAudioBuffer<FloatType>::getSubBufferChannels(
        int channelOffset, int numChannels) {
    return juce::AudioBuffer<FloatType>(
            subBuffer.getArrayOfWritePointers() + channelOffset,
            numChannels, subBuffer.getNumSamples());
}

template
class FixedAudioBuffer<float>;

template
class FixedAudioBuffer<double>;
