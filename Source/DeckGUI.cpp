// include the JuceHeader file
#include <JuceHeader.h>

// Include the DeckGUI header file
#include "DeckGUI.h"

//==============================================================================
// Constructor
DeckGUI::DeckGUI(DJAudioPlayer* _player, juce::AudioFormatManager& formatManagerToUse, juce::AudioThumbnailCache& cacheToUse) : player(_player), waveformDisplay(formatManagerToUse, cacheToUse)
{
   // Add UI elements to the component
    addAndMakeVisible(playButton);
    addAndMakeVisible(stopButton);
    addAndMakeVisible(loadButton);
    addAndMakeVisible(volSlider);
    addAndMakeVisible(speedSlider);
    addAndMakeVisible(posSlider);
    addAndMakeVisible(waveformDisplay);
    addAndMakeVisible(clearCuesButton);

    // Buttons and Volume sliders
    playButton.addListener(this);
    stopButton.addListener(this);
    loadButton.addListener(this);
    volSlider.addListener(this);
    speedSlider.addListener(this);
    posSlider.addListener(this);
    clearCuesButton.addListener(this);

    // Add and make visible the equaliser sliders and the BPM label
    addAndMakeVisible(lowSlider);
    addAndMakeVisible(midSlider);
    addAndMakeVisible(highSlider);
    addAndMakeVisible(bpmLabel);

    // Add listeners to the sliders
    lowSlider.addListener(this);
    midSlider.addListener(this);
    highSlider.addListener(this);

    // Set initial text
    bpmLabel.setText("BPM: --", juce::dontSendNotification);

    // Make the Hot Cue buttons visible
    for (auto& btn : hotCueButtons)
    {
        addAndMakeVisible(btn);
        btn.addListener(this);
    }

    // Volume 0 to 1
    volSlider.setRange(0.0, 1.0);

    // Speed Range as well as default speed
    speedSlider.setRange(0.5, 2.0, 0.01);
    speedSlider.setValue(1.0);

    // Relative position 0..1
    posSlider.setRange(0.0, 1.0);

    // Equaliser sliders
    lowSlider.setRange(0.0, 2.0, 0.01);
    midSlider.setRange(0.0, 2.0, 0.01);
    highSlider.setRange(0.0, 2.0, 0.01);

    // Default Equaliser values = 1.0
    lowSlider.setValue(1.0);
    midSlider.setValue(1.0);
    highSlider.setValue(1.0);

    // Start a timer that calls timerCallback() every 200ms
    startTimer(200);
}

// Destructor
DeckGUI::~DeckGUI()
{
    // Stop timer
    stopTimer();
}

// Paint draws the background
void DeckGUI::paint(juce::Graphics& design)
{
    // Fill the component background
    design.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    design.drawRect(getLocalBounds(), 1);

    // Set drawing colour and font
    design.setColour(juce::Colours::white);
    design.setFont(14.0f);

    // Draw label text in the centre
    design.drawText("DeckGUI", getLocalBounds(), juce::Justification::centred, true);
}

// Resize the UI layout
void DeckGUI::resized()
{
    // Compute a row height and 4 equal columns
    double rowH = getHeight() / 13.0;
    double colW = getWidth() / 4.0;

    // First row
    playButton.setBounds(0, 0, getWidth() / 3, rowH);
    stopButton.setBounds(getWidth() / 3, 0, getWidth() / 3, rowH);
    loadButton.setBounds(2 * getWidth() / 3, 0, getWidth() / 3, rowH);

    // Next row
    volSlider.setBounds(0, rowH, getWidth(), rowH);
    speedSlider.setBounds(0, rowH * 2, getWidth(), rowH);
    posSlider.setBounds(0, rowH * 3, getWidth(), rowH);

    // Wave form holds 2 rows
    waveformDisplay.setBounds(0, rowH * 4, getWidth(), rowH * 2);

    // Hot cue buttons
    for (int i = 0; i < 4; i++)
    {
        hotCueButtons[i].setBounds(i * colW, rowH * 6, colW, rowH);
        hotCueButtons[i + 4].setBounds(i * colW, rowH * 7, colW, rowH);
    }

    // EQ sliders
    double eqW = getWidth() / 3.0;
    lowSlider.setBounds(0, rowH * 8, eqW, rowH);
    midSlider.setBounds(eqW, rowH * 8, eqW, rowH);
    highSlider.setBounds(2 * eqW, rowH * 8, eqW, rowH);

    // Button to clear cues
    clearCuesButton.setBounds(0, rowH * 9, getWidth(), rowH);

    // BPM label 
    bpmLabel.setBounds(0, rowH * 10, getWidth(), rowH);
}

void DeckGUI::buttonClicked(juce::Button* button) 
{
    // If play was clicked
    if (button == &playButton)
    {
        player->play();
    }
    // If stop was clicked, stop playback
    else if (button == &stopButton)
    {
        player->stop();
    }
    // if Load was clicked, open file chooser
    else if (button == &loadButton)
    {
        // Allow selecting files only
        auto fileChooserFlags = juce::FileBrowserComponent::canSelectFiles;

        // Launch async chooser so UI doesn't freeze
        fChooser.launchAsync(fileChooserFlags, [this](const juce::FileChooser& chooser)
        {
            // Get the selected file 
            auto file = chooser.getResult();

            // if user picked a real file, load it
            if (file.existsAsFile())
                loadFile(juce::URL{ file });
        });
    }

    // Check if a hot cue button was clicked
    for (int i = 0; i < (int)hotCueButtons.size(); ++i)
    {
        if (button == &hotCueButtons[(size_t)i])
        {
            // trigger cue i on the player
            player->triggerHotCue(i);

            if (onCuesChanged && loadedTrackPath.isNotEmpty())
            {
                // Save current cues to JSON
                onCuesChanged(loadedTrackPath, deckIndex, player->getHotCues());
            }

            // Return 
            return;
        }
    }

    // If clear cue was clicked
    if (button == &clearCuesButton)
    {
        // remove all stored cues in the player
        player->clearHotCues();

        // Notify parent so it can save cleared cues
        if (onCuesChanged && loadedTrackPath.isNotEmpty())
        {
            onCuesChanged(loadedTrackPath, deckIndex, player->getHotCues());
        }
    }
}

// Called when a slider with this listener changes
void DeckGUI::sliderValueChanged(juce::Slider* slider)
{
    // If an EQ slider moved, apply EQ
    if (slider == &lowSlider || slider == &midSlider || &highSlider)
    {
        // Apply EQ settings 
        player->setEQ(lowSlider.getValue(), midSlider.getValue(), highSlider.getValue());

        // Checks if the EQ changed and the loaded track isnt empty
        if (onEQChanged && loadedTrackPath.isNotEmpty())
        {
            onEQChanged(loadedTrackPath, lowSlider.getValue(), midSlider.getValue(), highSlider.getValue());
        }

        // Volume slider controls the gain
        if (slider == &volSlider)
        {
            player->setGain(slider->getValue());
        }

        // Speed Slider controls the playback speed
        if (slider == &speedSlider)
        {
            player->setSpeed(slider->getValue());
        }

        // Position slider controls
        if (slider == &posSlider)
        {
            player->setPositionRelative(slider->getValue());
        }
    }
}

// Asks if the dragged files are acceptable
bool DeckGUI::isInterestedInFileDrag(const juce::StringArray& files)
{
    return true;
}

void DeckGUI::loadFile(juce::URL audioURL)
{
    // Load audio into the player
    player->loadURL(audioURL);

    // Load audio into the waveform display
    waveformDisplay.loadURL(audioURL);

    // Estimate the BPM
    double estimatedBpm = player->estimateBPM(audioURL);

    // Update the BPM Label
    bpmLabel.setText("BPM: " + juce::String(estimatedBpm, 1),
        juce::dontSendNotification);

    // If the URL points to the local file, store its path
    if (audioURL.isLocalFile())
    {
        loadedTrackPath = audioURL.getLocalFile().getFullPathName();

        // Notify the parent that a track was loaded
        if (onTrackLoaded)
        {
            onTrackLoaded(loadedTrackPath, deckIndex, player);
        }
    }
}


// Calls this when files are dropped
void DeckGUI::filesDropped(const juce::StringArray& files, int x, int y)
{
    if (files.size() == 1)
    {
        loadFile(juce::URL{ juce::File{ files[0] } });
    }
}
// Called periodically 
void DeckGUI::timerCallback()
{
    waveformDisplay.setPositionRelative(player->getPositionRelative());
}

// Stores the deck number
void DeckGUI::setDeckIndex(int index)
{
    deckIndex = index;
}

// Pass callbacks into DeckGUI
void DeckGUI::setCueCallbacks(
    std::function<void(const juce::String&, int, const std::array<double, 8>&)> cuesChanged,
    std::function<void(const juce::String&, int, DJAudioPlayer*)> trackLoaded)
{
    // Save the callbacks for later use
    onCuesChanged = std::move(cuesChanged);
    onTrackLoaded = std::move(trackLoaded);
}