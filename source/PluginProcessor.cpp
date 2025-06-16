#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PluginProcessor::PluginProcessor()
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    // Optionally, you can add activation state callbacks here:
    // moonbaseClient->addActivationStateChangedCallback([](Moonbase::JUCEClient::ActivationState state)
    // {
    //     // React to activation state changes if needed.
    // });

    apvts.state.setProperty(Service::PresetManager::presetNameProperty, "", nullptr);
    presetManager = std::make_unique<Service::PresetManager>(apvts);
}

PluginProcessor::~PluginProcessor()
{
}

//==============================================================================
const juce::String PluginProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PluginProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PluginProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PluginProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PluginProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PluginProcessor::getNumPrograms()
{
    return 1; // At least 1 program should be provided.
}

int PluginProcessor::getCurrentProgram()
{
    return 0;
}

void PluginProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String PluginProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void PluginProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void PluginProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Prepare the DSP processor
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<uint32>(samplesPerBlock);
    spec.numChannels = static_cast<uint32>(getTotalNumOutputChannels());
    dspProcessor.prepare(spec);
}

void PluginProcessor::releaseResources()
{
    // Reset the DSP processor
    dspProcessor.reset();
}

bool PluginProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
   #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
   #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
        layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif
    return true;
   #endif
}

void PluginProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                    juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);
    juce::ScopedNoDenormals noDenormals;
    
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear any output channels that don't have input data.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Check if bypassed
    bool isBypassed = *apvts.getRawParameterValue("BYPASS");
    if (isBypassed)
        return;    // Get parameter values and update DSP processor
    float inputGain = *apvts.getRawParameterValue("INPUT_GAIN");
    float outputGain = *apvts.getRawParameterValue("OUTPUT_GAIN");
    float mix = *apvts.getRawParameterValue("MIX");
    float delay = *apvts.getRawParameterValue("DELAY");
    float brightness = *apvts.getRawParameterValue("BRIGHTNESS");
    float character = *apvts.getRawParameterValue("CHARACTER");
    float lowCut = *apvts.getRawParameterValue("LOW_CUT");
    float highCut = *apvts.getRawParameterValue("HIGH_CUT");
    float width = *apvts.getRawParameterValue("WIDTH");
    bool limiterEnabled = *apvts.getRawParameterValue("LIMITER") > 0.5f;

    // Update DSP processor parameters
    // dspProcessor.setInputGain(inputGain);
    // dspProcessor.setOutputGain(outputGain);
    // dspProcessor.setMix(mix);
    // dspProcessor.setDelay(delay);
    // dspProcessor.setBrightness(brightness);
    // dspProcessor.setCharacter(character);
    // dspProcessor.setLowCut(lowCut);
    // dspProcessor.setHighCut(highCut);
    // dspProcessor.setWidth(width);
    // dspProcessor.setLimiterEnabled(limiterEnabled);
    dspProcessor.updateParameters(inputGain, outputGain, mix, delay, brightness,
                                  character, lowCut, highCut, width, limiterEnabled);

    // Process the audio using function from
    dspProcessor.processBlock(buffer);
}

//==============================================================================
bool PluginProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* PluginProcessor::createEditor()
{
    return new PluginEditor (*this);
}

//==============================================================================
void PluginProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void PluginProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginProcessor();
}
