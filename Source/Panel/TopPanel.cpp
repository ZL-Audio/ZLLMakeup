#include "TopPanel.h"

TopPanel::TopPanel(juce::AudioProcessorValueTreeState &parameters,
                   Controller<float> *controller) {
    // init sliders
    std::array sliderList{&segmentSlider};
    std::array sliderID{ZLDsp::segment::ID};
    for (size_t i = 0; i < sliderList.size(); ++i) {
        *sliderList[i] = std::make_unique<RotarySliderComponent>(
                parameters.getParameter(sliderID[i])->name);
        addAndMakeVisible(*(*sliderList[i]));
        sliderAttachments.add(
                std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                        parameters, sliderID[i], (*sliderList[i])->getSlider()));
    }
    // init buttons
    std::array buttonList{&ceilButton, &accurateButton};
    std::array buttonID{ZLDsp::ceil::ID, ZLDsp::accurate::ID};
    for (size_t i = 0; i < buttonList.size(); ++i) {
        *buttonList[i] = std::make_unique<ButtonComponent>(
                parameters.getParameter(buttonID[i])->name);
        addAndMakeVisible(*(*buttonList[i]));
        buttonAttachments.add(
                std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
                        parameters, buttonID[i], (*buttonList[i])->getButton()));
    }
    // init comboBoxes
    std::array comboboxList{&modeBox, &measurementBox};
    std::array comboboxID{ZLDsp::mode::ID, ZLDsp::measurement::ID};
    for (size_t i = 0; i < comboboxList.size(); ++i) {
        *comboboxList[i] = std::make_unique<ComboboxComponent>(
                parameters.getParameter(comboboxID[i])->name,
                parameters.getParameter(comboboxID[i])->getAllValueStrings());
        addAndMakeVisible(*(*comboboxList[i]));
        comboboxAttachments.add(
                std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
                        parameters, comboboxID[i], (*comboboxList[i])->getComboBox()));
    }
    // init changing value label
    gainValue = std::make_unique<ChangingValueComponent>(controller);
    addAndMakeVisible(*gainValue);

    addAndMakeVisible(logoPanel);
}

TopPanel::~TopPanel() = default;

void TopPanel::paint(juce::Graphics &) {}

void TopPanel::resized() {
    juce::Grid grid;
    using Track = juce::Grid::TrackInfo;
    using Fr = juce::Grid::Fr;

    grid.templateRows = {Track(Fr(1)), Track(Fr(1))};
    grid.templateColumns = {Track(Fr(1)), Track(Fr(1)), Track(Fr(1)),
                            Track(Fr(1)), Track(Fr(1))};

    grid.items = {
            juce::GridItem(logoPanel).withArea(1, 1),
            juce::GridItem(*gainValue).withArea(1, 2, 2, 5),
            juce::GridItem(*modeBox).withArea(2, 1),
            juce::GridItem(*measurementBox).withArea(2, 2),
            juce::GridItem(*accurateButton).withArea(2, 4),
            juce::GridItem(*ceilButton).withArea(2, 3),
            juce::GridItem(*segmentSlider).withArea(1, 5, 3, 6),
    };

    grid.performLayout(getLocalBounds());
}

void TopPanel::setFontSize(float size) {
    std::array sliderList{&segmentSlider};
    for (auto const &c: sliderList) {
        (*c)->setFontSize(size);
    }
    std::array buttonList{&ceilButton, &accurateButton};
    for (auto const &c: buttonList) {
        (*c)->setFontSize(size);
    }
    std::array comboboxList{&modeBox, &measurementBox};
    for (auto const &c: comboboxList) {
        (*c)->setFontSize(size);
    }
    gainValue->setFontSize(size);
}

void TopPanel::setMode(int modeID) {
    if (modeID == ZLDsp::mode::effect) {
        std::array sliderList{&segmentSlider};
        for (auto const &c: sliderList) {
            (*c)->setEditable(true);
        }
        std::array buttonList{&ceilButton, &accurateButton};
        for (auto const &c: buttonList) {
            (*c)->setEditable(true);
        }
        std::array comboboxList{&modeBox, &measurementBox};
        for (auto const &c: comboboxList) {
            (*c)->setEditable(true);
        }
    } else if (modeID == ZLDsp::mode::envelope) {
        std::array sliderList{&segmentSlider};
        for (auto const &c: sliderList) {
            (*c)->setEditable(false);
        }
        std::array buttonList{&ceilButton, &accurateButton};
        for (auto const &c: buttonList) {
            (*c)->setEditable(false);
        }
        std::array comboboxList{&measurementBox};
        for (auto const &c: comboboxList) {
            (*c)->setEditable(false);
        }
    }
}