/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "modules/audio_processing/audio_buffer.h"

#include <string.h>

#include <cstdint>

#include "common_audio/channel_buffer.h"
#include "common_audio/include/audio_util.h"
#include "common_audio/resampler/push_sinc_resampler.h"
#include "modules/audio_processing/splitting_filter.h"
#include "rtc_base/checks.h"

namespace webrtc {
namespace {

constexpr size_t kSamplesPer32kHzChannel = 320;
constexpr size_t kSamplesPer48kHzChannel = 480;
constexpr size_t kMaxSamplesPerChannel = AudioBuffer::kMaxSampleRate / 100;

size_t NumBandsFromFramesPerChannel(size_t num_frames) {
  if (num_frames == kSamplesPer32kHzChannel) {
    return 2;
  }
  if (num_frames == kSamplesPer48kHzChannel) {
    return 3;
  }
  return 1;
}

}  // namespace

AudioBuffer::AudioBuffer(size_t input_rate,
                         size_t input_num_channels,
                         size_t buffer_rate,
                         size_t buffer_num_channels,
                         size_t output_rate,
                         size_t output_num_channels)
    : AudioBuffer(static_cast<int>(input_rate) / 100,
                  input_num_channels,
                  static_cast<int>(buffer_rate) / 100,
                  buffer_num_channels,
                  static_cast<int>(output_rate) / 100) {}

AudioBuffer::AudioBuffer(size_t input_num_frames,
                         size_t input_num_channels,
                         size_t buffer_num_frames,
                         size_t buffer_num_channels,
                         size_t output_num_frames)
    : input_num_frames_(input_num_frames),
      input_num_channels_(input_num_channels),
      buffer_num_frames_(buffer_num_frames),
      buffer_num_channels_(buffer_num_channels),
      output_num_frames_(output_num_frames),
      output_num_channels_(0),
      num_channels_(buffer_num_channels),
      num_bands_(NumBandsFromFramesPerChannel(buffer_num_frames_)),
      num_split_frames_(rtc::CheckedDivExact(buffer_num_frames_, num_bands_)),
      data_(
          new ChannelBuffer<float>(buffer_num_frames_, buffer_num_channels_)) {
  RTC_DCHECK_GT(input_num_frames_, 0);
  RTC_DCHECK_GT(buffer_num_frames_, 0);
  RTC_DCHECK_GT(output_num_frames_, 0);
  RTC_DCHECK_GT(input_num_channels_, 0);
  RTC_DCHECK_GT(buffer_num_channels_, 0);
  RTC_DCHECK_LE(buffer_num_channels_, input_num_channels_);

  const bool input_resampling_needed = input_num_frames_ != buffer_num_frames_;
  const bool output_resampling_needed =
      output_num_frames_ != buffer_num_frames_;
  if (input_resampling_needed) {
    for (size_t i = 0; i < buffer_num_channels_; ++i) {
      input_resamplers_.push_back(std::unique_ptr<PushSincResampler>(
          new PushSincResampler(input_num_frames_, buffer_num_frames_)));
    }
  }

  if (output_resampling_needed) {
    for (size_t i = 0; i < buffer_num_channels_; ++i) {
      output_resamplers_.push_back(std::unique_ptr<PushSincResampler>(
          new PushSincResampler(buffer_num_frames_, output_num_frames_)));
    }
  }

  if (num_bands_ > 1) {
    split_data_.reset(new ChannelBuffer<float>(
        buffer_num_frames_, buffer_num_channels_, num_bands_));
    splitting_filter_.reset(new SplittingFilter(
        buffer_num_channels_, num_bands_, buffer_num_frames_));
  }
}

AudioBuffer::~AudioBuffer() {}

void AudioBuffer::set_downmixing_to_specific_channel(size_t channel) {
  downmix_by_averaging_ = false;
  RTC_DCHECK_GT(input_num_channels_, channel);
  channel_for_downmixing_ = std::min(channel, input_num_channels_ - 1);
}

void AudioBuffer::set_downmixing_by_averaging() {
  downmix_by_averaging_ = true;
}

void AudioBuffer::CopyTo(AudioBuffer* buffer) const {
  RTC_DCHECK_EQ(buffer->num_frames(), output_num_frames_);

  const bool resampling_needed = output_num_frames_ != buffer_num_frames_;
  if (resampling_needed) {
    for (size_t i = 0; i < num_channels_; ++i) {
      output_resamplers_[i]->Resample(data_->channels()[i], buffer_num_frames_,
                                      buffer->channels()[i],
                                      buffer->num_frames());
    }
  } else {
    for (size_t i = 0; i < num_channels_; ++i) {
      memcpy(buffer->channels()[i], data_->channels()[i],
             buffer_num_frames_ * sizeof(**buffer->channels()));
    }
  }

  for (size_t i = num_channels_; i < buffer->num_channels(); ++i) {
    memcpy(buffer->channels()[i], buffer->channels()[0],
           output_num_frames_ * sizeof(**buffer->channels()));
  }
}

void AudioBuffer::RestoreNumChannels() {
  num_channels_ = buffer_num_channels_;
  data_->set_num_channels(buffer_num_channels_);
  if (split_data_.get()) {
    split_data_->set_num_channels(buffer_num_channels_);
  }
}

void AudioBuffer::set_num_channels(size_t num_channels) {
  RTC_DCHECK_GE(buffer_num_channels_, num_channels);
  num_channels_ = num_channels;
  data_->set_num_channels(num_channels);
  if (split_data_.get()) {
    split_data_->set_num_channels(num_channels);
  }
}

void AudioBuffer::SplitIntoFrequencyBands() {
  splitting_filter_->Analysis(data_.get(), split_data_.get());
}

void AudioBuffer::MergeFrequencyBands() {
  splitting_filter_->Synthesis(split_data_.get(), data_.get());
}

void AudioBuffer::ExportSplitChannelData(size_t channel,
                                         int16_t* const* split_band_data) {
  for (size_t k = 0; k < num_bands(); ++k) {
    const float* band_data = split_bands(channel)[k];

    RTC_DCHECK(split_band_data[k]);
    RTC_DCHECK(band_data);
    for (size_t i = 0; i < num_frames_per_band(); ++i) {
      split_band_data[k][i] = FloatS16ToS16(band_data[i]);
    }
  }
}

void AudioBuffer::ImportSplitChannelData(
    size_t channel,
    const int16_t* const* split_band_data) {
  for (size_t k = 0; k < num_bands(); ++k) {
    float* band_data = split_bands(channel)[k];
    RTC_DCHECK(split_band_data[k]);
    RTC_DCHECK(band_data);
    for (size_t i = 0; i < num_frames_per_band(); ++i) {
      band_data[i] = split_band_data[k][i];
    }
  }
}

}  // namespace webrtc
