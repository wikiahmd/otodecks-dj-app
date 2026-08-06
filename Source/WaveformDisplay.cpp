// include the JUCE framework
#include <JuceHeader.h> 

// include the WaveformDisplay header file. 
#include "WaveformDisplay.h" 

// includes math utilities 
#include <cmath> 

// Inheritance and initilization of states as well as construction
WaveformDisplay::WaveformDisplay(juce::AudioFormatManager& formatManagerNeeded, juce::AudioThumbnailCache& cacheNeeded) : audioThumbnail(1000, formatManagerNeeded, cacheNeeded), fileLoaded(false), position(0) 
{
    // registration of this component to recieve thumbnail change events
    audioThumbnail.addChangeListener(this); 
}

// The WaveFormDisplayDestructor 
WaveformDisplay::~WaveformDisplay()
{

}

// Draw the waveform display
void WaveformDisplay::paint(juce::Graphics& design)
{
    // Fill the background with the window color 
    design.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));


    // Set the drawing colour to grey 
    design.setColour(juce::Colours::grey);

    // Draw the border around the component
    design.drawRect(getLocalBounds(), 1);

    // Set the drawimg colour to orange
    design.setColour(juce::Colours::orange); 

    // Checks if an audio file has loaded
    if (fileLoaded)
    {
        // Draw the waveforms for the audio that's been loaded 
        audioThumbnail.drawChannel(design, getLocalBounds(), 0, audioThumbnail.getTotalLength(), 0, 1.0f);

        // Sets the colour for the playhead
        design.setColour(juce::Colours::lightgreen);

        // Converts the relative position to pixel X coordinate
        const int pixel = juce::jlimit(0, getWidth(), juce::roundToInt(position * (double)getWidth()));

        // Calculation of the playhead width with a minimum of 1 pixel
        const int rectW = std::max(1, getWidth() / 20);

        // Draws playback rectangle over the waveform
        design.drawRect(pixel, 0, rectW, getHeight());
    }
    else // Happens when no file is loaded 
    {
        // Sets the font size of the placeholder text
        design.setFont(20.0f);   

        // Center placeholder message is displayed
        design.drawText("File not loaded...", getLocalBounds(), juce::Justification::centred, true);
    }
}

// This function is called when the component size changes
void WaveformDisplay::resized()
{

}

// This loads audio data from the URL
void WaveformDisplay::loadURL(juce::URL audioURL)
{
    // Any previously loaded waveform data is cleared 
    audioThumbnail.clear();

    // A new audio source is loaded and the loaded state is updated
    fileLoaded = audioThumbnail.setSource(new juce::URLInputSource(audioURL));
}

void WaveformDisplay::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    // It triggers the repaint to update the waveform
    repaint();
}

// The playhead position is updated using the relative value
void WaveformDisplay::setPositionRelative(double pos)
{
    // Checks if there is not a number or an infinity value
    if (!std::isfinite(pos))
    {
        // An invalid position is set to 0
        pos = 0.0;
    }

    // The position is clamped between 0 and 1
    pos = juce::jlimit(0.0, 1.0, pos);

    // Checks if the playhead position has changed
    if (pos != position)
    {
        // Stores a new playhead position
        position = pos;

        // A waveform is redrawn with an updated playhead
        repaint();
    }
}