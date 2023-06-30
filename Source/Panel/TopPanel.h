#ifndef ZLLMATCH_TOPPANEL_H
#define ZLLMATCH_TOPPANEL_H

#include <juce_audio_processors/juce_audio_processors.h>
#include "../DSP/dsp_defines.h"
#include "../GUI/interface_defines.h"
#include "../GUI/ComboboxComponent.h"
#include "../GUI/RotarySliderComponent.h"
#include "../GUI/ButtonComponent.h"
#include "../GUI/ChangingValueComponent.h"
#include "LogoPanel.h"

class TopPanel : public juce::Component {
public:
    explicit TopPanel(juce::AudioProcessorValueTreeState &parameters,
                      Controller<float> *controller);

    ~TopPanel() override;

    void paint(juce::Graphics &) override;

    void resized() override;

    void setFontSize(float size);

    void setMode(int modeID);

private:
    LogoPanel logoPanel;
    std::unique_ptr<ChangingValueComponent> gainValue;
    std::unique_ptr<ComboboxComponent> modeBox, measurementBox;
    juce::OwnedArray<juce::AudioProcessorValueTreeState::ComboBoxAttachment> comboboxAttachments;
    std::unique_ptr<ButtonComponent> ceilButton;
    juce::OwnedArray<juce::AudioProcessorValueTreeState::ButtonAttachment> buttonAttachments;
    std::unique_ptr<RotarySliderComponent> segmentSlider;
    juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment> sliderAttachments;
};

#endif //ZLLMATCH_TOPPANEL_H
