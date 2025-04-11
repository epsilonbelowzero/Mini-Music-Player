#pragma once

extern "C" {
//https://rodic.fr/blog/libavcodec-tutorial-decode-audio-file/
#include <libswresample/swresample.h>
}

//uses SWResample library to convert any input to MONO PCM format with
//16 bits

class Converter {
	private:
		SwrContext* swr;
		AVChannelLayout outChannelLayout;
		
		enum AVSampleFormat outSampleFmt = AV_SAMPLE_FMT_S16;
		int channels = 2;
		
		void ce(int errnum, std::string msg) {
			if( errnum < 0 ) {
				char err[AV_ERROR_MAX_STRING_SIZE];
				av_strerror(errnum, err, AV_ERROR_MAX_STRING_SIZE);
				
				std::stringstream ss;
				ss << msg << ":" << err;
				
				throw std::runtime_error(ss.str());
			}
		}
		
		void init_( AVCodecContext* aCodecCtx,
						int64_t outChLayout,
						enum AVSampleFormat outSampleFmt_,
						int outSampleRate_
		) {
			int outSampleRate = outSampleRate_ == -1 ? aCodecCtx->sample_rate : outSampleRate_;
			outSampleFmt = outSampleFmt_;
			av_channel_layout_from_mask( &outChannelLayout, outChLayout );
			swr_alloc_set_opts2( &swr,
					&outChannelLayout, outSampleFmt, outSampleRate,
					&(aCodecCtx->ch_layout), aCodecCtx->sample_fmt, aCodecCtx->sample_rate,
					0, NULL
			);
			ce( -(swr == NULL), "Coudln't alloc swr-context");
			ce( swr_init( swr ), "Coudn't init swr.");
		}
	public:
		~Converter() {
			if( swr ) {
				swr_free( &swr );
			}
		}
	
		void init(AVCodecContext* aCodecCtx) {
			init_( aCodecCtx, AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16, -1);
		}
		void init( AVCodecContext* aCodecCtx,
						int64_t outChLayout,
						enum AVSampleFormat outSampleFmt_,
						int outSampleRate_
		) {
			init_( aCodecCtx, outChLayout, outSampleFmt_, outSampleRate_ );
		}
		
		uint8_t* convert(uint8_t** data, int samples, int* outputSamples) {
			uint8_t* output;
			//convert input to signed 16Bit-Stereo, same freq
			ce( av_samples_alloc( &output, NULL, channels, samples, outSampleFmt, 0 ), "Couldn't alloc resembled sample output buffer.");
			*outputSamples = swr_convert( swr, &output, samples, (const uint8_t**) data, samples );
			ce( *outputSamples, "Couldn't resample decoded audio.");
			
			return output;
		}
};
