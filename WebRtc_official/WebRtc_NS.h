#pragma once
#include "noise_suppression.c"
#include "ns_core.c"
#include "defines.h"
#include "typedefs.h"
#include "checks.cc"
#include <iostream>
//#include <string>

/*
	WebRtc noise suppression module had been supported 48 khz sample reate since c5ebbdq branch.
	The noise suppression process flow can be summed up as below:

	audio data in	->	if fs > 16khz	->	split into frequency bands (16kdz per band,numMaxUpperBands is 2,that is 48 khz)
										->	analyze / process per bands
										->	merge frequency bands
										->	output
					-> if fs < 16khz	->	analyze / process per bands
										->	output
	WebRtc uses AudioBuffer class to split audio date into different bands.								

*/
class WebRtc_NS
{
public:
	enum nsLevel {
		kLow,
		kModerate,
		kHigh,
		kVeryHigh
	};
	NsHandle * nsHandle = nullptr;
	unsigned int sample_rate = 48000;
	nsLevel ns_level = kHigh;
	int Init(unsigned int _sample_rate, nsLevel _ns_level);
	void Process(float _data_in[160], float _data_out[160]) {
		// process per frame should be 10ms, different sample rate requires different points per frame.
		// Max input size is 320 points for 32 khz
		// As to unify,160 points per band is suggested.
		if (sample_rate < 32000) {
			float *input_buffer[1] = { _data_in };				//ns input buffer [band][data]   band:1~2 [L H]
			float *output_buffer[1] = { _data_out };			//ns output buffer [band][data]  band:1~2 [L H]
			WebRtcNs_Analyze(nsHandle, input_buffer[0]);
			WebRtcNs_Process(nsHandle, (const float *const *)input_buffer, 1, output_buffer);	// num_bands = 1 or 2 
			return;
		}
		else if ( sample_rate == 32000 )
		{
			// TODO: add a resampler for 32k sample rate
		}
		else if ( sample_rate == 48000 )
		{
			// TODO: add a resampler for 32k sample rate
		}
		else {
			std::cout << "Only support up to 48000khz";
			return;
		}
		return;
	
	};
	int NS_32(float _data_in[320], float _data_out[320]){
		//// float -> short for QMF
		//short BufferIn[320] = { 0 };
		//for (size_t i = 0; i < 320; i++)
		//{
		//	short tmp =short( _data_in[i] * 322768.0 );
		//	uint8_t bytes[2];
		//	bytes[0] = tmp & 0xFF;
		//	bytes[1] = (tmp >> 8) & 0xFF;

		//	memcpy(BufferIn + i, bytes, sizeof(short) );
		//	//BufferIn[i] = _data_in[i];
		//}
		//short InL[160]{ 0 }, InH[160]{0};
		//short OutL[160]{ 0 }, OutH[160]{ 0 };
		//int  filter_state1[6]{ 0 }, filter_state2[6]{0};
		//int  Synthesis_state1[6]{ 0 }, Synthesis_state2[6]{0};

		//WebRtcSpl_AnalysisQMF(BufferIn, 320, InL, InH, filter_state1, filter_state2);	// filter_state1 �Ƿ���Ҫ����Ϊȫ�֣���
		//short *input_buffer[2] = { InL,InH };					//ns input buffer [band][data]   band:1~2 [L H]
		////float *output_buffer[2] = { data_out,data_out2 };		//ns output buffer [band][data] band:1~2
		////float *output_buffer[2] = { OutL,OutH };				//ns input buffer [band][data]   band:1~2 [L H]
		//WebRtcNs_Process(nsHandle, (const float *const *)input_buffer, 2, (float *const *)output_buffer);
		//short BufferOut[320]{};
		//WebRtcSpl_SynthesisQMF(OutL, OutH, 160, BufferOut, Synthesis_state1, Synthesis_state2);

		return 0;
		
	
	};
	int NS_16(float _data_in[160],float _data_out[160]){
		float *input_buffer[1] = { _data_in  };				//ns input buffer [band][data]   band:1~2 [L H]
		float *output_buffer[1] = { _data_out };			//ns output buffer [band][data]  band:1~2 [L H]
		WebRtcNs_Analyze(nsHandle, input_buffer[0]);
		WebRtcNs_Process(nsHandle, (const float *const *)input_buffer, 1, output_buffer);	// num_bands = 1 or 2 
		return 0;
	};
	int NS_8(float _data_in[80], float _data_out[80]){
		float *input_buffer[1] = { _data_in };				//ns input buffer [band][data]   band:1~2 [L H]
		float *output_buffer[1] = { _data_out };			//ns output buffer [band][data]  band:1~2 [L H]
		WebRtcNs_Analyze(nsHandle, input_buffer[0]);
		WebRtcNs_Process(nsHandle, (const float *const *)input_buffer, 1, output_buffer);	// num_bands = 1 or 2 
		return 0;
	};
	WebRtc_NS();
	virtual ~WebRtc_NS();
};



inline int WebRtc_NS::Init(unsigned int _sample_rate, nsLevel _ns_level)
{
	// Initialization
	sample_rate = _sample_rate;
	ns_level = _ns_level;
	nsHandle = WebRtcNs_Create();
	int status = WebRtcNs_Init(nsHandle, sample_rate);
	if (status != 0) {
		std::cout<<"WebRtcNs_Init fail\n";
		return -1;
	}
	status = WebRtcNs_set_policy(nsHandle, ns_level);
	if (status != 0) {
		std::cout<<"WebRtcNs_set_policy fail\n";
		return -1;
	}
	return 0;
}

WebRtc_NS::WebRtc_NS()
{
}


WebRtc_NS::~WebRtc_NS()
{
}




/*
// some previous code for reference

// TODO:
//	1. �߲������µĽ��봦��
//	2. �����㷨ԭ����
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
vector<vector<double>> output(2);							// Ĭ��Ϊ˫����
int sample_rate = audio_file.getSampleRate();
int total_samples = audio_file.getNumSamplesPerChannel();
input.push_back(audio_file.samples[0]);

if (audio_file.getNumChannels() > 1) {						// ��ȡ˫��������
isMono = false;
input.push_back(audio_file.samples[1]);
}
//	load noise suppression module
//	ÿ�δ��� 10ms ��֡���ݣ���ͬ��������Ҫ��ͬ�ĵ�����8 16 32 ��Ӧ 80 160 320 ��
//  �������̶�����Ϊ 320 �ֽڣ�16khz �����ʻ��ȡ����
size_t samples = MIN(160, sample_rate / 100);				// ԭ��֧��160����,�� 16 k��32khz��Ҫ�������16k��
const int maxSamples = 320;
size_t total_frames = (total_samples / samples);			// �����֡��

//	����������֡����)
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


*/