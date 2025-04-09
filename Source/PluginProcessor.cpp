/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
//#include <iostream>

//==============================================================================
ReversatronAudioProcessor::ReversatronAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
                       apvts(*this, nullptr, "Parameters", addParameters())
#endif
{
	apvts.state = juce::ValueTree("Parameters");
	seconds = 10.0f;
	status = STOPPED;
	frame = 0;
	sampleRate = 44100.0;
}

ReversatronAudioProcessor::~ReversatronAudioProcessor()
{
}

//==============================================================================
const juce::String ReversatronAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ReversatronAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ReversatronAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ReversatronAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ReversatronAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ReversatronAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ReversatronAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ReversatronAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ReversatronAudioProcessor::getProgramName (int index)
{
    return {};
}

void ReversatronAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void ReversatronAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    reversatronBuffer.setSize(getTotalNumInputChannels(), static_cast<int> (getSampleRate() * seconds));
    sampleRate = getSampleRate();
    //std::cout << "Buffer Channels: " << std::to_string(reversatronBuffer.getNumChannels()) << std::endl;
	//std::cout << "Buffer Size: " << std::to_string(reversatronBuffer.getNumSamples()) << std::endl;
	//std::cout << "Sample Rate: " << std::to_string(sampleRate) << std::endl;
}

void ReversatronAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ReversatronAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
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

void ReversatronAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    
    
    uint64_t numSamples = buffer.getNumSamples();
    uint64_t initialFrame = frame;
    
    float crossfade = crossfadeTime;
    double currentSampleRate = getSampleRate();
    if ( ( crossfade * currentSampleRate ) > (reversatronBuffer.getNumSamples() / 2))
    {
        crossfade = (reversatronBuffer.getNumSamples() / 2) / currentSampleRate;
    }
    
    if (status == RECORDING)
    {
		for (int channel = 0; channel < totalNumInputChannels; ++channel)
		{
			auto* channelData = buffer.getReadPointer (channel);
			
			frame = initialFrame;

			for (int sample = 0; sample < numSamples ; ++sample)
			{
				if (frame < reversatronBuffer.getNumSamples())
				{
					reversatronBuffer.setSample(channel, frame, buffer.getSample(channel, sample));
					frame++;
				}
			}
		}
	}
	
	else if (status == PLAYBACK)
	{
		for (int channel = 0; channel < totalNumInputChannels; ++channel)
		{
			auto* channelData = buffer.getWritePointer (channel);
			
			frame = initialFrame;

			for (int sample = 0; sample < numSamples ; ++sample)
			{
				if (frame < reversatronBuffer.getNumSamples())
				{
                    if ( frame < (crossfade * currentSampleRate) )
                    {
						// We're within the fade time from the start of playback
						// Fade out buffer, fade in reservatron buffer
                        float gain = frame / (crossfade * currentSampleRate);
                        float bufferSample = buffer.getSample(channel, sample) * (1.0f - gain);
                        float reversatronBufferSample = reversatronBuffer.getSample(channel, reversatronBuffer.getNumSamples() - frame - 1) * gain;
                        buffer.setSample(channel, sample, bufferSample + reversatronBufferSample);
                    }
                    else if (frame > (reversatronBuffer.getNumSamples() - (crossfade * currentSampleRate)))
                    {
						// We're within the fade time towards the end of playback
						// Fade in buffer, fade out reservatron buffer
                        float gain = (frame - (reversatronBuffer.getNumSamples() - (crossfade * currentSampleRate))) / (crossfade * currentSampleRate);
                        float bufferSample = buffer.getSample(channel, sample) * gain;
                        float reversatronBufferSample = reversatronBuffer.getSample(channel, reversatronBuffer.getNumSamples() - frame - 1) * (1.0f - gain);
                        buffer.setSample(channel, sample, bufferSample + reversatronBufferSample);
                    }
					else
                    {   
                        buffer.setSample(channel, sample, reversatronBuffer.getSample(channel, reversatronBuffer.getNumSamples() - frame - 1));
                    }
					frame++;
				}
			}
		}
	}
	
	if (frame >= reversatronBuffer.getNumSamples())
	{
		frame = 0;
		if (status == PLAYBACK)
		{
			status = RECORDING;
		}
		else if (status == RECORDING)
		{
			status = PLAYBACK;
		}
	}
}

//==============================================================================
bool ReversatronAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ReversatronAudioProcessor::createEditor()
{
    return new ReversatronAudioProcessorEditor (*this);
}

//==============================================================================
void ReversatronAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary(*xml, destData);
}

void ReversatronAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout ReversatronAudioProcessor::addParameters()
{
    juce::AudioProcessorValueTreeState::ParameterLayout paramLayout;
    
    paramLayout.add(std::make_unique<juce::AudioParameterFloat>("bufferLength", "Buffer Length", 0.5f, 500.0f, 10.0f));
    paramLayout.add(std::make_unique<juce::AudioParameterFloat>("crossfadeTime", "Crossfade Time", 0.0f, 250.0f, 2.0f));
    
    return paramLayout;
}

AudioProcessorValueTreeState& ReversatronAudioProcessor::getApvts()
{
    return apvts;
}


void ReversatronAudioProcessor::setupAudioBuffer(float timeInSeconds)
{
	seconds = timeInSeconds;
	sampleRate = getSampleRate();
	reversatronBuffer.setSize(getTotalNumInputChannels(), static_cast<int> (getSampleRate() * seconds));
    //std::cout << "Buffer Channels: " << std::to_string(reversatronBuffer.getNumChannels()) << std::endl;
	//std::cout << "Buffer Size: " << std::to_string(reversatronBuffer.getNumSamples()) << std::endl;
	//std::cout << "Sample Rate: " << std::to_string(sampleRate) << std::endl;
	
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ReversatronAudioProcessor();
}
