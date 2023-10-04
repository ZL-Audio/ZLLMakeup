#ifndef ZLLMATCH_TOPPANEL_H
#define ZLLMATCH_TOPPANEL_H

#include "../DSP/dsp_definitions.h"
#include "../GUI/button_component.h"
#include "../GUI/changing_value_component.h"
#include "../GUI/combobox_component.h"
#include "../GUI/interface_definitions.h"
#include "../GUI/rotary_slider_component.h"
#include "../PluginProcessor.h"
#include "logo_panel.h"
#include "panel_definitions.h"
#include <juce_audio_processors/juce_audio_processors.h>

class TopPanel : public juce::Component, private juce::AsyncUpdater {
public:
    explicit TopPanel(PluginProcessor &p, zlinterface::UIBase &base);

    ~TopPanel() override;

    void paint(juce::Graphics &) override;

    void resized() override;

    void setMode(int modeID);

private:
    zlpanel::LogoPanel logoPanel;
    std::unique_ptr<zlinterface::ChangingValueComponent> gainValue;

    std::unique_ptr<zlinterface::ComboboxComponent> modeBox, measurementBox;
    juce::OwnedArray<juce::AudioProcessorValueTreeState::ComboBoxAttachment> boxAttachments;
    std::array<std::unique_ptr<zlinterface::ComboboxComponent> *, 2> boxList{&modeBox, &measurementBox};

    std::unique_ptr<zlinterface::ButtonComponent> ceilButton, accurateButton, sideoutButton;
    juce::OwnedArray<juce::AudioProcessorValueTreeState::ButtonAttachment> buttonAttachments;
    std::array<std::unique_ptr<zlinterface::ButtonComponent> *, 3> buttonList{&ceilButton, &accurateButton,
                                                                              &sideoutButton};

    std::unique_ptr<zlinterface::RotarySliderComponent> segmentSlider;
    juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment> sliderAttachments;
    std::array<std::unique_ptr<zlinterface::RotarySliderComponent> *, 1> rotarySliderList{&segmentSlider};

    void handleAsyncUpdate() override;
};

#endif //ZLLMATCH_TOPPANEL_H
