#ifndef ZLLMAKEUP_DSP_DEFINES_H
#define ZLLMAKEUP_DSP_DEFINES_H

#include <juce_audio_processors/juce_audio_processors.h>

namespace ZLDsp {
    // float
    template<class T>
    class FloatParameters {
    public:
        static std::unique_ptr<juce::AudioParameterFloat> get() {
            return std::make_unique<juce::AudioParameterFloat>(T::ID, T::name, T::range,
                                                               T::defaultV);
        }
    };

    class segment : public FloatParameters<segment> {
    public:
        auto static constexpr ID = "segment";
        auto static constexpr name = "Segment (ms)";
        inline auto static const range =
                juce::NormalisableRange<float>(10.f, 100.f, 1.f);
        auto static constexpr defaultV = 20.f;
    };

    class window : public FloatParameters<window> {
    public:
        auto static constexpr ID = "window";
        auto static constexpr name = "Window";
        inline auto static const range =
                juce::NormalisableRange<float>(1.f, 50.0f, 1.f);
        auto static constexpr defaultV = 20.f;
    };

    class lookahead : public FloatParameters<lookahead> {
    public:
        auto static constexpr ID = "lookahead";
        auto static constexpr name = "Lookahead (%)";
        inline auto static const range =
                juce::NormalisableRange<float>(0.f, 100.0f, 1.f);
        auto static constexpr defaultV = 0.f;

        static float formatV(float v) { return v / 100.f; }
        static double formatV(double v) { return v / 100; }
    };

    class strength : public FloatParameters<strength> {
    public:
        auto static constexpr ID = "strength";
        auto static constexpr name = "Strength (%)";
        inline auto static const range =
                juce::NormalisableRange<float>(0.0f, 100.0f, 1.f);
        auto static constexpr defaultV = 100.0f;

        static float formatV(float v) { return v / 100.f; }
        static double formatV(double v) { return v / 100; }
    };

    class bound : public FloatParameters<bound> {
    public:
        auto static constexpr ID = "bound";
        auto static constexpr name = "Bound (dB)";
        inline auto static const range =
                juce::NormalisableRange<float>(0., 30.f, 0.1f);
        auto static constexpr defaultV = 30.f;
    };

    class gain : public FloatParameters<gain> {
    public:
        auto static constexpr ID = "gain";
        auto static constexpr name = "Gain (dB)";
        inline auto static const range =
                juce::NormalisableRange<float>(-30., 30.f, 0.01f);
        auto static constexpr defaultV = 0.f;
    };


    // bool
    template<class T>
    class BoolParameters {
    public:
        static std::unique_ptr<juce::AudioParameterBool> get() {
            return std::make_unique<juce::AudioParameterBool>(T::ID, T::name,
                                                              T::defaultV);
        }
    };

    class ceil : public BoolParameters<ceil> {
    public:
        auto static constexpr ID = "ceil";
        auto static constexpr name = "Ceil";
        auto static constexpr defaultV = false;
    };

    // choice
    template<class T>
    class ChoiceParameters {
    public:
        static std::unique_ptr<juce::AudioParameterChoice> get() {
            return std::make_unique<juce::AudioParameterChoice>(
                    T::ID, T::name, T::choices, T::defaultI);
        }
    };

    class mode : public ChoiceParameters<mode> {
    public:
        auto static constexpr ID = "mode";
        auto static constexpr name = "Mode";
        inline auto static const choices = juce::StringArray{"Effect", "Envelope"};
        int static constexpr defaultI = 0;
        enum {
            effect, envelope, modeNUM
        };
    };

    class measurement : public ChoiceParameters<measurement> {
    public:
        auto static constexpr ID = "measurement";
        auto static constexpr name = "Measurement";
        inline auto static const choices = juce::StringArray{"RMS", "K-RMS"};
        int static constexpr defaultI = 0;
        enum {
            rms, k_rms, loudnessNUM
        };

        template<typename FloatType>
        static FloatType getSilentLoudness() {
            return static_cast<FloatType>(-180);
        }

        template<typename FloatType>
        static std::vector<FloatType> getEmptyLoudness() {
            return std::vector<FloatType>(loudnessNUM, getSilentLoudness<FloatType>());
        }
    };

    inline juce::AudioProcessorValueTreeState::ParameterLayout getParameterLayout() {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;
        layout.add(segment::get(), window::get(), lookahead::get(),
                   strength::get(), bound::get(), gain::get(),
                   ceil::get(),
                   measurement::get(), mode::get());
        return layout;
    }
} // namespace ZLDsp

#endif //ZLLMAKEUP_DSP_DEFINES_H
