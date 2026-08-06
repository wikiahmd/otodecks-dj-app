// The MainComponent Header file
#include "MainComponent.h"

//==============================================================================

// The constructor
MainComponent::MainComponent() : playlistComponent(formatManager)
{
    setSize(800, 600);

    // Adds UI components
    addAndMakeVisible(deck1);
    addAndMakeVisible(deck2);
    addAndMakeVisible(playlistComponent);

    // 0 audio inputs, 2 outputs
    setAudioChannels(0, 2);

    // Registers basic audio formats
    formatManager.registerBasicFormats();

    // loads the playlist library
    playlistComponent.loadLibrary();

    // A user component calls this callback when a track is loaded into the deck
    playlistComponent.onLoadToDeck = [this](juce::File file, int deckIndex)
        {
            // if Deck 1
            if (deckIndex == 1)
            {
                // Load file into deck1
                deck1.loadFile(juce::URL{ file });

                // Restore previously saved Equaliser settings
                applyEqualiserToPlayer(file.getFullPathName(), &player1);
            }
            // if Deck 2
            else if (deckIndex == 2)
            {
                // Load file into deck 2
                deck2.loadFile(juce::URL{ file });

                // Restore the previously saved Equaliser Settings
                applyEqualiserToPlayer(file.getFullPathName(), &player2);
            }
        };

    deck1.setDeckIndex(1);
    deck2.setDeckIndex(2);

    // Set the cue callback for deck 1
    deck1.setCueCallbacks(

        // When deck 1 changes or saves hot cues, store them by their path
        [this](const juce::String& path, int, const std::array<double, 8>& cues)
        {
            storeCuesForTrack(path, cues);
        },
        // when deck 1 want to restore the same cues. load them and apply to the player
        [this](const juce::String& path, int, DJAudioPlayer* player)
        {
            applyCuesToPlayer(path, player);
        }
    );

    // Same cue callbacks for deck 2
    deck2.setCueCallbacks(
        [this](const juce::String& path, int, const std::array<double, 8>& cues)
        {
            storeCuesForTrack(path, cues);
        },
        [this](const juce::String& path, int, DJAudioPlayer* player)
        {
            applyCuesToPlayer(path, player);
        }
    );

    // When deck1 or Equaliser sliders change, store the values 
    deck1.onEQChanged = [this](const juce::String& path, double low, double mid, double high)
        {
            storeEqualiser(path, low, mid, high);
        };

    // Same for deck 2
    deck2.onEQChanged = [this](const juce::String& path, double low, double mid, double high)
        {
            storeEqualiser(path, low, mid, high);
        };
}

// The Destructor
MainComponent::~MainComponent()
{
    // Save playlist to disk before closing'
    playlistComponent.saveLibrary();

    // Stops audio device and disconnects
    shutdownAudio();
}

// Called by JUCE whne audio device starts
void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    // Add both players as inputs to the mixer
    mixerSource.addInputSource(&player1, false);
    mixerSource.addInputSource(&player2, false);

    // Prepare the mixer
    mixerSource.prepareToPlay(samplesPerBlockExpected, sampleRate);

    // This prepares both players too
    player1.prepareToPlay(samplesPerBlockExpected, sampleRate);
    player2.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

// Get the next audio block
void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    // Mixer fills the output buffer
    mixerSource.getNextAudioBlock(bufferToFill);
}

// Called when audio stops
void MainComponent::releaseResources()
{
    // Remove all inputs from the mixer
    mixerSource.removeAllInputs();

    // Release any resources 
    mixerSource.releaseResources();

    // Release resources used by each player
    player1.releaseResources();
    player2.releaseResources();
}

//==============================================================================


// Paint that is used for custom drawing
void MainComponent::paint(juce::Graphics& design)
{
    // Fill background with the default colour
    design.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
    // Top half being decks
    deck1.setBounds(0, 0, getWidth() / 2, getHeight() / 2);
    deck2.setBounds(getWidth() / 2, 0, getWidth() / 2, getHeight() / 2);

    // Bottom half
    playlistComponent.setBounds(0, getHeight() / 2, getWidth(), getHeight() / 2);

    // Debug Message
    DBG("MainComponent::resized");
}

// Returns the file path where you store cues and equalisers
juce::File MainComponent::getCuesFile() const
{
    // Get user app-data directory
    auto dir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getChildFile(ProjectInfo::projectName);

    // Create durectory if missing
    if (!dir.exists())
    {
        dir.createDirectory();
    }

    // Return the final JSON file path
    return dir.getChildFile("cues.json");
}

// Loads cues JSON into a juce::var structure
juce::var MainComponent::loadCuesJson() const
{
    // Get JSON file location
    auto f = getCuesFile();
    
    // if the file doesn't exist, return an empty object 
    if (!f.existsAsFile())
    {
        return juce::var(new juce::DynamicObject());
    }

    // Parse JSON text
    auto parsed = juce::JSON::parse(f.loadFileAsString());

    // Check if the parsing failed, then return an object
    if (parsed.isVoid() || parsed.isUndefined())
    {
        return juce::var(new juce::DynamicObject());
    }

    // If the object is not parsed, return a new juce::DynamicObject
    if (!parsed.isObject())
    {
        return juce::var(new juce::DynamicObject);
    }

    // Return the parsed object
    return parsed;
}

// Saves a juce::var as JSON
void MainComponent::saveCuesJson(const juce::var& v) const
{
    // Convert var -> JSON
    getCuesFile().replaceWithText(juce::JSON::toString(v));

}

// Store 8 hot cues
void MainComponent::storeCuesForTrack(const juce::String& trackPath, const std::array<double, 8>& cues)
{
    // Load current JSON root
    auto root = loadCuesJson();

    // Get DynamicObject pointer
    auto* object = root.getDynamicObject();

    // If root isn't an object, do nothing
    if (!object)
    {
        return;
    }

    // JUCE JSON arrays
    juce::Array<juce::var> cueArray;

    // Copy the array to the JUCE var array 
    for (auto c : cues)
    {
        cueArray.add(c);
    }

    // Store key = trackPath
    object->setProperty(trackPath, juce::var(cueArray));

    // Save JSON back
    saveCuesJson(root);
}

// ===================================================

// Restore cues for a track and apply them
void MainComponent::applyCuesToPlayer(const juce::String& trackPath, DJAudioPlayer* player)
{
    // If player point is null, abort
    if (!player)
    {
        return;
    }

    // Loads JSON root
    auto root = loadCuesJson();
    auto* object = root.getDynamicObject();

    // if it's not a valid object, abort
    if (!object)
    {
        return;
    }

    // Get the property stored
    auto v = object->getProperty(trackPath);

    // if the propert isn't an array, then theres no stored cues
    if (!v.isArray())
    {
        return;
    }

    // Get the pointer to the JUCE var array
    auto* array = v.getArray();

    if (!array)
    {
        return;
    }

    // Create cues array and default all values
    std::array<double, 8> cues;
    cues.fill(-1.0);

    // Copy up to 8 values from JSON array
    for (int i = 0; i < juce::jmin(8, array->size()); ++i)
    {
        cues[(size_t)i] = (double)(*array)[i];
    }

    // Apply cues to the player
    player->setHotCues(cues);
}

// Store Equaliser Values
void MainComponent::storeEqualiser(const juce::String& trackPath, double low, double mid, double high)
{
    // Load current JSON root
    auto root = loadCuesJson();

    // Get Dynamic Object 
    auto* object = root.getDynamicObject();

    // Create a 3 value array for Equaliser
    juce::Array<juce::var> equaliserValues;
    equaliserValues.add(low);
    equaliserValues.add(mid);
    equaliserValues.add(high);

    // Store Equalisers a different key so it doesn't overwrite the cue array
    object->setProperty(trackPath + "_eq", juce::var(equaliserValues));

    // Save JSON back to disk
    saveCuesJson(root);
}

// Load Equaliser values for a track
void MainComponent::applyEqualiserToPlayer(const juce::String& trackPath, DJAudioPlayer* player)
{
    // Load JSON
    auto root = loadCuesJson();

    // Get Dynamic Object
    auto* object = root.getDynamicObject();

    // Check if the Equaliser key exists
    if (object && object->hasProperty(trackPath + "_eq"))
    {
        auto equaliserVariable = object->getProperty(trackPath + "_eq");

        // Ensure it is an array
        if (equaliserVariable.isArray())
        {
            // Get array pointer 
            auto* array = equaliserVariable.getArray();

            // Apply to player
            player->setEQ((double)(*array)[0], (double)(*array)[1], (double)(*array)[2]);
        }
    }
}
