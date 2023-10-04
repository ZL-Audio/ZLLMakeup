#include "MainPanel.h"

MainPanel::MainPanel(juce::AudioProcessorValueTreeState &parameters,
                     Controller<float> *controller) : topPanel(parameters, controller),
                                                      bottomPanel(parameters) {
    apvts = &parameters;
    addAndMakeVisible(topPanel);
    addAndMakeVisible(bottomPanel);
    setMode(ZLDsp::mode::defaultI);
}

MainPanel::~MainPanel() = default;

void MainPanel::paint(juce::Graphics &g) {
    g.fillAll(ZLInterface::BackgroundColor);
    auto bound = getLocalBounds().toFloat();
    float fontSize = bound.getHeight() * 0.0514f;
    bound = ZLInterface::fillRoundedShadowRectangle(g, bound, fontSize * 0.5f);
    ZLInterface::fillRoundedInnerShadowRectangle(g, bound, fontSize * 0.5f,
                                                 fontSize * 0.15f,
                                                 true, true, true, true, true);
}

void MainPanel::resized() {
    topPanel.setFontSize(
            static_cast<float> (getLocalBounds().toFloat().getHeight()) * 0.0514f);
    bottomPanel.setFontSize(
            static_cast<float> (getLocalBounds().toFloat().getHeight()) * 0.0514f);

    juce::Grid grid;
    using Track = juce::Grid::TrackInfo;
    using Fr = juce::Grid::Fr;

    grid.templateRows = {Track(Fr(6)), Track(Fr(5))};
    grid.templateColumns = {Track(Fr(1))};

    juce::Array<juce::GridItem> items;
    items.add(topPanel);
    items.add(bottomPanel);
    grid.items = items;

    auto bound = getLocalBounds().toFloat();
    auto padding = bound.getHeight() * 0.09142857f;
    bound = bound.withSizeKeepingCentre(bound.getWidth() - padding,
                                        bound.getHeight() - padding);
    grid.performLayout(bound.toNearestInt());
}

void MainPanel::setMode(int modeID) {
    topPanel.setMode(modeID);
    bottomPanel.setMode(modeID);
}