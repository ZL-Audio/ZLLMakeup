#include "top_panel.h"

TopPanel::TopPanel(PluginProcessor &p, zlinterface::UIBase &base) : logoPanel(p, base) {
    // init sliders
    std::array<std::string, 1> sliderID{zldsp::segment::ID};
    zlpanel::attachSliders<zlinterface::RotarySliderComponent, 1>(*this, rotarySliderList, sliderAttachments, sliderID,
                                                                  p.parameters, base);

    // init buttons
    std::array<std::string, 3> buttonID{zldsp::ceil::ID, zldsp::accurate::ID, zldsp::sideout::ID};
    zlpanel::attachButtons<zlinterface::ButtonComponent, 3>(*this, buttonList, buttonAttachments, buttonID,
                                                            p.parameters, base);

    // init comboBoxes
    std::array<std::string, 2> boxID{zldsp::mode::ID, zldsp::measurement::ID};
    zlpanel::attachBoxes<zlinterface::ComboboxComponent, 2>(*this, boxList, boxAttachments, boxID,
                                                            p.parameters, base);

    // init changing value label
    gainValue = std::make_unique<zlinterface::ChangingValueComponent>(p.getController(), base);
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
            juce::GridItem(*gainValue).withArea(1, 2, 2, 4),
            juce::GridItem(*sideoutButton).withArea(1, 4, 2, 5),
            juce::GridItem(*modeBox).withArea(2, 1),
            juce::GridItem(*measurementBox).withArea(2, 2),
            juce::GridItem(*accurateButton).withArea(2, 4),
            juce::GridItem(*ceilButton).withArea(2, 3),
            juce::GridItem(*segmentSlider).withArea(1, 5, 3, 6),
    };
    grid.performLayout(getLocalBounds());
}

void TopPanel::setMode(int modeID) {
    if (modeID == zldsp::mode::effect) {
        for (auto const &c: {&segmentSlider}) {
            (*c)->setEditable(true);
        }
        for (auto const &c: {&ceilButton, &accurateButton}) {
            (*c)->setEditable(true);
        }
        for (auto const &c: {&modeBox, &measurementBox}) {
            (*c)->setEditable(true);
        }
    } else if (modeID == zldsp::mode::envelope) {
        for (auto const &c: {&segmentSlider}) {
            (*c)->setEditable(false);
        }
        for (auto const &c: {&ceilButton, &accurateButton}) {
            (*c)->setEditable(false);
        }
        for (auto const &c: {&measurementBox}) {
            (*c)->setEditable(false);
        }
    }
    triggerAsyncUpdate();
}

void TopPanel::handleAsyncUpdate() {
    repaint();
}