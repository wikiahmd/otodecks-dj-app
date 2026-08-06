// Prevents the file form being included multiple times
#pragma once 

// The JUCE framework
#include <JuceHeader.h>

// DJ Audio Player Header
#include "DJAudioPlayer.h"

// The GUI for each Deck
#include "DeckGUI.h"

// The Playlist Component header file
#include "PlaylistComponent.h"

//==============================================================================

// Handles callbackks and GUI
class MainComponent : public juce::AudioAppComponent
{
public:

	// The Constructor 
	MainComponent();

	// The Destructor
	~MainComponent() override;

	// Called before the beginning of the audio
	void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;

	// Called to fill audio buffers
	void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;

	// Called when the audio stops
	void releaseResources() override;

	// Draw the Graphics on the screen
	void paint(juce::Graphics& design) override;

	// Reposition's UI Components 
	void resized() override;

	// Save Equaliser settings
	void storeEqualiser(const juce::String& trackPath, double lowGain, double midGain, double highGain);

private:

	// Manages audio formats
	juce::AudioFormatManager formatManager;

	// Cache used for thumbnails
	juce::AudioThumbnailCache thumbCache{ 100 };

	// Returns the JSON files used to store cue points
	juce::File getCuesFile() const;

	//  Loads the cue data from the JSON file
	juce::var loadCuesJson() const;

	// Saves the cue data
	void saveCuesJson(const juce::var& v) const;

	// Applies Cues to Player
	void applyCuesToPlayer(const juce::String& trackPath, DJAudioPlayer* player);

	// Stores the cues for a track
	void storeCuesForTrack(const juce::String& trackPath, const std::array<double, 8>& cues);

	// Applies saved Equaliser values 
	void applyEqualiserToPlayer(const juce::String& trackPath, DJAudioPlayer* player);

	// The first audio player
	DJAudioPlayer player1{ formatManager };

	// The first deck
	DeckGUI deck1{ &player1, formatManager, thumbCache };

	// The second audio player
	DJAudioPlayer player2{ formatManager };

	// The seocnd deck
	DeckGUI deck2{ &player2, formatManager, thumbCache };

	// Playlist component
	PlaylistComponent playlistComponent{ formatManager };

	// Mixes audio players
	juce::MixerAudioSource mixerSource;

	// Prevents copying and leak detection
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};