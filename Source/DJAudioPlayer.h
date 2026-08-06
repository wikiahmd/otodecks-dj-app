// prevents header from being included more than once
#pragma once

// Includes JUCE classes
#include <JuceHeader.h>

// Includes the JUCE DSP module
#include <juce_dsp/juce_dsp.h>

// This is an audio engine class
class DJAudioPlayer : public juce::AudioSource
{
	//public:
public:

	// Constructor
	DJAudioPlayer(juce::AudioFormatManager& formatManager);

	// Destructor
	~DJAudioPlayer();

	// Called before audio playback begins
	void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;

	// Called repeatedly by JUCE to fill the output audio buffer
	void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;

	// Free the resources
	void releaseResources() override;

	// loads an audio file
	void loadURL(juce::URL audioURL);

	// Sets output volume
	void setGain(double gain);

	// Sets playback speed
	void setSpeed(double ratio);

	// Sets playback position
	void setPositionRelative(double pos);

	// Starts playback
	void play();

	// Stops playback
	void stop();

	// Gets the relative position
	double getPositionRelative();

	// Gets the current playback position
	double getCurrentPosition() const;

	// Sets playback position 
	void setPosition(double seconds);

	// Sets a hot cue
	void setHotCue(int index);

	// Jumps to the hot cue
	void triggerHotCue(int index);

	// Clears all stored hot cues
	void clearHotCues();

	// Returns all cue positions as an array of 8 values
	std::array<double, 8> getHotCues() const;

	// Sets cue positions '
	void setHotCues(const std::array<double, 8>& cues);

	// Applies Equaliser
	void setEQ(double lowGain, double midGain, double highGain);

	// Estimate BPM
	double estimateBPM(juce::URL audioURL);

private:

	// 8 cue points all defaulted to -1
	std::array<double, 8> hotCues{ -1, -1, -1, -1, -1, -1, -1, -1 };

	// Used to create AudioFormatReaders
	juce::AudioFormatManager& formatManager;

	// unique_ptr means DJAudioPlayer owns it
	std::unique_ptr<juce::AudioFormatReaderSource> readerSource;

	// Controls transport-style playback
	juce::AudioTransportSource transportSource;

	// ResamplingAudioSource changes playback speed
	juce::ResamplingAudioSource resampleSource{ &transportSource, false, 2 };

	// A filter that processes float samples
	using IIRFilter = juce::dsp::IIR::Filter<float>;

	// Coefficients define filter shape
	using IIRCoeffs = juce::dsp::IIR::Coefficients<float>;

	// ProcessorDuplicator makesa mono processor work on multi-channel data by duplicating 
	using DuplicatedIIR = juce::dsp::ProcessorDuplicator<IIRFilter, IIRCoeffs>;

	// ProcessorChain allows stacking multiple DSP processors
	juce::dsp::ProcessorChain<
		DuplicatedIIR,
		DuplicatedIIR,
		DuplicatedIIR
	> eqChain;
};

