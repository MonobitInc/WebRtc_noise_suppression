#pragma once


#include <vector>
#include "AudioFile/AudioFile.h"

using AudioFileFlt = AudioFile<float>;

template <class T>
class VAFrame
{
public:
    VAFrame(uint32_t srate):sampleRate(srate) {}

    /** @Returns the sample rate */
    uint32_t getSampleRate() const { return sampleRate; }
    
    /** @Returns the number of audio channels in the buffer */
    int getNumChannels() const { return buf.size(); }

    /** @Returns the number of samples per channel */
    int getNumSamplesPerChannel() const { return buf[0].size(); }

    AudioFileFlt::AudioBuffer buf;

private:
    uint32_t sampleRate;

};

using VAFrameFlt = VAFrame<float>;

