// my_splitting_filter.cpp: 定义控制台应用程序的入口点。
//

#include <iostream>
#include "AudioFile.h"
#include "signal_processing_library.h"
#include "splitting_filter.h"
#include "three_band_filter_bank.h"
#include "audio_util.h"
using namespace std;
#define TEST_32K
#define TEST_48K


int main()
{
	AudioFile<float> af;
	//af.load("32k_mono.wav");
	af.load("audio_48k_mono.wav");
	if (af.getNumChannels() > 1)		// to mono
	{
		for (size_t i = 0; i < af.getNumSamplesPerChannel(); i++)
		{
			af.samples[0][i] = 0.5 * (af.samples[0][i] + af.samples[1][i]);
		}
		af.samples.pop_back();
	}
	vector<float> data(af.samples[0]);

	//for (size_t i = 0; i < 100; i++)
	//{
	//	cout << "Float data: " << data[i] << "  Int16 data: " << webrtc::FloatToFloatS16(data[i]) <<endl;
	//}
	cout << " \n Next test:\n";
#ifdef TEST_48K

	auto iter = data.begin();

	vector<float> data_out;
	webrtc::ThreeBandFilterBank spl_48k(480);

	for (size_t n = 0; n < data.size() / 480 ; n++)
	{
		vector<float> data_f(iter,iter + 480);

		float band_1[160]{};
		float band_2[160]{};
		float band_3[160]{};
		float * three_band[3] = { band_1,band_2,band_3 };

		float syn_out[480]{};
		spl_48k.Analysis(&data_f[0], 480, three_band);
		spl_48k.Synthesis(three_band, 160, syn_out);
		for (size_t i = 0; i < 480; i++)
		{
			data_out.push_back(syn_out[i]);
		}
		iter += 480;
	}

	vector<vector<float>> temp_out{ data_out };
	af.setAudioBuffer(temp_out);
	af.samples.push_back(data_out);
	af.save("3_bands_splitting_sync_output.wav");
	//vector<vector<float>> three_band(3);
	//for (auto &per_band : three_band) {
	//	per_band.assign(160,0);
	//}


#endif // TEST_48K

#ifdef TEST_32K

	vector<float> data_f(data.begin(), data.begin() + 320);
	vector<int16_t> data_i(320);
	webrtc::FloatToS16(&data_f[0], 320,&data_i[0] );
	//for (size_t i = 0; i < data_f.size(); i++)
	//{
	//	cout << "Float data: " << data_f[i] << "  Int16 data: " << data_i[i] << endl;
	//}
	webrtc::TwoBandsStates TwoBands;
	vector<int16_t> data_i_l(160);
	vector<int16_t> data_i_h(160);

	vector<int16_t> data_i_in(data_i);
	// analysis
	WebRtcSpl_AnalysisQMF(&data_i[0], 320, &data_i_l[0], &data_i_h[0], TwoBands.analysis_state1, TwoBands.analysis_state2);
	// Two bands process
	vector<int16_t> data_i_out(320);
	// synthesis
	WebRtcSpl_SynthesisQMF(&data_i_l[0], &data_i_h[0], 160, &data_i_out[0], TwoBands.synthesis_state1, TwoBands.synthesis_state2);
	for (size_t i = 0; i < 320; i++)
	{
		cout <<"Original data : "<< webrtc::S16ToFloat(data_i_in[i]) <<" data_in : " <<webrtc::S16ToFloat(data_i[i]) << " data_out : " << webrtc::S16ToFloat(data_i_out[i]) << endl;
	}
#endif // TEST_32K

    return 0;
}

