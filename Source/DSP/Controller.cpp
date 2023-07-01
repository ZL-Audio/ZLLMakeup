#include "Controller.h"

template<typename FloatType>
Controller<FloatType>::Controller(juce::AudioProcessor *processor,
                                  juce::AudioProcessorValueTreeState &parameters) :
        fixedAudioBuffer() {
    m_processor = processor;
    apvts = &parameters;

    gain.store(ZLDsp::gain::defaultV);
    lookahead.store(ZLDsp::lookahead::defaultV);
    bound.store(ZLDsp::bound::defaultV);
    strength.store(ZLDsp::strength::defaultV);
    setSegmentToReset(ZLDsp::segment::defaultV);
    setWindow(ZLDsp::window::defaultV);

    ceil.store(ZLDsp::ceil::defaultV);

    modeID.store(ZLDsp::mode::defaultI);
    setMeasurementID(ZLDsp::measurement::defaultI);
}

template<typename FloatType>
void Controller<FloatType>::prepareToPlay(juce::dsp::ProcessSpec spec) {
    spec.numChannels = spec.numChannels * 2;
    fixedAudioBuffer.prepareToPlay(spec);
}

template<typename FloatType>
void Controller<FloatType>::processBlock(juce::AudioBuffer<FloatType> &buffer) {
    if (isSegmentReset.load()) {
        isSegmentReset.store(false);
        setSegment(segment.load());
    }
    auto currentPos = m_processor->getPlayHead()->getPosition();
    if (std::abs(lastBufferSize + lastBufferTime -
                 currentPos->getTimeInSamples().orFallback(0)) > buffer.getNumSamples()) {
        isPlaying.store(false);
    } else {
        isPlaying.store(true);
    }
    lastBufferSize = buffer.getNumSamples();
    lastBufferTime = currentPos->getTimeInSamples().orFallback(0);
    if (modeID.load() == ZLDsp::mode::effect && isPlaying.load()) {
        fixedAudioBuffer.pushBuffer(buffer);
        while (fixedAudioBuffer.isSubReady()) {
            fixedAudioBuffer.popSubBuffer();

            // calculate loudness makeup
            auto mainBusNumChannel = static_cast<int>(
                    fixedAudioBuffer.getSubSpec().numChannels / 2);
            mainPreTracker.process(
                    fixedAudioBuffer.getSubBufferChannels(0, mainBusNumChannel));
            auxTracker.process(fixedAudioBuffer.getSubBufferChannels(mainBusNumChannel,
                                                                     2 *
                                                                     mainBusNumChannel));
            auto actualGain = auxTracker.getMomentaryLoudness() -
                              mainPreTracker.getMomentaryLoudness();
            actualGain = juce::jlimit(-bound.load(), bound.load(), actualGain);

            // apply loudness makeup
            juce::dsp::AudioBlock<FloatType> block(fixedAudioBuffer.subBuffer);
            auto mainBlock = block.getSubsetChannelBlock(0,
                                                         static_cast<size_t>(mainBusNumChannel));
            delayLineDSP.process(
                    juce::dsp::ProcessContextReplacing<FloatType>(mainBlock));

            if (ceil.load()) {
                actualGain = juce::jmin(actualGain,
                                        -juce::Decibels::gainToDecibels(
                                                fixedAudioBuffer.getSubBufferChannels(
                                                        0,
                                                        mainBusNumChannel).getMagnitude(
                                                        0,
                                                        static_cast<int>(fixedAudioBuffer.getSubSpec().maximumBlockSize))));
            }

            gain.store(actualGain);
            gainDSP.setGainDecibels(
                    actualGain * ZLDsp::strength::formatV(strength.load()));

            gainDSP.process(juce::dsp::ProcessContextReplacing<FloatType>(mainBlock));

            fixedAudioBuffer.pushSubBuffer();
        }
        fixedAudioBuffer.popBuffer(buffer);
    }
    if (modeID.load() == ZLDsp::mode::envelope) {
        auto currentGain = juce::Decibels::decibelsToGain(
                gain.load() * ZLDsp::strength::formatV(strength.load()));
        auto mainBuffer = m_processor->getBusBuffer(buffer, true, 0);
        mainBuffer.applyGain(currentGain);
    }
}

template<typename FloatType>
void Controller<FloatType>::setSegmentToReset(FloatType v) {
    segment.store(v);
    isSegmentReset.store(true);
}

template<typename FloatType>
void Controller<FloatType>::setSegment(FloatType v) {
    auto subBufferSize = static_cast<int>(std::round(
            v * fixedAudioBuffer.getMainSpec().sampleRate / 1000));
    fixedAudioBuffer.setSubBufferSize(subBufferSize);

    auto subSpec = fixedAudioBuffer.getSubSpec();
    auto subBusSpec = subSpec;
    subBusSpec.numChannels = subBusSpec.numChannels / 2;
    for (auto &f: {&mainPreTracker, &mainAfterTracker, &auxTracker}) {
        (*f).prepareToPlay(subBusSpec);
    }
    delayLineDSP.prepare(subBusSpec);
    gainDSP.prepare(subBusSpec);
    gainDSP.setRampDurationSeconds(static_cast<double>(v) / 4000);
    delayLineDSP.setMaximumDelayInSamples(
            subBufferSize * static_cast<int>(ZLDsp::lookahead::range.end));
    setLookahead(lookahead.load());
}

template<typename FloatType>
void Controller<FloatType>::setLookahead(FloatType v) {
    lookahead.store(v);
    auto latencyInSamples =
            static_cast<int>(segment.load() / 1000 *
                             ZLDsp::lookahead::formatV(lookahead.load()) * window.load() *
                             static_cast<FloatType>(fixedAudioBuffer.getSubSpec().maximumBlockSize));
    delayLineDSP.setDelay(static_cast<FloatType>(latencyInSamples));
    m_processor->setLatencySamples(
            static_cast<int>(fixedAudioBuffer.getLatencySamples()) +
            latencyInSamples - 1);
}

template<typename FloatType>
void Controller<FloatType>::setWindow(FloatType v) {
    window.store(v);
    for (auto &f: {&mainPreTracker, &mainAfterTracker, &auxTracker}) {
        (*f).setMomentarySize(static_cast<size_t>(v));
    }
    setLookahead(lookahead.load());
}

template<typename FloatType>
void Controller<FloatType>::setGain(FloatType v) {
    gain.store(v);
}

template<typename FloatType>
void Controller<FloatType>::setBound(FloatType v) {
    bound.store(v);
}

template<typename FloatType>
void Controller<FloatType>::setStrength(FloatType v) {
    strength.store(v);
}

template<typename FloatType>
void Controller<FloatType>::setModeID(int ID) {
    modeID.store(ID);
}

template<typename FloatType>
void Controller<FloatType>::setMeasurementID(int ID) {
    if (ID == ZLDsp::measurement::rms) {
        mainPreTracker.setKWeight(false);
        mainAfterTracker.setKWeight(false);
        auxTracker.setKWeight(false);
    } else if (ID == ZLDsp::measurement::k_rms) {
        mainPreTracker.setKWeight(true);
        mainAfterTracker.setKWeight(true);
        auxTracker.setKWeight(true);
    }
}

template<typename FloatType>
void Controller<FloatType>::setCeil(bool f) {
    ceil.store(f);
}

template<typename FloatType>
FloatType Controller<FloatType>::getGain() {
    return gain.load();
}

template<typename FloatType>
bool Controller<FloatType>::getIsPlaying() {
    return isPlaying.load();
}

template
class Controller<float>;

template
class Controller<double>;

template<typename FloatType>
ControllerAttach<FloatType>::ControllerAttach(Controller<FloatType> &gainController,
                                              juce::AudioProcessorValueTreeState &parameters) {
    controller = &gainController;
    apvts = &parameters;
    modeID.store(ZLDsp::mode::effect);
    startTimerHz(60);
    addListeners();
}

template<typename FloatType>
ControllerAttach<FloatType>::~ControllerAttach() {
    stopTimer();
    std::array IDs{ZLDsp::segment::ID, ZLDsp::window::ID, ZLDsp::lookahead::ID,
                   ZLDsp::strength::ID, ZLDsp::bound::ID, ZLDsp::gain::ID,
                   ZLDsp::ceil::ID,
                   ZLDsp::measurement::ID, ZLDsp::mode::ID};
    for (auto &ID: IDs) {
        apvts->removeParameterListener(ID, this);
    }
}

template<typename FloatType>
void ControllerAttach<FloatType>::timerCallback() {
    if (modeID == ZLDsp::mode::effect && controller->getIsPlaying()) {
        apvts->getParameter(ZLDsp::gain::ID)->beginChangeGesture();
        apvts->getParameter(ZLDsp::gain::ID)
                ->setValueNotifyingHost(
                        ZLDsp::gain::range.convertTo0to1(
                                static_cast<float>(controller->getGain())));
        apvts->getParameter(ZLDsp::gain::ID)->endChangeGesture();
    }
}

template<typename FloatType>
void ControllerAttach<FloatType>::addListeners() {
    std::array IDs{ZLDsp::segment::ID, ZLDsp::window::ID, ZLDsp::lookahead::ID,
                   ZLDsp::strength::ID, ZLDsp::bound::ID, ZLDsp::gain::ID,
                   ZLDsp::ceil::ID,
                   ZLDsp::measurement::ID, ZLDsp::mode::ID};
    for (auto &ID: IDs) {
        apvts->addParameterListener(ID, this);
    }
}

template<typename FloatType>
void ControllerAttach<FloatType>::parameterChanged(const juce::String &parameterID,
                                                   float newValue) {
    if (parameterID == ZLDsp::segment::ID) {
        controller->setSegmentToReset(newValue);
    } else if (parameterID == ZLDsp::window::ID) {
        controller->setWindow(newValue);
    } else if (parameterID == ZLDsp::lookahead::ID) {
        controller->setLookahead(newValue);
    } else if (parameterID == ZLDsp::strength::ID) {
        controller->setStrength(newValue);
    } else if (parameterID == ZLDsp::bound::ID) {
        controller->setBound(newValue);
    } else if (parameterID == ZLDsp::gain::ID) {
        if (modeID.load() == ZLDsp::mode::envelope) {
            controller->setGain(newValue);
        }
    } else if (parameterID == ZLDsp::ceil::ID) {
        controller->setCeil(static_cast<bool>(newValue));
    } else if (parameterID == ZLDsp::mode::ID) {
        modeID.store(static_cast<int>(newValue));
        if (modeID.load() == ZLDsp::mode::effect) {
            startTimerHz(10);
        } else {
            stopTimer();
        }
        controller->setModeID(static_cast<int>(newValue));
    } else if (parameterID == ZLDsp::measurement::ID) {
        controller->setMeasurementID(static_cast<int>(newValue));
    }
}

template
class ControllerAttach<float>;

template
class ControllerAttach<double>;
