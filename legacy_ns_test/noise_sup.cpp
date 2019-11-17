// WebRtc_noise_suppression.cpp: 定义控制台应用程序的入口点。
//

#include <iostream>
#include "modules/audio_processing/legacy_ns/noise_suppression.h"
#include "AudioFile/AudioFile.h"

using namespace std;

#ifndef MIN
#define MIN(A, B)        ((A) < (B) ? (A) : (B))
#endif

#define WEBRTC_WIN
enum nsLevel {
	kLow,
	kModerate,
	kHigh,
	kVeryHigh
};
using namespace std;

NsHandle* nsInit(int sample_rate, nsLevel);
vector<vector<double>> nsProcess(NsHandle* nsHandle, AudioFile<double> &audio_file);

int main()
{
	// NsHandle *nsHandle = WebRtcNs_Create();
	std::cout << "WebRtc noise suppression \n";

	AudioFile<double> af;
	af.load("audio_with_noise_16k_stereo.wav");

    NsHandle *ns = nsInit(16000, kHigh);
    auto res = nsProcess(ns, af);

	af.setAudioBuffer(res);
	af.save("noise_sup_out.wav");
    return 0;
}

// TODO:
//	1. 高采样率下的降噪处理
//	2. 降噪算法原理讲解
NsHandle* nsInit(int sample_rate, nsLevel ns_level)
{
	// Initialization
	NsHandle *nsHandle = WebRtcNs_Create();
	int status = WebRtcNs_Init(nsHandle, sample_rate);
	if (status != 0) {
		printf("WebRtcNs_Init fail\n");
		return nullptr;
	}

	status = WebRtcNs_set_policy(nsHandle, ns_level);
	if (status != 0) {
		printf("WebRtcNs_set_policy fail\n");
		return nullptr;
	}

	return nsHandle;
};

vector<vector<double>> nsProcess(NsHandle * nsHandle, AudioFile<double> &audio_file)
{
	bool isMono = true;
	vector<vector<double>> input;
	vector<vector<double>> output(2);							// 默认为双声道
	int sample_rate = audio_file.getSampleRate();
	int total_samples = audio_file.getNumSamplesPerChannel();
	input.push_back(audio_file.samples[0]);

	if (audio_file.getNumChannels() > 1) {						// 提取双声道数据
		isMono = false;
		input.push_back(audio_file.samples[1]);
	}
	//	load noise suppression module
	//	每次处理 10ms 的帧数据，不同采样率需要不同的点数，8 16 32 对应 80 160 320 点
	//  缓冲区固定长度为 320 字节，16khz 采样率会读取两次
	size_t samples = MIN(160, sample_rate / 100);				// 原生支持160个点,即 16 k，32khz需要拆成两个16k的
	const int maxSamples = 320;
	size_t total_frames = (total_samples / samples);			// 处理的帧数

																//	主处理函数（帧处理)
	for (int i = 0; i < total_frames; i++) {
		float data_in[maxSamples];
		float data_out[maxSamples];
		float data_in2[maxSamples];
		float data_out2[maxSamples];
		//  input the signal to process,input points <= 160 (10ms)
		for (int n = 0; n != samples; ++n) {
			data_in[n] = input[0][samples * i + n];
			data_in2[n] = input[1][samples * i + n];
		}
		float *input_buffer[2] = { data_in ,data_in2 };			//ns input buffer [band][data]   band:1~2
		float *output_buffer[2] = { data_out,data_out2 };		//ns output buffer [band][data] band:1~2
		WebRtcNs_Analyze(nsHandle, input_buffer[0]);
		WebRtcNs_Process(nsHandle, (const float *const *)input_buffer, isMono ? 1 : 2, output_buffer);	// num_bands = 1 or 2 
																										//WebRtcSpl_AnalysisQMF
																										//	output the processed signal
		for (int n = 0; n != samples; ++n) {
			output[0].push_back(output_buffer[0][n]);			// Lift band	
			if (!isMono)
				output[1].push_back(output_buffer[1][n]);		// Right band

		}

	}

	WebRtcNs_Free(nsHandle);

	return output;
}
