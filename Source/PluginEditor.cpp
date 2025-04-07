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
    
    addAndMakeVisible (&runningInfo);
    runningInfo.setText("Stopped", juce::dontSendNotification);
    
    addAndMakeVisible (&timeInfo);
        
    addAndMakeVisible (&secondsLabel);
    secondsLabel.setText("seconds", juce::dontSendNotification);
    
    addAndMakeVisible (&startStop);
    startStop.setButtonText ("START");
    startStop.onClick = [this] { startStopButtonClicked(); };
    
    addAndMakeVisible (&timeInput);
    timeInput.setText ("10");
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
    timeInput.setBounds(50, 50, 50, 30);
    secondsLabel.setBounds(100, 50, 100, 30);
    startStop.setBounds(50,100,100,30);
    runningInfo.setBounds(50, 150, 250, 30);
    timeInfo.setBounds(50, 200, 250, 30);
}

void ReversatronAudioProcessorEditor::startStopButtonClicked()
{
	if (audioProcessor.status == audioProcessor.RECORDING)
	{
		startStop.setButtonText ("START");
		audioProcessor.status = audioProcessor.STOPPED;
		runningInfo.setText("Stopped", juce::dontSendNotification);
		timeInfo.setText("Countdown: (Stopped)", juce::dontSendNotification);
		stopTimer();
	}
	else if (audioProcessor.status == audioProcessor.PLAYBACK)
	{
		startStop.setButtonText ("START");
		audioProcessor.status = audioProcessor.STOPPED;
		runningInfo.setText("Stopped", juce::dontSendNotification);
		timeInfo.setText("Countdown: (Stopped)", juce::dontSendNotification);
		stopTimer();
		
	}
	else if (audioProcessor.status == audioProcessor.STOPPED)
	{
		uint64_t timeInSeconds = timeInput.getText().getIntValue();
		if (timeInSeconds < 1 || timeInSeconds > 500)
			timeInSeconds = 10;
		audioProcessor.setupAudioBuffer(timeInSeconds);
		startStop.setButtonText ("STOP");
		audioProcessor.status = audioProcessor.RECORDING;
		audioProcessor.frame = 0;
		startTimerHz(10);
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
