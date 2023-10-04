/*
==============================================================================
Copyright (C) daniel Foleys Finest Audio
==============================================================================
*/


#ifndef ZLLMAKEUP_FIFOAUDIOBUFFER_H
#define ZLLMAKEUP_FIFOAUDIOBUFFER_H

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

template<typename FloatType>
class FIFOAudioBuffer {
public:
    FIFOAudioBuffer(int channels, int bufferSize);

    void clear();

    void setSize(int channels, int bufferSize);

    void push(const FloatType **samples, int numSamples);

    void push(const juce::AudioBuffer<FloatType> &samples, int numSamples = -1);

    void pop(FloatType **samples, int numSamples);

    void pop(juce::AudioBuffer<FloatType> &samples, int numSamples = -1);

    inline auto getNumChannels() const { return buffer.getNumChannels(); }

    inline auto getNumSamples() const { return fifo.getTotalSize(); }

    inline auto getNumReady() const { return fifo.getNumReady(); }

    inline auto getFreeSpace() const { return fifo.getFreeSpace(); }

    inline auto isFull() const { return fifo.getNumReady() == fifo.getTotalSize(); }

private:
    juce::AbstractFifo fifo;

    /*< The actual audio buffer */
    juce::AudioBuffer<FloatType> buffer;
};


#endif //ZLLMAKEUP_FIFOAUDIOBUFFER_H
