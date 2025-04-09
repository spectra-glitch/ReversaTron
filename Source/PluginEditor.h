/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class ReversatronAudioProcessorEditor  : public juce::AudioProcessorEditor
													, juce::Timer
{
public:
    ReversatronAudioProcessorEditor (ReversatronAudioProcessor&);
    ~ReversatronAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void startTimer();
    void timerCallback() override;
    void startStopButtonClicked();

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    ReversatronAudioProcessor& audioProcessor;
    
    juce::Slider bufferLengthSlider;
    juce::Slider crossfadeTimeSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bufferLengthSliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> crossfadeTimeSliderAttachment;
    
    juce::Label bufferLengthLabel;
    juce::Label crossfadeTimeLabel;
    juce::TextButton startStop;
    juce::Label runningInfo;
    juce::Label timeInfo;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ReversatronAudioProcessorEditor)
};
