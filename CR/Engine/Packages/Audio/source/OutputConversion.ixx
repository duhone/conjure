module;

#include "core/Log.h"
#include <samplerate.h>

export module CR.Engine.Audio.OutputConversion;

import CR.Engine.Audio.ChannelWeights;
import CR.Engine.Audio.Constants;
import CR.Engine.Audio.Sample;

import<span>;
import<vector>;

using SRC_STATE = struct SRC_STATE_tag;

namespace CR::Engine::Audio {
	// Handle conversion for engine standard audio format, to devices format
	export class OutputConversion {
	  public:
		OutputConversion();
		~OutputConversion();

		OutputConversion(const OutputConversion&) = delete;
		OutputConversion(OutputConversion&&)      = delete;

		OutputConversion& operator=(const OutputConversion&) = delete;
		OutputConversion& operator=(OutputConversion&&) = delete;

		void ConvertSampleRate(int32_t a_deviceSampleRate, std::span<const Sample> a_input,
		                       std::span<Sample> a_output);

		// a_output must have a_weights.size() entries per Sample in a_input.
		void ConvertChannelCount(const std::span<Sample> a_input, std::span<float> a_output,
		                         const std::vector<ChannelWeights>& a_weights) const;

	  private:
		SRC_STATE* m_srcState = nullptr;
		bool m_firstFrame     = true;
	};
}    // namespace CR::Engine::Audio

module : private;

namespace cea = CR::Engine::Audio;

cea::OutputConversion::OutputConversion() {
	int error  = 0;
	m_srcState = src_new(SRC_SINC_FASTEST, cea::c_mixChannels, &error);
	CR_ASSERT(m_srcState, "Failed to create lib sample rate state: error: {}", src_strerror(error));
}

cea::OutputConversion::~OutputConversion() {
	src_delete(m_srcState);
}

void cea::OutputConversion::ConvertSampleRate(int32_t a_deviceSampleRate, std::span<const Sample> a_input,
                                              std::span<Sample> a_output) {
	SRC_DATA srcData;
	srcData.end_of_input = 0;
	srcData.src_ratio    = static_cast<double>(a_deviceSampleRate) / cea::c_mixSampleRate;
	srcData.data_in      = (float*)a_input.data();
	srcData.input_frames = (long)a_input.size();

	srcData.data_out      = (float*)a_output.data();
	srcData.output_frames = (long)a_output.size();

	int error = src_process(m_srcState, &srcData);
	CR_ASSERT(error == 0, "failed to resample audio: {}", src_strerror(error));
	CR_ASSERT(srcData.input_frames_used == a_input.size(), "resampling must consume all input currently");

	int silence = (int)a_output.size() - srcData.output_frames_gen;
	if(silence > 0) {
		CR_ASSERT(m_firstFrame, "only the first frame should have an incomplete resample result");
		memmove(a_output.data() + silence, a_output.data(), srcData.output_frames_gen * sizeof(Sample));
		memset(a_output.data(), 0, silence * sizeof(Sample));
	}
	m_firstFrame = false;
}

void cea::OutputConversion::ConvertChannelCount(const std::span<Sample> a_input, std::span<float> a_output,
                                                const std::vector<ChannelWeights>& a_weights) const {
	CR_ASSERT(a_weights.size() * a_input.size() == a_output.size(), "Invalid input to ConvertChannelCount");

	int32_t inputStep = static_cast<int32_t>(a_weights.size());
	for(int32_t sample = 0; sample < a_input.size(); ++sample) {
		for(int32_t weight = 0; weight < inputStep; ++weight) {
			a_output[sample * inputStep + weight] = a_weights[weight].Left * a_input[sample].Left +
			                                        a_weights[weight].Right * a_input[sample].Right;
		}
	}
}