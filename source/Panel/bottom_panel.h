#ifndef ZLLMATCH_BOTTOMPANEL_H
#define ZLLMATCH_BOTTOMPANEL_H

#include "../DSP/dsp_definitions.h"
#include "../GUI/interface_definitions.h"
#include "../GUI/rotary_slider_component.h"
#include "../PluginProcessor.h"
#include "panel_definitions.h"
#include <juce_audio_processors/juce_audio_processors.h>

class BottomPanel : public juce::Component, private juce::AsyncUpdater {
public:
    explicit BottomPanel(PluginProcessor &p, zlinterface::UIBase &base);

    ~BottomPanel() override;

    void paint(juce::Graphics &) override;

    void resized() override;

    void setMode(int modeID);

private:
    std::unique_ptr<zlinterface::RotarySliderComponent> strengthSlider, boundSlider, windowSlider, lookaheadSlider, sensitivitySlider;
    juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment> sliderAttachments;
    std::array<std::unique_ptr<zlinterface::RotarySliderComponent> *, 5> rotarySliderList{&strengthSlider, &boundSlider,
                                                                                          &windowSlider,
                                                                                          &lookaheadSlider,
                                                                                          &sensitivitySlider};
    void handleAsyncUpdate() override;
};

#endif //ZLLMATCH_BOTTOMPANEL_H
