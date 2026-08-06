// This helps ensure the header file isn't included more than once
#pragma once 

// The main JUCE header that conatins all the JUCE classes and functions
#include <JuceHeader.h> 

// Provides the std::vector container
#include <vector> 

// Incldues the std::string type
#include <string>

//==============================================================================

// A UI component that acts like a table model and a listener
class PlaylistComponent : public juce::Component, public juce::TableListBoxModel, public juce::Button::Listener
{
public:

	// A constructor that takes the audio format manager as a reference
	PlaylistComponent(juce::AudioFormatManager& formatManager);

	// Declaration of the destructor
	~PlaylistComponent() override;

	// The method that draws the component is declared
	void paint(juce::Graphics&) override;

	// The method that rezies the component is declared
	void resized() override;

	// The number of rows is returned by this method
	int getNumRows();

	// The background of a table row is painted
	void paintRowBackground(juce::Graphics&, int rowNumber, int width, int height, bool rowIsSelected) override;

	// An individual table cell is painted
	void paintCell(juce::Graphics&, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;

	// Method which handles a click on the table cell
	Component* refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, Component* existingComponentToUpdate) override;

	// When the listened button is clicked, this method is executed
	void buttonClicked(juce::Button* button) override;

	// A callback is stored to load a selected file into a certain deck
	std::function<void(juce::File, int)> onLoadToDeck;

	// Metadata for one track
	struct Track {
		juce::File file;
		juce::String fileName;
		juce::String duration;
		double lengthInSeconds;
	};

	// A helper that converts seconds into a mm::ss style string
	static juce::String secondToMinutes(double seconds);

	// Get the library file
	juce::File getLibraryFile() const;

	// Loads the playlist library 
	void loadLibrary();

	// Saves the playlist library without object modification
	void saveLibrary() const;

private:

	// Import a track button
	juce::TextButton importButton{ "Import Track" };

	// A UI component that displays track rows
	juce::TableListBox tableComponent;

	// Stores the list of tracks
	std::vector<Track> tracks;

	// Stores a reference to the audio format manager
	juce::AudioFormatManager& formatManager;

	// A file chooser dialog
	juce::FileChooser fChooser{ "Select files..." };

	// Disables copying and enables JUCE leak detection 
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlaylistComponent)
};