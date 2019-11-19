// WebRtc noise suppression

#include <vector>
#include <iostream>
#include "modules/audio_processing/ns/noise_suppressor.h"
#include "AudioFile/AudioFile.h"
#include "VAFrame/VAFrame.h"

using std::vector;
using namespace webrtc;

#ifndef MIN
#define MIN(A, B)        ((A) < (B) ? (A) : (B))
#endif

using NSupressor = std::shared_ptr<NoiseSuppressor>;

vector<vector<float>> nsProcess(NSupressor nsHandle, AudioFileFlt *audio_file, float gain)
{
	bool isMono = true;
    int channelNum = audio_file->getNumChannels();
	vector<vector<float>> output(channelNum);

	int sample_rate = audio_file->getSampleRate();
	int total_samples = audio_file->getNumSamplesPerChannel();

    AudioBuffer ab(audio_file->getSampleRate(),
                   audio_file->getNumChannels(),
                   audio_file->getSampleRate(),
                   audio_file->getNumChannels(),
                   audio_file->getSampleRate(),
                   audio_file->getNumChannels()
            );

	//	load noise suppression module
    // 每个Frame大小为10ms数据，与AudioBuffer保持一致
	const size_t samples = sample_rate / 100;
	size_t total_frames = (total_samples / samples);			// 处理的帧数

	for (int i = 0; i < total_frames; i++) {
        VAFrameFlt input_buffer(sample_rate), output_buffer(sample_rate);
        for (int c = 0; c < channelNum; c++) {
            vector<float> data_in(samples);
            vector<float> data_out(samples);

            input_buffer.buf.push_back(std::move(data_in));
            output_buffer.buf.push_back(std::move(data_out));
        }

        for (int c = 0; c < channelNum; c++) {
		    for (int n = 0; n != samples; ++n) {
			    input_buffer.buf[c][n] = audio_file->samples[c][samples * i + n];
            }
		}
        ab.CopyFrom(&input_buffer);

        nsHandle->Analyze(ab);
        nsHandle->Process(&ab);

        ab.CopyTo(&output_buffer);
        for (int c = 0; c < channelNum; c++) {
            for (int i = 0; i < samples; i++) {
                output[c].push_back(output_buffer.buf[c][i] * gain);
            }
        }
	}

	return output;
} 

int main(int argc, char **argv)
{
    char defaultFileIn[] = "../assets/audio_with_noise_16k_stereo.wav";
    char defaultFileOut[] = "default_out.wav";
    char *fileIn = defaultFileIn;
    char *fileOut = defaultFileOut;
    if (argc >= 2) {
        fileIn = argv[1];
    }
    if (argc >= 3) {
        fileOut = argv[2];
    }

	AudioFile<float> af;
    printf("In file name: %s\n", fileIn);
    printf("Out file name: %s\n", fileOut);
	af.load(fileIn);
    af.printSummary();

    NSupressor ns;
    NsConfig cfg;
    cfg.target_level = NsConfig::SuppressionLevel::k18dB;
    ns = std::make_unique<NoiseSuppressor>(cfg, af.getSampleRate(), af.getNumChannels());
    auto res = nsProcess(ns, &af, 4); // +6dB

	af.setAudioBuffer(res);
	af.save(fileOut);
    return 0;
}

