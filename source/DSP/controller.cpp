#include "controller.h"

template<typename FloatType>
Controller<FloatType>::Controller(juce::AudioProcessor *processor,
                                  juce::AudioProcessorValueTreeState &parameters) :
        fixedAudioBuffer() {
    processorRef = processor;

    gain.store(zldsp::gain::defaultV);
    lookahead.store(zldsp::lookahead::defaultV);
    bound.store(zldsp::bound::defaultV);
    strength.store(zldsp::strength::defaultV);
    setSensitivity(zldsp::sensitivity::defaultV);
    setSegment(zldsp::segment::defaultV);
    setWindow(zldsp::window::defaultV);

    ceil.store(zldsp::ceil::defaultV);
    accurate.store(zldsp::accurate::defaultV);
    sideout.store(zldsp::sideout::defaultV);

    modeID.store(zldsp::mode::defaultI);
    setMeasurementID(zldsp::measurement::defaultI);
}

template<typename FloatType>
void Controller<FloatType>::prepareToPlay(juce::dsp::ProcessSpec spec) {

    spec.numChannels = spec.numChannels * 2;
    fixedAudioBuffer.prepare(spec);
}

template<typename FloatType>
void Controller<FloatType>::processBlock(juce::AudioBuffer<FloatType> &buffer) {
    auto currentPos = processorRef->getPlayHead()->getPosition();
    if (std::abs(lastBufferSize + lastBufferTime -
                 currentPos->getTimeInSamples().orFallback(0)) > buffer.getNumSamples()) {
        isPlaying.store(false);
        if (modeID.load() == zldsp::mode::effect) {
            gain.store(0);
            gainDSP.setGainDecibels(0);
        }
        for (auto &f: {&mainSubTracker, &auxSubTracker, &mainTracker, &auxTracker}) {
            (*f).reset();
        }
    } else {
        isPlaying.store(true);
    }
    lastBufferSize = buffer.getNumSamples();
    lastBufferTime = currentPos->getTimeInSamples().orFallback(0);
    if (modeID.load() == zldsp::mode::effect && isPlaying.load()) {
        fixedAudioBuffer.pushBuffer(buffer);
        while (fixedAudioBuffer.isSubReady()) {
            fixedAudioBuffer.popSubBuffer();

            // calculate loudness makeup
            auto mainBusNumChannel = static_cast<int>(
                    fixedAudioBuffer.getSubSpec().numChannels / 2);
            mainSubTracker.process(
                    fixedAudioBuffer.getSubBufferChannels(0, mainBusNumChannel));
            auxSubTracker.process(fixedAudioBuffer.getSubBufferChannels(mainBusNumChannel,
                                                                        mainBusNumChannel));
            auto actualGain = auxSubTracker.getMomentaryLoudness() -
                              mainSubTracker.getMomentaryLoudness();
            // apply delay
            juce::dsp::AudioBlock<FloatType> block(fixedAudioBuffer.subBuffer);
            auto mainBlock = block.getSubsetChannelBlock(0,
                                                         static_cast<size_t>(mainBusNumChannel));
            delayLineDSP.process(juce::dsp::ProcessContextReplacing<FloatType>(block));
            // apply ceil
            if (ceil.load()) {
                actualGain = juce::jmin(actualGain,
                                        -juce::Decibels::gainToDecibels(
                                                fixedAudioBuffer.getSubBufferChannels(
                                                        0,
                                                        mainBusNumChannel).getMagnitude(
                                                        0,
                                                        static_cast<int>(fixedAudioBuffer.getSubSpec().maximumBlockSize))));
            }
            // apply bound
            actualGain = juce::jlimit(-bound.load(), bound.load(), actualGain);
            actualGain = static_cast<FloatType>(static_cast<int>(std::round(
                    actualGain * 1000))) / 1000;
            // compare with sensitivity
            if (accurate.load() && std::abs(mainTracker.getIntegratedTotalLoudness() -
                                            auxTracker.getIntegratedTotalLoudness()) >=
                                   10 * sensitivity.load()) {
                mainTracker.reset();
                auxTracker.reset();
                gain.store(actualGain);
                gainDSP.setGainDecibels(
                        gain.load() * zldsp::strength::formatV(strength.load()));
            } else if (std::abs(gain.load() - actualGain) >= sensitivity.load()) {
                mainTracker.reset();
                auxTracker.reset();
                gain.store(actualGain);
                gainDSP.setGainDecibels(
                        gain.load() * zldsp::strength::formatV(strength.load()));
            }
            // apply final gain
            gainDSP.process(juce::dsp::ProcessContextReplacing<FloatType>(mainBlock));
            if (accurate.load()) {
                mainTracker.process(
                        fixedAudioBuffer.getSubBufferChannels(0, mainBusNumChannel));
                auxTracker.process(
                        fixedAudioBuffer.getSubBufferChannels(mainBusNumChannel,
                                                              mainBusNumChannel));
            }
            if (sideout.load()) {
                mainBlock.copyFrom(block.getSubsetChannelBlock(
                        static_cast<size_t>(mainBusNumChannel),
                        static_cast<size_t>(mainBusNumChannel)));
            }
            fixedAudioBuffer.pushSubBuffer();
        }
        fixedAudioBuffer.popBuffer(buffer);
    } else if (modeID.load() == zldsp::mode::envelope) {
        auto currentGain = juce::Decibels::decibelsToGain(
                gain.load() * zldsp::strength::formatV(strength.load()));
        auto mainBuffer = processorRef->getBusBuffer(buffer, true, 0);
        if (sideout.load()) {
            mainBuffer.makeCopyOf(processorRef->getBusBuffer(buffer, true, 1), true);
        } else {
            mainBuffer.applyGain(currentGain);
        }
    }
}

template<typename FloatType>
void Controller<FloatType>::setSegment(FloatType v) {
    segment.store(v);
    const juce::GenericScopedLock<juce::CriticalSection> processLock(processorRef->getCallbackLock());
    auto subBufferSize = static_cast<int>(std::round(
            v * fixedAudioBuffer.getMainSpec().sampleRate / 1000));
    fixedAudioBuffer.setSubBufferSize(subBufferSize);

    auto subSpec = fixedAudioBuffer.getSubSpec();
    auto subBusSpec = subSpec;
    delayLineDSP.prepare(subBusSpec);
    subBusSpec.numChannels = subBusSpec.numChannels / 2;
    for (auto &f: {&mainSubTracker, &auxSubTracker, &mainTracker, &auxTracker}) {
        (*f).prepareToPlay(subBusSpec);
    }
    gainDSP.prepare(subBusSpec);
    gainDSP.setRampDurationSeconds(static_cast<double>(1) / 1024);
    delayLineDSP.setMaximumDelayInSamples(
            subBufferSize * static_cast<int>(zldsp::lookahead::range.end));
    setLookahead(lookahead.load());
}

template<typename FloatType>
void Controller<FloatType>::setLookahead(FloatType v, bool useLock) {
    lookahead.store(v);
    if (useLock) {
        const juce::GenericScopedLock<juce::CriticalSection> processLock(processorRef->getCallbackLock());
        toSetLookAhead();
    } else {
        toSetLookAhead();
    }
}

template<typename FloatType>
void Controller<FloatType>::toSetLookAhead() {
    if (modeID.load() == zldsp::mode::effect) {
        auto latencyInSamples =
                static_cast<int>(zldsp::lookahead::formatV(lookahead.load()) * window.load() *
                                 static_cast<FloatType>(fixedAudioBuffer.getSubSpec().maximumBlockSize));
        delayLineDSP.setDelay(static_cast<FloatType>(latencyInSamples));
        processorRef->setLatencySamples(
                static_cast<int>(fixedAudioBuffer.getLatencySamples()) + latencyInSamples);
    }
}


template<typename FloatType>
void Controller<FloatType>::setWindow(FloatType v) {
    const juce::GenericScopedLock<juce::CriticalSection> processLock(processorRef->getCallbackLock());
    window.store(v);
    for (auto &f: {&mainSubTracker, &auxSubTracker}) {
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
void Controller<FloatType>::setSensitivity(FloatType v) {
    externSensitivity.store(v);
    sensitivity.store(zldsp::sensitivity::formatV(v));
}

template<typename FloatType>
void Controller<FloatType>::setModeID(int ID) {
    modeID.store(ID);
    if (modeID.load() == zldsp::mode::effect) {
        setSegment(segment.load());
    } else if (modeID.load() == zldsp::mode::envelope) {
        processorRef->setLatencySamples(0);
    }
}

template<typename FloatType>
void Controller<FloatType>::setMeasurementID(int ID) {
    if (ID == zldsp::measurement::rms) {
        for (auto &f: {&mainSubTracker, &auxSubTracker, &mainTracker, &auxTracker}) {
            (*f).setKWeight(false);
        }
    } else if (ID == zldsp::measurement::k_rms) {
        for (auto &f: {&mainSubTracker, &auxSubTracker, &mainTracker, &auxTracker}) {
            (*f).setKWeight(true);
        }
    }
}

template<typename FloatType>
void Controller<FloatType>::setCeil(bool f) {
    ceil.store(f);
}

template<typename FloatType>
void Controller<FloatType>::setAccurate(bool f) {
    accurate.store(f);
    if (f) {
        mainTracker.reset();
        auxTracker.reset();
    }
}

template<typename FloatType>
void Controller<FloatType>::setSideout(bool f) {
    sideout.store(f);
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
    modeID.store(zldsp::mode::defaultI);
    startTimerHz(60);
    addListeners();
}

template<typename FloatType>
ControllerAttach<FloatType>::~ControllerAttach() {
    stopTimer();
    std::array IDs{zldsp::segment::ID, zldsp::window::ID, zldsp::lookahead::ID, zldsp::strength::ID, zldsp::bound::ID,
                   zldsp::gain::ID, zldsp::sensitivity::ID, zldsp::ceil::ID, zldsp::accurate::ID, zldsp::sideout::ID,
                   zldsp::measurement::ID, zldsp::mode::ID};
    for (auto &ID: IDs) {
        apvts->removeParameterListener(ID, this);
    }
}

template<typename FloatType>
void ControllerAttach<FloatType>::timerCallback() {
    if (modeID == zldsp::mode::effect && controller->getIsPlaying()) {
        apvts->getParameter(zldsp::gain::ID)->beginChangeGesture();
        apvts->getParameter(zldsp::gain::ID)
                ->setValueNotifyingHost(
                        zldsp::gain::range.convertTo0to1(
                                static_cast<float>(controller->getGain())));
        apvts->getParameter(zldsp::gain::ID)->endChangeGesture();
    }
}

template<typename FloatType>
void ControllerAttach<FloatType>::addListeners() {
    std::array IDs{zldsp::segment::ID, zldsp::window::ID, zldsp::lookahead::ID, zldsp::strength::ID, zldsp::bound::ID,
                   zldsp::gain::ID, zldsp::sensitivity::ID, zldsp::ceil::ID, zldsp::accurate::ID, zldsp::sideout::ID,
                   zldsp::measurement::ID, zldsp::mode::ID};
    for (auto &ID: IDs) {
        apvts->addParameterListener(ID, this);
    }
}

template<typename FloatType>
void ControllerAttach<FloatType>::parameterChanged(const juce::String &parameterID,
                                                   float newValue) {
    if (parameterID == zldsp::segment::ID) {
        controller->setSegment(newValue);
    } else if (parameterID == zldsp::window::ID) {
        controller->setWindow(newValue);
    } else if (parameterID == zldsp::lookahead::ID) {
        controller->setLookahead(newValue);
    } else if (parameterID == zldsp::strength::ID) {
        controller->setStrength(newValue);
    } else if (parameterID == zldsp::bound::ID) {
        controller->setBound(newValue);
    } else if (parameterID == zldsp::sensitivity::ID) {
        controller->setSensitivity(newValue);
    } else if (parameterID == zldsp::gain::ID) {
        if (modeID.load() == zldsp::mode::envelope) {
            controller->setGain(newValue);
        }
    } else if (parameterID == zldsp::ceil::ID) {
        controller->setCeil(static_cast<bool>(newValue));
    } else if (parameterID == zldsp::accurate::ID) {
        controller->setAccurate(static_cast<bool>(newValue));
    } else if (parameterID == zldsp::sideout::ID) {
        controller->setSideout(static_cast<bool>(newValue));
    } else if (parameterID == zldsp::mode::ID) {
        modeID.store(static_cast<int>(newValue));
        controller->setModeID(static_cast<int>(newValue));
        if (static_cast<int>(newValue) == zldsp::mode::effect) {
            startTimerHz(60);
        } else {
            stopTimer();
        }
    } else if (parameterID == zldsp::measurement::ID) {
        controller->setMeasurementID(static_cast<int>(newValue));
    }
}

template
class ControllerAttach<float>;

template
class ControllerAttach<double>;
