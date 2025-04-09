/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ReversatronAudioProcessorEditor::ReversatronAudioProcessorEditor (ReversatronAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
    
    bufferLengthSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.getApvts(), "bufferLength", bufferLengthSlider);
    crossfadeTimeSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.getApvts(), "crossfadeTime", crossfadeTimeSlider);
    
    addAndMakeVisible (&bufferLengthLabel);
    bufferLengthLabel.setText("Buffer Length (s)", juce::dontSendNotification);
    addAndMakeVisible (&crossfadeTimeLabel);
    crossfadeTimeLabel.setText("Crossfade (s)", juce::dontSendNotification);
    
    addAndMakeVisible (&bufferLengthSlider);
    bufferLengthSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    bufferLengthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 30);
    bufferLengthSlider.setRange(0.5f, 500.0f, 0.01f);
    
    addAndMakeVisible (&crossfadeTimeSlider);
    crossfadeTimeSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    crossfadeTimeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 30);
    crossfadeTimeSlider.setRange(0.0f, 250.0f, 0.01f);
    
    addAndMakeVisible (&runningInfo);
    runningInfo.setText("Stopped", juce::dontSendNotification);
    
    addAndMakeVisible (&timeInfo);
    
    addAndMakeVisible (&startStop);
    startStop.setButtonText ("START");
    startStop.onClick = [this] { startStopButtonClicked(); };
}

ReversatronAudioProcessorEditor::~ReversatronAudioProcessorEditor()
{
	
}

//==============================================================================
void ReversatronAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
}

void ReversatronAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    bufferLengthLabel.setBounds(50, 10, 100, 20);
    crossfadeTimeLabel.setBounds(200, 10, 100, 20);
    bufferLengthSlider.setBounds(50, 30, 100, 100);
    crossfadeTimeSlider.setBounds(200, 30, 100, 100);
    startStop.setBounds(50, 150, 100,30);
    runningInfo.setBounds(50, 200, 250, 20);
    timeInfo.setBounds(50, 250, 250, 20);
}

void ReversatronAudioProcessorEditor::startStopButtonClicked()
{
	if (audioProcessor.status == audioProcessor.RECORDING || audioProcessor.status == audioProcessor.PLAYBACK)
	{
		startStop.setButtonText ("START");
		audioProcessor.status = audioProcessor.STOPPED;
		runningInfo.setText("Stopped", juce::dontSendNotification);
		timeInfo.setText("Countdown: (Stopped)", juce::dontSendNotification);
		stopTimer();
		bufferLengthSlider.setEnabled(true);
		crossfadeTimeSlider.setEnabled(true);
	}
	else if (audioProcessor.status == audioProcessor.STOPPED)
	{
		float timeInSeconds = *audioProcessor.getApvts().getRawParameterValue("bufferLength");
		if (timeInSeconds < 0.5f)
			timeInSeconds = 0.5f;
		else if (timeInSeconds > 500.0f)
			timeInSeconds = 500.0f;
		
		float crossfade = *audioProcessor.getApvts().getRawParameterValue("crossfadeTime");
		if (crossfade < 0.0f)
			crossfade = 0.0f;
		else if (crossfade > 250.0f)
			crossfade = 250.0f;
		
		audioProcessor.crossfadeTime = crossfade;
		audioProcessor.setupAudioBuffer(timeInSeconds);
		startStop.setButtonText ("STOP");
		audioProcessor.status = audioProcessor.RECORDING;
		audioProcessor.frame = 0;
		startTimerHz(10);
		
		bufferLengthSlider.setEnabled(false);
		crossfadeTimeSlider.setEnabled(false);
	}
	
}

void ReversatronAudioProcessorEditor::timerCallback()
{
	if (audioProcessor.status == audioProcessor.RECORDING)
	{
		runningInfo.setText("Now recording", juce::dontSendNotification);
		timeInfo.setText("Countdown: " + juce::String(static_cast<int>(audioProcessor.seconds - (audioProcessor.frame / audioProcessor.sampleRate))), juce::dontSendNotification);
	}
	else if (audioProcessor.status == audioProcessor.PLAYBACK)
	{
		runningInfo.setText("Now playing back reversed", juce::dontSendNotification);
		timeInfo.setText("Countdown: " + juce::String(static_cast<int>(audioProcessor.seconds - (audioProcessor.frame / audioProcessor.sampleRate))), juce::dontSendNotification);
	}
	else if (audioProcessor.status == audioProcessor.STOPPED)
	{
		runningInfo.setText("Stopped", juce::dontSendNotification);
		timeInfo.setText("Countdown: (Stopped)", juce::dontSendNotification);
	}
}
