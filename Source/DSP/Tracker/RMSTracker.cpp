#include "RMSTracker.h"

template<typename FloatType>
RMSTracker<FloatType>::RMSTracker(bool useKWeight) :
        preFilter(static_cast<FloatType>(1.53512485958697), // b0
                  static_cast<FloatType>(-2.69169618940638), // b1
                  static_cast<FloatType>(1.19839281085285), // b2
                  static_cast<FloatType>(-1.69065929318241), // a1
                  static_cast<FloatType>(0.73248077421585)), // a2
        revisedLowFrequencyBCurveFilter(static_cast<FloatType>(1.0), // b0
                                        static_cast<FloatType>(-2.0), // b1
                                        static_cast<FloatType>(1.0), // b2
                                        static_cast<FloatType>(-1.99004745483398), // a1
                                        static_cast<FloatType>(0.99007225036621)) // a2
{
    kWeight = useKWeight;
}

template<typename FloatType>
RMSTracker<FloatType>::~RMSTracker() {
    loudness.clear();
}

template<typename FloatType>
void RMSTracker<FloatType>::prepareToPlay(const juce::dsp::ProcessSpec &spec) {
    reset();
    bufferCopy.setSize(static_cast<int>(spec.numChannels),
                       static_cast<int>(spec.maximumBlockSize));
    preFilter.prepareToPlay(static_cast<float>(spec.sampleRate),
                            static_cast<int>(spec.numChannels));
    revisedLowFrequencyBCurveFilter.prepareToPlay(static_cast<float>(spec.sampleRate),
                                                  static_cast<int>(spec.numChannels));
}

template<typename FloatType>
void RMSTracker<FloatType>::reset() {
    loudness.clear();
    mLoudness = 0;
    iLoudness = 0;
    numBuffer = 0;
}

template<typename FloatType>
void RMSTracker<FloatType>::setMomentarySize(size_t mSize) {
    size = mSize;
    while (loudness.size() > size) {
        mLoudness -= loudness.front();
        loudness.pop_front();
    }
}

template<typename FloatType>
void RMSTracker<FloatType>::setKWeight(bool f) {
    kWeight = f;
}

template<typename FloatType>
void RMSTracker<FloatType>::process(const juce::AudioBuffer<FloatType> &buffer) {
    for (int channel = 0; channel < bufferCopy.getNumChannels(); ++channel) {
        bufferCopy.copyFrom(channel, 0, buffer, channel, 0, buffer.getNumSamples());
    }

    if (kWeight) {
        preFilter.processBlock(bufferCopy);
        revisedLowFrequencyBCurveFilter.processBlock(bufferCopy);
    }

    FloatType _ms = 0;
    for (auto channel = 0; channel < bufferCopy.getNumChannels(); channel++) {
        auto data = bufferCopy.getReadPointer(channel);
        for (auto i = 0; i < bufferCopy.getNumSamples(); i++) {
            _ms += data[i] * data[i];
        }
    }

    _ms = _ms / static_cast<FloatType> (bufferCopy.getNumSamples());
    loudness.push_back(_ms);
    mLoudness += _ms;
    while (loudness.size() > size) {
        mLoudness -= loudness.front();
        loudness.pop_front();
    }

    iLoudness += _ms;
    numBuffer += 1;
}

template
class RMSTracker<float>;

template
class RMSTracker<double>;