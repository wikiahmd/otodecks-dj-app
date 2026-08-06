// This helps ensure the header file isn't included more than once
#pragma once 

// This is a JUCE header file that contains the classes and functions in JUCE library
#include <JuceHeader.h> 

//==============================================================================

// A class that inherits from the Compoenent and Change Listener Class in JUCE
class WaveformDisplay : public juce::Component, public juce::ChangeListener 
{
// The public interface of the class 
public: 

    // This is a constructor that declares the dependies of a class which are the audio format manager and the audio thumbnail
    WaveformDisplay(juce::AudioFormatManager& formatManagerNeeded, juce::AudioThumbnailCache& cacheNeeded); 

    // Overrides the default destructor
    ~WaveformDisplay() override; 

    // The method used to draw the waveform
    void paint(juce::Graphics&) override; 

    // Resizes the component
    void resized() override; 

    // Load a file from a URL
    void loadURL(juce::URL audioURL); 

    // The audio callback triggers when the thumbnail changes
    void changeListenerCallback(juce::ChangeBroadcaster* source) override; 

    // Set the playhead position as a 0-1 value
    void setPositionRelative(double pos); 

// Begin the private data members
private:  

    // The playhead position which is a value between 0 and 1.
    double position; 

    // This is the audio thumbnail object
    juce::AudioThumbnail audioThumbnail; 

    // Flag to check whether a file has loaded or not.
    bool fileLoaded; 

    // Prevents copying and checks for leaks within the class. This is a MACRO function.
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformDisplay) 
};