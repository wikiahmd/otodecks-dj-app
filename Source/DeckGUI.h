// Prevents this headeer from being included more than once
#pragma once

// Include the JUCE Header
#include <JuceHeader.h>

// Audio playback engine
#include "DJAudioPlayer.h"

// Draws the Waveform
#include "WaveformDisplay.h"

//==============================================================================

// Represents one deck
class DeckGUI : public juce::Component, public juce::Button::Listener, public juce::Slider::Listener, public juce::FileDragAndDropTarget, public juce::Timer
{
public:
	
	// Constructor
	DeckGUI(DJAudioPlayer* player,
		juce::AudioFormatManager& formatManagerToUse,
		juce::AudioThumbnailCache& cacheToUse);

	// Destructor
	~DeckGUI();

	// Paint method 
	void paint(juce::Graphics&) override;

	// Called when the component is resized
	void resized() override;

	// Called when any button in the component is clicked
	void buttonClicked(juce::Button*) override;

	// Called when a slider value is changed
	void sliderValueChanged(juce::Slider* slider) override;

	// Checks if dragged files are accepted
	bool isInterestedInFileDrag(const juce::StringArray& files) override;

	// Called when files are dropped into the component
	void filesDropped(const juce::StringArray& files, int x, int y) override;

	// Timer callback
	void timerCallback() override;

	// Loads the file into the deck
	void loadFile(juce::URL audioURL);

	// Sets deck index so callbacks know which deck it is
	void setDeckIndex(int index);

	// Set callbacks for cue handling
	void setCueCallbacks(
		std::function<void(const juce::String&, int, const std::array<double, 8>&)> cuesChanged,
		std::function<void(const juce::String&, int, DJAudioPlayer*)> trackLoaded
	);

	// Callback triggered when equaliser sliders change
	std::function<void(const juce::String&, double, double, double)> onEQChanged;

private: 

	// Play track
	juce::TextButton playButton{ "PLAY" };

	// Stop track
	juce::TextButton stopButton{ "STOP" };
	
	// Open file chooser
	juce::TextButton loadButton{ "LOAD" };

	// Clear all hot cues
	juce::TextButton clearCuesButton{ "Clear Cues" };

	// Array of 8 cues
	std::array<juce::TextButton, 8> hotCueButtons
	{
		juce::TextButton{ "C1" },
		juce::TextButton{ "C2" },
		juce::TextButton{ "C3" },
		juce::TextButton{ "C4" },
		juce::TextButton{ "C5" },
		juce::TextButton{ "C6" },
		juce::TextButton{ "C7" },
		juce::TextButton{ "C8" },
	};

	// Path of the current track thats loaded
	juce::String loadedTrackPath;
	
	// Callback triggered when cues changed
	std::function<void(const juce::String& trackPath, int deckIndex, const std::array<double, 8>& cues)> onCuesChanged;

	// Callback triggered when the track is loaded
	std::function<void(const juce::String& trackPath, int deckIndex, DJAudioPlayer* player)> onTrackLoaded;

	// Callback triggered when a track is loaded
	int deckIndex = 1;

	// Volume slider
	juce::Slider volSlider;
	
	// Playback speed
	juce::Slider speedSlider;
	
	// 3 Band EQ sliders
	juce::Slider lowSlider;
	juce::Slider midSlider;
	juce::Slider highSlider;

	// Displays detected BPM value
	juce::Label bpmLabel;

	// Current BPM 
	double currentBPM = 120.0;

	// Track position 
	juce::Slider posSlider;

	// File chooser
	juce::FileChooser fChooser{ "Select a file..." };

	// Points to the audio engine
	DJAudioPlayer* player;

	// Visual waveform display 
	WaveformDisplay waveformDisplay;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DeckGUI)

};
