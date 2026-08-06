// include the JUCE framework
#include <JuceHeader.h>

// Includes the PlaylistComponent header file
#include "PlaylistComponent.h" 

//==============================================================================

// Construction of the component and storage of the reference to the audio format manager
PlaylistComponent::PlaylistComponent(juce::AudioFormatManager& _formatManager) : formatManager(_formatManager)
{
	// Adds the import button and makes it visbile
	addAndMakeVisible(importButton);

	// This class is registered to recieve click events from the import button
	importButton.addListener(this);

	// Adds the table component to the UI
	addAndMakeVisible(tableComponent);

	// Header flags for resizable and visble columns
	auto titleFlags = juce::TableHeaderComponent::visible | juce::TableHeaderComponent::resizable;

	// Header flags for button columns
	auto buttonFlags = juce::TableHeaderComponent::visible;

	// The table components
	tableComponent.getHeader().addColumn("Track Title", 1, 200, 100, -1, titleFlags);
	tableComponent.getHeader().addColumn("Duration", 2, 80, 50, 100, buttonFlags);
	tableComponent.getHeader().addColumn("Deck 1", 3, 70, 50, 100, buttonFlags);
	tableComponent.getHeader().addColumn("Deck 2", 4, 70, 50, 100, buttonFlags);

	tableComponent.getHeader().setStretchToFitActive(true);
	tableComponent.setModel(this);
}

// The Destructor that is used for clean up
PlaylistComponent::~PlaylistComponent()
{

}

// Paints the component's background
void PlaylistComponent::paint(juce::Graphics& design)
{
	// Fills the background with the window colour
	design.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

	// The drawing colour is set to grey
	design.setColour(juce::Colours::grey);

	// Sets the font size
	design.setFont(juce::FontOptions(14.0f));

	// The placeholder componet name text
	design.drawText("PlaylistComponent", getLocalBounds(), juce::Justification::centred, true);
}

void PlaylistComponent::resized()
{
	// Positions the import button at the top
	importButton.setBounds(0, 0, getWidth(), 30);

	// Positions the table below the button 
	tableComponent.setBounds(0, 30, getWidth(), getHeight() - 30);
}

int PlaylistComponent::getNumRows()
{
	// Returns the amount of tracks that are in the playlist 
	return tracks.size();
}

// Paint the text content of an individual cell
void PlaylistComponent::paintCell(juce::Graphics& design, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
{
	// Checks if the row index is valid
	if (rowNumber < tracks.size())
	{
		// Checks if the current column is the Track Title column
		if (columnId == 1)
		{
			// Draws the track file name in the cell
			design.drawText(tracks[rowNumber].fileName, 2, 0, width - 4, height, juce::Justification::centredLeft, true);
		}
		// Checks if the current column is the DUration column
		else if (columnId == 2)
		{
			// Draws the track's duration text in the cell
			design.drawText(tracks[rowNumber].duration, 2, 0, width - 4, height, juce::Justification::centredLeft, true);
		}
	}
}

// Paints the background behind each row
void PlaylistComponent::paintRowBackground(juce::Graphics& design, int rowNumber, int width, int height, bool rowIsSelected)
{
	// Checks if the row is selected
	if (rowIsSelected)
	{
		// Fills the selected row background with orange
		design.fillAll(juce::Colours::orange);
	}
	// Handles the non-selcetd rows
	else 
	{
		// Fills the non-selected row background with dark grey
		design.fillAll(juce::Colours::darkgrey);
	}
}

// Creates or updates the custom component inside a table cell
juce::Component* PlaylistComponent::refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, Component* existingComponentToUpdate)
{
	// Checks if the cell is in one of the deck button columns
	if (columnId == 3 || columnId == 4)
	{
		// Reuse the existing component as a text button
		juce::TextButton* btn = dynamic_cast<juce::TextButton*>(existingComponentToUpdate);

		// if no reusable button exists
		if (btn == nullptr)
		{
			// Allocates a new button labeled "Load"
			btn = new juce::TextButton("Load");

			// Registers this class to recieve click events
			btn->addListener(this);

			// Updates the returned component pointer 
			existingComponentToUpdate = btn;
		}

		// A unique ID string encoding row and column
		juce::String id{ std::to_string(rowNumber) + "_" + std::to_string(columnId) };

		// Stores the ID on the button
		btn->setComponentID(id);

		// Sets the button bounds with padding
		btn->setBounds(2, 2, tableComponent.getHeader().getColumnWidth(columnId) - 4, tableComponent.getRowHeight() - 4);
	}

	// Returns the component JUCE should place in this cell
	return existingComponentToUpdate;
}

// Handles click events 
void PlaylistComponent::buttonClicked(juce::Button* button)
{
	// Checks if the clicked button is the import button
	if (button == &importButton)
	{
		// Sets file chooser to allow selecting files 
		auto fileChooserFlags = juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::canSelectMultipleItems;

		// Opens the file chooser asynchronously and runs a callback
		fChooser.launchAsync(fileChooserFlags, [this](const juce::FileChooser& chooser)
			{
				// Gets the list of files the user had selected 
				auto results = chooser.getResults();

				// Loops through each selected file
				for (auto& file : results)
				{
					// Creates an audio reader 
					std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));

					// Checks if the reader was succesfully created
					if (reader != nullptr)
					{

						double length = reader->lengthInSamples / reader->sampleRate;

						// Creates a Track struct with file properties
						Track newTrack{ file, file.getFileName(), secondToMinutes(length), length };

						// Adds the new track to the playlist vector
						tracks.push_back(newTrack);
					}
				}

				// Refreshes the table
				tableComponent.updateContent();

				// Saves the updated playlist
				saveLibrary();
			});
	}
	// Handles clicks from dynamically created "Load" buttons
	else
	{
		// Reads the button's ID string
		juce::String id = button->getComponentID(); 

		// Finds the underscore seperating row and column
		int underScorePos = id.indexOf("_");

		// Checks that the underscore was found
		if (underScorePos > -1)
		{
			// Parses the row index
			int row = id.substring(0, underScorePos).getIntValue();

			// Parses the column ID after the understore to the end
			int col = id.substring(underScorePos + 1).getIntValue();

			// Validates that the row index is within the vector bounds
			if (row >= 0 && row < tracks.size())
			{
				// Converts column ID 3 into a deck number which is either 1 or 2
				int deckIndex = (col == 3) ? 1 : 2;

				// Checks that a load callback has been assigned
				if (onLoadToDeck)
				{
					// Calls the callback with the chosen file and the target deck
					onLoadToDeck(tracks[row].file, deckIndex);
				}
			}
		}
	}
}

// Converts a duration into seconds
juce::String PlaylistComponent::secondToMinutes(double seconds)
{
	// Casts seconds to an integer
	int secondsInt = (int)seconds;

	// Computes whole minutes from the total seconds
	int minutes = secondsInt / 60;

	// Computes leftover seconds 
	int remainingSeconds = secondsInt % 60; 

	// Returns zero-padded mm:ss string
	return juce::String(minutes) + ":" + ((remainingSeconds < 10) ? "0" : "") + juce::String(remainingSeconds);
}

// Returns the file path where the playlist library JSON is
juce::File PlaylistComponent::getLibraryFile() const
{
	// Builds an app specific deirectory path
	auto appDataDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getChildFile(ProjectInfo::projectName);

	// Checks whether the application data directory exists 
	if (!appDataDir.exists())
	{
		// Creates the directory if its shown that it doesnt exist
		appDataDir.createDirectory();
	}

	// Returns an aobject pointing to the library.json file
	return appDataDir.getChildFile("library.json");
}

// Saves the current tracks vector into a JSOn file
void PlaylistComponent::saveLibrary() const
{
	// Array of var objects
	juce::Array<juce::var> tracksList;

	// Iterates over each track in the playlist
	for (const auto& track : tracks)
	{
		// Allocates a JSON-like dynamic object
		auto* object = new juce::DynamicObject();

		// Creates objects out of the file properties
		object->setProperty("path", track.file.getFullPathName());
		object->setProperty("fileName", track.fileName);
		object->setProperty("lengthInSeconds", track.lengthInSeconds);
		object->setProperty("duration", track.duration);

		// Adds that object to the tracksList
		tracksList.add(object);
	}

	// Wraps the array into a root var value
	juce::var root(tracksList);

	// Serializes the var structure into a JSON string
	auto libraryJson = juce::JSON::toString(root);

	// Gets the output JSON file path
	auto libraryFile = getLibraryFile();

	// Writes the JSON string to the file 
	libraryFile.replaceWithText(libraryJson);
}

// Loads the tracks from the library JSON file
void PlaylistComponent::loadLibrary()
{
	// Clears the current tracks 
	tracks.clear();

	// Gets the file path
	auto libraryFile = getLibraryFile();

	// Checks if the library file exists
	if (!libraryFile.existsAsFile())
	{
		// Refreshes the table 
		tableComponent.updateContent();

		// Exits early
		return;
	}

	// Loads the entire JSON file into string
	auto text = libraryFile.loadFileAsString();

	// Parses the string into a JSON var structure
	auto parsed = juce::JSON::parse(text);

	// Checks that the parsed JSON is an array
	if (!parsed.isArray())
	{
		// Refreshes the table 
		tableComponent.updateContent();

		// Exits early 
		return;
	}

	// Gets a pointer to the underlying array
	auto* array = parsed.getArray();

	// Iterates through each item in the JSON list
	for (const auto& item : *array)
	{
		// Checks that the object is a JSON object
		if (auto* object = item.getDynamicObject())
		{
			// reads the stored file path from the JSON object
			const auto path = object->getProperty("path").toString();

			// Creates a JUCE file from that path
			juce::File file(path);

			// Skips the entries of files that dont exist on disk
			if (!file.existsAsFile())
			{
				// Continues to the next item without adding the track
				continue;
			}

			// Empty track created that is populated with JSON
			Track track1;

			// File reference stored in that track
			track1.file = file;

			// Reads the saved filename from that JSON
			track1.fileName = object->getProperty("fileName").toString();

			// Reads the saved numeric length from JSON
			track1.lengthInSeconds = object->getProperty("lengthInSeconds");

			// Reads the saved formatted duration string from JSON
			track1.duration = object->getProperty("duration").toString();

			// Checks if the stored filename is missing
			if (track1.fileName.isEmpty())
			{
				// Uses the actual file name from disk
				track1.fileName = file.getFileName();
			}

			// Checks if the stored duration string is missing
			if (track1.duration.isEmpty())
			{
				// Checks the duration string 
				track1.duration = secondToMinutes(track1.lengthInSeconds);
			}

			// Adds the track to the playlist vector
			tracks.push_back(track1);
		}
	}

	// Updates the table
	tableComponent.updateContent();
}