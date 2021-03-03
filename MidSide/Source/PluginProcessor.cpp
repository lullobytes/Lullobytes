/*
  ==============================================================================

    Lullobytes - Mid/Side

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MidSideProcessor::MidSideProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),  treeState(*this, nullptr, "PARAMETERS", createParameterLayout())

#else
    : AudioProcessor (BusesProperties()
                    #if ! JucePlugin_IsMidiEffect
                    #if ! JucePlugin_IsSynth
                      .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        #endif
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                    #endif
                     ), treeState(*this, nullptr, "PARAMETERS", createParameterLayout())

#endif
    {
        treeState.addParameterListener("drive", this);
        treeState.getParameter ("drive")->setValue (0.0f);
        treeState.addParameterListener("clean", this);
        treeState.getParameter ("clean")->setValue (0.0f);
        treeState.addParameterListener("linked", this);
        treeState.getParameter ("linked")->setValue (true);
    }
    



MidSideProcessor::~MidSideProcessor()
{
}

//==============================================================================
const juce::String MidSideProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MidSideProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MidSideProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MidSideProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MidSideProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MidSideProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MidSideProcessor::getCurrentProgram()
{
    return 0;
}

void MidSideProcessor::setCurrentProgram (int index)
{
}

const juce::String MidSideProcessor::getProgramName (int index)
{
    return {};
}

void MidSideProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout MidSideProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add(std::make_unique<juce::AudioParameterFloat>("drive", "Drive", 0.0f, 1.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("clean", "Clean", 0.0f, 1.0f, 1.0f));
    layout.add(std::make_unique<juce::AudioParameterBool>("linked", "Linked", true));
    return layout;
}

//==============================================================================
void MidSideProcessor::parameterChanged  (const juce::String &parameterID, float newValue)
{
    if (parameterID.compare("drive")==0)
    {
        drive = newValue;
        targetDrive.setTargetValue(drive);
    }
    else targetDrive.reset(getSampleRate(), 0.1f);

    if (parameterID.compare("clean")==0)
    {
        clean = newValue;
        targetClean.setTargetValue(clean);
    }
    else targetClean.reset(getSampleRate(), 0.1f);
    
    if (parameterID.compare("linked")==0)
    {
        if (linked)
        {
            linked = false;
        }
        else if (!linked)
        {
            linked = true;
        }
    }
}

void MidSideProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{

}

//==============================================================================
void MidSideProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}


//==============================================================================
#ifndef JucePlugin_PreferredChannelConfigurations
bool MidSideProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

//==============================================================================
void MidSideProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel); 
        
        for (int sample = 0; sample < buffer.getNumSamples(); sample++)
        {
            drive = targetDrive.getNextValue();
            clean = targetClean.getNextValue();
            
            float wetSignal = (2.0f / juce::float_Pi) * atanf(*channelData * drive * 10.0f);
            *channelData = (clean * *channelData) + (wetSignal*0.707f);
            channelData++;
        }
        
    }
    

}

//==============================================================================
bool MidSideProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* MidSideProcessor::createEditor()
{
    return new MidSideProcessorEditor (*this);
}

//==============================================================================
void MidSideProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = treeState.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void MidSideProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    
    if (xmlState.get() != nullptr)
    {
        
        if (xmlState->hasTagName (treeState.state.getType()))
        {
            treeState.replaceState (juce::ValueTree::fromXml (*xmlState));
        }
        
    }

}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MidSideProcessor();
}