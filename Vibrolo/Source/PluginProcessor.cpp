/*
 ==============================================================================
 
 Lullobytes - Vibrolo (vibrato and tremolo) processor.
 
 ==============================================================================
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
VibroloAudioProcessor::VibroloAudioProcessor()
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
    treeState.addParameterListener("vibe", this);
    treeState.getParameter ("vibe")->setValue (0.0f);
    treeState.addParameterListener("vibeRate", this);
    treeState.getParameter ("vibeRate")->setValue (0.0f);
    treeState.addParameterListener("trem", this);
    treeState.getParameter ("trem")->setValue (0.0f);
    treeState.addParameterListener("tremRate", this);
    treeState.getParameter ("tremRate")->setValue (0.0f);
    treeState.addParameterListener("vibrato", this);
    treeState.getParameter ("vibrato")->setValue (true);
    treeState.addParameterListener("tremolo", this);
    treeState.getParameter ("tremolo")->setValue (true);
    treeState.addParameterListener("linked", this);
    treeState.getParameter ("linked")->setValue (true);
    
    vibe = 0.0f;
    vibeRate = 0.0f;
    trem = 0.0f;
    tremRate = 0.0f;
    
    process0 = std::make_unique<soundtouch::SoundTouch>();
    process0->setChannels(1);
    process0->setSampleRate(getSampleRate());
    process0->setPitchSemiTones(2);
    process1 = std::make_unique<soundtouch::SoundTouch>();
    process1->setChannels(1);
    process1->setSampleRate(getSampleRate());
    process1->setPitchSemiTones(2);
}




VibroloAudioProcessor::~VibroloAudioProcessor()
{
}

//==============================================================================
const juce::String VibroloAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool VibroloAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool VibroloAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool VibroloAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double VibroloAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int VibroloAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int VibroloAudioProcessor::getCurrentProgram()
{
    return 0;
}

void VibroloAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String VibroloAudioProcessor::getProgramName (int index)
{
    return {};
}

void VibroloAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout VibroloAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add(std::make_unique<juce::AudioParameterFloat>("vibe", "Vibe", 0.0f, 1.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("trem", "Trem", 0.0f, 1.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("vibeRate", "VibeRate", 0.0f, 1.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("tremRate", "TremRate", 0.0f, 1.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterBool>("vibrato", "Vibrato", true));
    layout.add(std::make_unique<juce::AudioParameterBool>("tremolo", "Tremolo", true));
    layout.add(std::make_unique<juce::AudioParameterBool>("linked", "Linked", true));
    return layout;
}

//==============================================================================
void VibroloAudioProcessor::parameterChanged  (const juce::String &parameterID, float newValue)
{
    if (parameterID.compare("vibe")==0)
    {
        vibe = newValue;
    }
    
    if (parameterID.compare("trem")==0)
    {
        trem = newValue;
    }
    
    if (parameterID.compare("vibRate")==0)
    {
        vibeRate = newValue;
    }

    if (parameterID.compare("tremRate")==0)
    {
        tremRate = newValue;
    }
    
    if (parameterID.compare("vibrato")==0)
    {
        if (vibrato)
            vibrato = false;
        else
            vibrato = true;
    }
    
    if (parameterID.compare("tremolo")==0)
    {
        if (tremolo)
            tremolo = false;
        else
            tremolo = true;
    }
    
    if (parameterID.compare("linked")==0)
    {
        if (linked)
            linked = false;
        else
            linked = true;
    }
}

void VibroloAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{

}

//==============================================================================
void VibroloAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}


//==============================================================================
#ifndef JucePlugin_PreferredChannelConfigurations
bool VibroloAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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
void VibroloAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    auto bufferLength = buffer.getNumSamples();
    
    
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, bufferLength);
    

    if (totalNumInputChannels == 2)
    {
        
        auto* channelLeft = buffer.getWritePointer(0);
        auto* channelRight = buffer.getWritePointer(1);
        
        process0 -> putSamples(channelLeft, bufferLength);
        process1 -> putSamples(channelRight, bufferLength);
        
        process0->receiveSamples(channelLeft , bufferLength);
        process1->receiveSamples(channelRight , bufferLength);
        
        for (int sample = 0; sample < bufferLength; sample++)
        {
            
            channelLeft++;
            channelRight++;
            
        }
    }
    
    
}

//==============================================================================
bool VibroloAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* VibroloAudioProcessor::createEditor()
{
    return new VibroloAudioProcessorEditor (*this);
}

//==============================================================================
void VibroloAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = treeState.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void VibroloAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
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
    return new VibroloAudioProcessor();
}
