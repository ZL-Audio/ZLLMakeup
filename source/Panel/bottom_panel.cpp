#include "bottom_panel.h"

BottomPanel::BottomPanel(PluginProcessor &p, zlinterface::UIBase &base) {
    // init sliders
    std::array<std::string, 5> sliderID{zldsp::strength::ID, zldsp::bound::ID, zldsp::window::ID, zldsp::lookahead::ID,
                                        zldsp::sensitivity::ID};
    zlpanel::attachSliders<zlinterface::RotarySliderComponent, 5>(*this, rotarySliderList, sliderAttachments, sliderID,
                                                                  p.parameters, base);

}

BottomPanel::~BottomPanel() = default;

void BottomPanel::paint(juce::Graphics &) {}

void BottomPanel::resized() {
    juce::Grid grid;
    using Track = juce::Grid::TrackInfo;
    using Fr = juce::Grid::Fr;

    grid.templateRows = {Track(Fr(1))};
    grid.templateColumns = {Track(Fr(1)), Track(Fr(1)), Track(Fr(1)),
                            Track(Fr(1)), Track(Fr(1))};

    juce::Array<juce::GridItem> items;
    items.add(*strengthSlider);
    items.add(*boundSlider);
    items.add(*sensitivitySlider);
    items.add(*windowSlider);
    items.add(*lookaheadSlider);
    grid.items = items;

    grid.performLayout(getLocalBounds());
}

void BottomPanel::setMode(int modeID) {
    if (modeID == zldsp::mode::effect) {
        std::array _sliderList{&strengthSlider, &boundSlider, &windowSlider,
                              &lookaheadSlider, &sensitivitySlider};
        for (auto const &s: _sliderList) {
            (*s)->setEditable(true);
        }
    } else if (modeID == zldsp::mode::envelope) {
        std::array _sliderList{&boundSlider, &windowSlider, &lookaheadSlider,
                              &sensitivitySlider};
        for (auto const &s: _sliderList) {
            (*s)->setEditable(false);
        }
    }
    triggerAsyncUpdate();
}

void BottomPanel::handleAsyncUpdate() {
    repaint();
}