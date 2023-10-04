#include "main_panel.h"

MainPanel::MainPanel(PluginProcessor &p) : topPanel(p, uiBase), bottomPanel(p, uiBase) {
    addAndMakeVisible(topPanel);
    addAndMakeVisible(bottomPanel);
    setMode(zldsp::mode::defaultI);
}

MainPanel::~MainPanel() = default;

void MainPanel::paint(juce::Graphics &g) {
    g.fillAll(uiBase.getBackgroundColor());
    auto bound = getLocalBounds().toFloat();
    float fontSize = bound.getHeight() * 0.0589947298f;
    bound = uiBase.fillRoundedShadowRectangle(g, bound, fontSize * 0.5f, {});
    uiBase.fillRoundedInnerShadowRectangle(g, bound, fontSize * 0.5f, {.blurRadius=0.45f, .flip=true});
}

void MainPanel::resized() {
    auto bound = getLocalBounds().toFloat();
    auto fontSize = bound.getHeight() * 0.0589947298f;
    bound = uiBase.getRoundedShadowRectangleArea(bound, fontSize * 0.5f, {});
    bound = uiBase.getRoundedShadowRectangleArea(bound, fontSize * 0.5f, {});

    uiBase.setFontSize(fontSize);

    juce::Grid grid;
    using Track = juce::Grid::TrackInfo;
    using Fr = juce::Grid::Fr;

    grid.templateRows = {Track(Fr(1)), Track(Fr(1))};
    grid.templateColumns = {Track(Fr(1))};

    juce::Array<juce::GridItem> items;
    items.add(topPanel);
    items.add(bottomPanel);
    grid.items = items;

    grid.performLayout(bound.toNearestInt());
}

void MainPanel::setMode(int modeID) {
    topPanel.setMode(modeID);
    bottomPanel.setMode(modeID);
}