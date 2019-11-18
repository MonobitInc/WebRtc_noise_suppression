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

vector<vector<float>> nsProcess(NSupressor nsHandle, AudioFileFlt *audio_file)
{
	bool isMono = true;
	vector<vector<float>> input;
	vector<vector<float>> output(2);							// 默认为双声道

	int sample_rate = audio_file->getSampleRate();
	int total_samples = audio_file->getNumSamplesPerChannel();
	input.push_back(audio_file->samples[0]);

	if (audio_file->getNumChannels() > 1) {						// 提取双声道数据
		isMono = false;
		input.push_back(audio_file->samples[1]);
	}

    AudioBuffer ab(audio_file->getSampleRate(),
                   audio_file->getNumChannels(),
                   audio_file->getSampleRate(),
                   audio_file->getNumChannels(),
                   audio_file->getSampleRate(),
                   audio_file->getNumChannels()
            );

	//	load noise suppression module
	//	每次处理 10ms 的帧数据，不同采样率需要不同的点数，8 16 32 对应 80 160 320 点
	//  缓冲区固定长度为 320 字节，16khz 采样率会读取两次
	size_t samples = MIN(160, sample_rate / 100);				// 原生支持160个点,即 16 k，32khz需要拆成两个16k的
	const int maxSamples = sample_rate / 100;
	size_t total_frames = (total_samples / samples);			// 处理的帧数

																//	主处理函数（帧处理)
	for (int i = 0; i < total_frames; i++) {
		vector<float> data_in(maxSamples);
		vector<float> data_out(maxSamples);
		vector<float> data_in2(maxSamples);
		vector<float> data_out2(maxSamples);
		//  input the signal to process,input points <= 160 (10ms)
		for (int n = 0; n != samples; ++n) {
			data_in[n] = input[0][samples * i + n];
			data_in2[n] = input[1][samples * i + n];
		}
        VAFrameFlt input_buffer(sample_rate), output_buffer(sample_rate);
        input_buffer.buf.push_back(data_in);
        input_buffer.buf.push_back(data_in2);
        output_buffer.buf.push_back(data_out);
        output_buffer.buf.push_back(data_out2);

        ab.CopyFrom(&input_buffer);

        nsHandle->Analyze(ab);
        nsHandle->Process(&ab);

        ab.CopyTo(&output_buffer);
        for (int i = 0; i < samples; i++) {
            output[0].push_back(output_buffer.buf[0][i]);
            if (!isMono) {
                output[1].push_back(output_buffer.buf[1][i]);
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
    ns = std::make_unique<NoiseSuppressor>(cfg, 16000, af.getNumChannels());
    auto res = nsProcess(ns, &af);

	af.setAudioBuffer(res);
	af.save(fileOut);
    return 0;
}

