// Include the DJAudioPlayer class defintion
#include "DJAudioPlayer.h"


// Constructor
DJAudioPlayer::DJAudioPlayer(juce::AudioFormatManager& _formatManager) : formatManager(_formatManager)
{

}

// Destructor
DJAudioPlayer::~DJAudioPlayer()
{

}

// Called before playback starts
void DJAudioPlayer::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    // Prepare playback transport
    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);

    // Prepare resampling source
    resampleSource.prepareToPlay(samplesPerBlockExpected, sampleRate);

    // Setup DSP specification
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlockExpected);
    spec.numChannels = 2u;

    // Prepare EQ Filter chain
    eqChain.prepare(spec);

    // Intitalize EQ to neutral
    setEQ(1.0, 1.0, 1.0);
}

// Sets the equalisers
void DJAudioPlayer::setEQ(double lowGain, double midGain, double highGain)
{
    // Sample rate is 44100.0
    double sampleRate = 44100.0;

    // Checks if the reader source is null
    if (readerSource != nullptr) 
    {
        sampleRate = readerSource->getAudioFormatReader()->sampleRate;
    }

    // Ensure gain is at least a tiny non-zero value
    float safeLow = std::max(0.001f, (float)lowGain);
    float safeMid = std::max(0.001f, (float)midGain);
    float safeHigh = std::max(0.001f, (float)highGain);

    auto lowCoeffs = juce::dsp::IIR::Coefficients<float>::makeLowShelf(sampleRate, 200.0f, 0.7f, safeLow);
    auto midCoeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 1000.0f, 1.0f, safeMid);
    auto highCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, 5000.0f, 0.7f, safeHigh);

    *eqChain.get<0>().state = *lowCoeffs;
    *eqChain.get<1>().state = *midCoeffs;
    *eqChain.get<2>().state = *highCoeffs;
}

// Repeatedly called to fill audio buffer
void DJAudioPlayer::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    // Pull audio from transport
    resampleSource.getNextAudioBlock(bufferToFill);

    // Apply EQ only when audio is loaded
    if (readerSource != nullptr)
    {
        // Wrap buffer
        juce::dsp::AudioBlock<float> block(*bufferToFill.buffer);

        auto subBlock = block.getSubBlock((size_t)bufferToFill.startSample, (size_t)bufferToFill.numSamples);

        // In-place processing
        juce::dsp::ProcessContextReplacing<float> context(block);

        // Apply equaliser filters
        eqChain.process(context);
    }
}

// Called when audio stops 
void DJAudioPlayer::releaseResources()
{
    // Transport buffers released
    transportSource.releaseResources();

    // Resampler buffers released
    resampleSource.releaseResources();
}

// Load audio file
void DJAudioPlayer::loadURL(juce::URL audioURL)
{
    // Create a reader for the file
    auto* reader = formatManager.createReaderFor(audioURL.createInputStream(false));

    if (reader != nullptr)
    {
        // Wrap reader in AudioFormatReaderSource
        std::unique_ptr<juce::AudioFormatReaderSource> newSource(
            new juce::AudioFormatReaderSource(reader, true));

        // Attach source
        transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);

        // Reset speed to normal
        resampleSource.setResamplingRatio(1.0);

        // Transfer ownership
        readerSource.reset(newSource.release());
    }
}

// Sets the volume
void DJAudioPlayer::setGain(double gain)
{
    if (gain < 0 || gain > 1.0)
    {
        std::cout << "Gain should be between 0 and 1" << std::endl;
    }
    else
    {
        // Apply the volume
        transportSource.setGain(gain);
    }
}

// Set the playback speed
void DJAudioPlayer::setSpeed(double ratio)
{
    if (ratio < 0.5 || ratio > 2.0)
    {
        return;
    }

    // Apply the speed change
    resampleSource.setResamplingRatio(ratio);
}

// Return the current position
double DJAudioPlayer::getCurrentPosition() const
{
    return transportSource.getCurrentPosition();
}

// Jump to specific time
void DJAudioPlayer::setPosition(double seconds)
{
    transportSource.setPosition(seconds);
}

// Store hot cue
void DJAudioPlayer::setHotCue(int index)

{
    if (index < 0 || index >= hotCues.size())
    {
        return;
    }

    hotCues[index] = getCurrentPosition();
}

// Trigger hot cue 
void DJAudioPlayer::triggerHotCue(int index)
{
    if (index < 0 || index >= hotCues.size())
    {
        return;
    }

    double cuePosition = hotCues[index];

    if (cuePosition >= 0.0)
    {
        // jump to the cue
        setPosition(cuePosition);
    }
    else
    {
        // Create cue
        setHotCue(index);
    }
}

// Clear all hot cues
void DJAudioPlayer::clearHotCues()
{
    for (auto& c : hotCues)
    {
        c = -1.0;
    }
}

// Return all cues
std::array<double, 8> DJAudioPlayer::getHotCues() const
{
    return hotCues;
}

// Restore cues from saved data 
void DJAudioPlayer::setHotCues(const std::array<double, 8>& cues)
{
    hotCues = cues;
}

// Set playback position
void DJAudioPlayer::setPositionRelative(double pos)
{
    if (pos < 0 || pos > 1.0)
    {
        std::cout << "Position must be between 0 and 1" << std::endl;
    }
    else
    {
        double posInSecs = transportSource.getLengthInSeconds() * pos;
        setPosition(posInSecs);
    }
}

// Start the playback
void DJAudioPlayer::play()
{
    transportSource.start();
}

// Stop the playback
void DJAudioPlayer::stop()
{
    transportSource.stop();
}

// Return relative playhead position
double DJAudioPlayer::getPositionRelative()
{
    double len = transportSource.getLengthInSeconds();
    if (len <= 0.0)
    {
        return 0.0;
    }

    return transportSource.getCurrentPosition() / len;
}

// A BPM Estimation using peak detection
double DJAudioPlayer::estimateBPM(juce::URL audioURL)
{
    auto* reader = formatManager.createReaderFor(audioURL.createInputStream(false));

    if (!reader)
    {
        // fallback
        return 120.0;
    }

    juce::AudioBuffer<float> buffer(reader->numChannels, reader->lengthInSamples);
    reader->read(&buffer, 0, reader->lengthInSamples, 0, true, true);

    double sampleRate = reader->sampleRate;

    const float* channelData = buffer.getReadPointer(0);

    float threshold = 0.7f;
    int minSamplesBetweenPeaks = (int)(sampleRate * 0.2);
    int lastPeak = -minSamplesBetweenPeaks;

    std::vector<double> intervals;

    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        if (std::abs(channelData[i] > threshold && (i - lastPeak) > minSamplesBetweenPeaks))
        {
            if (lastPeak >= 0)
            {
                intervals.push_back((i - lastPeak) / sampleRate);
            }

            lastPeak = i;
        }
    }

    if (intervals.empty())
    {
        return 120.0;
    }

    double avg = 0.0;

    for (double v : intervals)
    {
        avg += v;
    }

    avg /= intervals.size();

    double bpm = 60.0 / avg;

    if (bpm < 60 || bpm > 200)
    {
        return 120.0;
    }

    return bpm;
}