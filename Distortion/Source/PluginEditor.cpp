/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DistortionAudioProcessorEditor::DistortionAudioProcessorEditor (DistortionAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    gui();
}

DistortionAudioProcessorEditor::~DistortionAudioProcessorEditor()
{
}

//==============================================================================
void DistortionAudioProcessorEditor::paint (juce::Graphics& g)
{

    g.fillAll (juce::Colours::burlywood);
    g.setColour (juce::Colours::black);
    g.setFont (15.0f);
    //g.drawFittedText ("Lulloybtes Distortion", getLocalBounds(), juce::Justification::centred, 1);

}

void DistortionAudioProcessorEditor::resized()
{
    frameImageComponent.setBounds(frameArea);
    
    infoImageComponent.setBounds(infoArea);
    
    logoImageComponent.setBounds(logoArea);
    
    buttonImageComponent.setBounds(buttonArea);
    
    dOffImageComponent.setBounds(dImageArea);
    dOnImageComponent.setBounds(dImageArea);
    dKnobImageComponent.setBounds(dKnobArea);
    dMarkerImageComponent.setBounds(dMarkerArea);
    dSlider.setBounds(dKnobArea);
    
    cOffImageComponent.setBounds(cImageArea);
    cOnImageComponent.setBounds(cImageArea);
    cKnobImageComponent.setBounds(cKnobArea);
    cMarkerImageComponent.setBounds(cMarkerArea);
    cSlider.setBounds(cKnobArea);
}

void DistortionAudioProcessorEditor:: gui()
{
    setSize (400, 400);
    
    infoImage = juce::ImageCache::getFromMemory (BinaryData::info_png, BinaryData::info_pngSize);
    infoImageComponent.setImage(infoImage);
    addAndMakeVisible (&infoImageComponent);
    infoImageComponent.setVisible(buttonImageComponent.getToggleState());
    
    logoImage = juce::ImageCache::getFromMemory (BinaryData::logo_png, BinaryData::logo_pngSize);
    logoImageComponent.setImage(logoImage);
    addAndMakeVisible (&logoImageComponent);
    
    frameImage = juce::ImageCache::getFromMemory (BinaryData::frame_png, BinaryData::frame_pngSize);
    frameImageComponent.setImage(frameImage);
    addAndMakeVisible (&frameImageComponent);
    
    dOffImage = juce::ImageCache::getFromMemory (BinaryData::d_off_png, BinaryData::d_off_pngSize);
    dOffImageComponent.setImage(dOffImage);
    addAndMakeVisible (&dOffImageComponent);
    
    cOffImage = juce::ImageCache::getFromMemory (BinaryData::c_off_png, BinaryData::c_off_pngSize);
    cOffImageComponent.setImage(cOffImage);
    addAndMakeVisible (&cOffImageComponent);

    dKnobImage = juce::ImageCache::getFromMemory (BinaryData::knob_png, BinaryData::knob_pngSize);
    dKnobImageComponent.setImage(dKnobImage);
    dKnobImageComponent.setBufferedToImage(true);
    addAndMakeVisible (&dKnobImageComponent);
    
    cKnobImage = juce::ImageCache::getFromMemory (BinaryData::knob_png, BinaryData::knob_pngSize);
    cKnobImageComponent.setImage(cKnobImage);
    cKnobImageComponent.setBufferedToImage(true);
    addAndMakeVisible (&cKnobImageComponent);
    
    dSlider.setSliderStyle (juce::Slider::Rotary);
    dSlider.setRange (0, 1);
    dSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    dSlider.setColour(juce::Slider::ColourIds::backgroundColourId, juce::Colours::burlywood);
    dSlider.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, juce::Colours::burlywood);
    dSlider.setColour(juce::Slider::ColourIds::rotarySliderFillColourId, juce::Colours::burlywood);
    dSlider.setColour(juce::Slider::ColourIds::trackColourId, juce::Colours::burlywood);
    dSlider.setColour(juce::Slider::ColourIds::thumbColourId, juce::Colours::burlywood);
   
    dSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, "drive", dSlider);;
    
    dKnobImageComponent.setTransform(dKnobTransform.rotation(4.72f*  *audioProcessor.treeState.getRawParameterValue("drive"),dKnobArea.getCentreX(), dKnobArea.getCentreY()));
    
    dSlider.onValueChange = [this]
    {
        dKnobImageComponent.setTransform(dKnobTransform.rotation(4.72f*  *audioProcessor.treeState.getRawParameterValue("drive"),dKnobArea.getCentreX(), dKnobArea.getCentreY()));
        if(dSlider.getValue()>0.01f)
        dOnImageComponent.setAlpha(0.5f * dSlider.getValue() + 0.5f);
        else dOnImageComponent.setAlpha(0.0f);
    };
    
    *audioProcessor.treeState.getRawParameterValue("drive") = dSlider.getValue();
    audioProcessor.treeState.addParameterListener("drive", &audioProcessor);
    addAndMakeVisible (dSlider);
    
    cSlider.setSliderStyle (juce::Slider::Rotary);
    cSlider.setRange (0, 1.0);
    cSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    cSlider.setColour(juce::Slider::ColourIds::backgroundColourId, juce::Colours::burlywood);
    cSlider.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, juce::Colours::burlywood);
    cSlider.setColour(juce::Slider::ColourIds::rotarySliderFillColourId, juce::Colours::burlywood);
    cSlider.setColour(juce::Slider::ColourIds::trackColourId, juce::Colours::burlywood);
    cSlider.setColour(juce::Slider::ColourIds::thumbColourId, juce::Colours::burlywood);
  
    cSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, "clean", cSlider);
    cKnobImageComponent.setTransform(cKnobTransform.rotation(4.72f* *audioProcessor.treeState.getRawParameterValue("clean"),cKnobArea.getCentreX(), cKnobArea.getCentreY()));
    
    cSlider.onValueChange = [this]
    {
        cKnobImageComponent.setTransform(cKnobTransform.rotation(4.72f* *audioProcessor.treeState.getRawParameterValue("clean"),cKnobArea.getCentreX(), cKnobArea.getCentreY()));
        if(cSlider.getValue()>0.01f)
            cOnImageComponent.setAlpha(0.5f * cSlider.getValue() + 0.5f);
        else cOnImageComponent.setAlpha(0.0f);
    };
    
    *audioProcessor.treeState.getRawParameterValue("clean") = cSlider.getValue();
    audioProcessor.treeState.addParameterListener("clean", &audioProcessor);
    addAndMakeVisible (cSlider);
    
    dOnImage = juce::ImageCache::getFromMemory (BinaryData::d_on_png, BinaryData::d_on_pngSize);
    dOnImageComponent.setImage(dOnImage);
    dOnImageComponent.setAlpha(dSlider.getValue());
    addAndMakeVisible (&dOnImageComponent);
    
    cOnImage = juce::ImageCache::getFromMemory (BinaryData::c_on_png, BinaryData::c_on_pngSize);
    cOnImageComponent.setImage(cOnImage);
    cOnImageComponent.setAlpha(cSlider.getValue());
    addAndMakeVisible (&cOnImageComponent);
    
    dMarkerImage = juce::ImageCache::getFromMemory (BinaryData::marker_png, BinaryData::marker_pngSize);
    dMarkerImageComponent.setImage(dMarkerImage);
    addAndMakeVisible (&dMarkerImageComponent);
    
    cMarkerImage = juce::ImageCache::getFromMemory (BinaryData::marker_png, BinaryData::marker_pngSize);
    cMarkerImageComponent.setImage(cMarkerImage);
    addAndMakeVisible (&cMarkerImageComponent);
 
    buttonOffImage = juce::ImageCache::getFromMemory (BinaryData::button_off_png, BinaryData::button_off_pngSize);
    buttonOnImage = juce::ImageCache::getFromMemory (BinaryData::button_on_png, BinaryData::button_on_pngSize);
    buttonImageComponent.setImages (false, true, true, buttonOffImage, 1.0f, {}, {}, 1.0f, {}, buttonOnImage, 1.0f, {});
    buttonImageComponent.setClickingTogglesState(true);
    buttonImageComponent.onStateChange = [this]
    {
        infoImageComponent.setVisible(buttonImageComponent.getToggleState());
        dSlider.setVisible(!buttonImageComponent.getToggleState());
        cSlider.setVisible(!buttonImageComponent.getToggleState());
        dOnImageComponent.setVisible(!buttonImageComponent.getToggleState());
        cOnImageComponent.setVisible(!buttonImageComponent.getToggleState());
        dOffImageComponent.setVisible(!buttonImageComponent.getToggleState());
        cOffImageComponent.setVisible(!buttonImageComponent.getToggleState());
        dKnobImageComponent.setVisible(!buttonImageComponent.getToggleState());
        cKnobImageComponent.setVisible(!buttonImageComponent.getToggleState());
        dMarkerImageComponent.setVisible(!buttonImageComponent.getToggleState());
        cMarkerImageComponent.setVisible(!buttonImageComponent.getToggleState());
    };
    addAndMakeVisible (&buttonImageComponent);
    
}





