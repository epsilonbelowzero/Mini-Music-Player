#pragma once

#include <vector>

#include "Converter.hpp"

extern "C" {
//https://rodic.fr/blog/libavcodec-tutorial-decode-audio-file/
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/dict.h>
}

//Uses FFMPEG's libraries to open media files, find the first auto stream
//and decode it. Uses Converter-class for a MONO-16bit output to it's
//buffer ready to be played using OpenAL

class Loader {
	private:
		//vectors for multiple files
		std::vector<std::string> fileNames;
		std::vector<AVFormatContext*> pFormatCtxs;
		
		std::vector<int> audioStreams;
		std::vector<AVCodecContext*> aCodecCtxs;
		std::vector<const AVCodec*> aCodecs;
		
		AVFrame* frame = NULL;
		AVPacket* packet = NULL;
		bool noNewRead = false;
		
		std::vector<Converter*> convs;
		
		std::vector<int> completes;
		
		int bufferSize = 1048575;//1MB
		std::vector<int> freqs;
	
		//error checking function
		void ce(int errnum, std::string msg) {
			if( errnum < 0 ) {
				char err[AV_ERROR_MAX_STRING_SIZE];
				av_strerror(errnum, err, AV_ERROR_MAX_STRING_SIZE);
				
				std::stringstream ss;
				ss << msg << ":" << err;
				
				throw std::runtime_error(ss.str());
			}
		}
		
#pragma GCC diagnostic ignored "-Wunused-parameter"
		//get rid of those nasty codec-warnings
		//pragmas are usefull to suppress the unsused paramter warnings
		static void avLogCallback( void* _, int __, const char* ___, va_list ____) {}
#pragma GCC diagnostic pop
	
	public:
		uint8_t* audioBuffer = nullptr;
		int size;
		
		~Loader() {
			close();
		}
		
		Loader() {
			av_log_set_callback(avLogCallback);
		}
	
		Loader(char** name) {
			av_log_set_callback(avLogCallback);
			
			if( *name ) {
				init( name );
			}
		}
		
		Loader& init(char** name) {
			//adds the required data structures to the class variables
			fileNames.push_back(*name);
			completes.push_back( 2 );
			
			AVFormatContext* pFormatCtx = NULL;
			ce( 
				avformat_open_input( &pFormatCtx, *name, NULL, NULL),
				"Coudln't open file"
			);			
			pFormatCtxs.push_back( pFormatCtx );
			
			return *this;
		}
		
		void printBanner(int i = 0) {
			//printes some meta information (title, artist etc.)
			if( (uint) i >= completes.size() )
				return;
			if( i < 0 ) {
				throw std::runtime_error("printBanner(): uninitialized i");
			}
			float duration = pFormatCtxs[i]->duration / 1e6;
			AVDictionaryEntry* tmp;
			tmp = av_dict_get(pFormatCtxs[i]->metadata, "title", NULL, AV_DICT_IGNORE_SUFFIX);
			std::string title((tmp ? tmp->value : ""));
			tmp = av_dict_get(pFormatCtxs[i]->metadata, "artist", NULL, AV_DICT_IGNORE_SUFFIX);
			std::string artist((tmp ? tmp->value : ""));
			
			std::stringstream ss;
			ss << '\r' << "Now playing »" << fileNames[i] << "« (";
			if( !title.empty() ) {
				ss << "'" << title << "' ";
			}
			if( ! artist.empty() ) {
				ss << "by '" << artist << "' ";
			}
			if( artist.empty() && title.empty() ) {
			} else {
				ss << "- ";
			}
			ss << std::setfill('0') << std::setw(2) << std::fixed << std::setprecision(0);
			ss << floor( duration / (3600)) << ":";
			ss << std::setfill('0') << std::setw(2) << std::fixed << std::setprecision(0);
			ss << fmod(floor( duration / (60)), 60) << ":";
			ss << std::setfill('0') << std::setw(4) << std::fixed << std::setprecision(1);
			ss << fmod(duration, 60);
			ss << ")" << std::endl;
			std::cout << ss.str() << std::flush;
		}
		
		//functions to navigate the data structures when a media file
		//is finished playing
		bool complete() {
			return completes.back() == 0;
		}
		int actSong() {
			int i = completes.size() - 1;
			while( i > 0 && completes[i] > 1 ) {
				i--;
			}
			return i;
		}
		int nextSong() {
			uint i = 0;
			while( i + 1 < completes.size() && completes[i] < 2 ) {
				i++;
			}
			return i;
		}
		void songCompleted() {
			uint i = completes.size() - 1;
			while( completes[i] == 2 && i > 0 ) {
				i--;
			}
			completes[i] = 0;
			if( completes.size() > i + 1 ) {
				completes[i+1] = 1;
			}
		}
		
		//interface to ffmpeg's functions
		Loader& findStreamInfo() {
			ce( avformat_find_stream_info( pFormatCtxs.back(), NULL ), "Couldn't find stream-info");
			
			return *this;
		}
		
		Loader& dumpFormat() {
			av_dump_format( pFormatCtxs.back(), 0, fileNames.back().c_str(), 0 );
			
			return *this;
		}
		
		//checks all streams for type audio. terminates when it founds one
		Loader& findAudioStream() {
			audioStreams.push_back( -1 );
			for( uint i = 0; i < pFormatCtxs.back()->nb_streams; i++ ) {
				if( pFormatCtxs.back()->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO &&
					audioStreams.back() < 0 )
				{
					audioStreams.back() = i;
				}
			}
			ce( audioStreams.back(), "Couldn't find audio-stream.");
			
			return *this;
		}
		
		Loader& createAudioContext() {
			aCodecCtxs.push_back(avcodec_alloc_context3(NULL) );
			ce(
				avcodec_parameters_to_context( aCodecCtxs.back(), pFormatCtxs.back()->streams[audioStreams.back()]->codecpar ),
				"Couldn't create audio context."
			);
			freqs.push_back(aCodecCtxs.back() -> sample_rate );
			
			return *this;
		}
		
		Loader& findDecoder() {
			aCodecs.push_back( avcodec_find_decoder(aCodecCtxs.back()->codec_id) );
			ce( -(aCodecs.back() == NULL), "Couldn't find matching codec.");
			
			return *this;
		}
		Loader& openDecoder() {
			ce( avcodec_open2( aCodecCtxs.back(), aCodecs.back(), NULL ), "Couldn't open decoder.");
			
			return *this;
		}
		
		Loader& setAudioBufferSize(int size) {
			bufferSize = size;
			if( audioBuffer ) {
				audioBuffer = (uint8_t*) realloc( audioBuffer, bufferSize );
			}
			
			return *this;
		}
		int getFreq() {
			return freqs[actSong()];
		}
		AVCodecContext* getAudioCodecContext() {
			return aCodecCtxs.back();
		}
		Loader& registerConverter( Converter* conv_ ) {
			convs.push_back( conv_ );
			return *this;
		}
		Loader& registerConverter(  ) {
			convs.push_back( nullptr );
			return *this;
		}
		Loader& unregisterConverter() {
			convs.back() = nullptr;
			
			return *this;
		}
		
		//uses ffmpeg functions to fill the audio buffer, until its size
		//is reached. as ffmpeg may decode more than there is room to
		//store it, noNewRead stores this information to not decode more
		//when the buffer needs to be refilled
		void fillAudioBuffer() {
			if( ! audioBuffer ) {
				audioBuffer = (uint8_t*) malloc( bufferSize );
			}
			size = 0;
			int dataSize, outputSamples;
			if( !noNewRead ) {
				packet = av_packet_alloc();
				frame = av_frame_alloc();
				ce( -(packet == NULL || frame == NULL), "Couldn't allocate mem for packet or frame");
			}
			while( noNewRead || av_read_frame( pFormatCtxs[actSong()], packet ) >= 0 )
			{
				if( noNewRead || packet->stream_index == audioStreams[actSong()] ) {
					if( ! noNewRead ) {
						try {
						    ce( avcodec_send_packet( aCodecCtxs[actSong()], packet ) ,"Coudln't send packet");
						} catch(const std::runtime_error& e) {
							std::cerr << '\r' << e.what() << std::endl;
							continue;
						}
					}
					
					while( noNewRead || avcodec_receive_frame( aCodecCtxs[actSong()], frame ) == 0) 
					{
						dataSize = av_samples_get_buffer_size( NULL, aCodecCtxs[actSong()]->ch_layout.nb_channels, frame->nb_samples, aCodecCtxs[actSong()]->sample_fmt, 1 );
						ce( dataSize, "Couldn't compute size of decoded audio");
												
						if( size + dataSize >= bufferSize ) {
							noNewRead = true;
							return;
						}
						assert( dataSize > 0 );
						
						if( convs[actSong()] ) {
							uint8_t* output = convs[actSong()]->convert( frame->data, frame->nb_samples, &outputSamples );
							memcpy( audioBuffer + size, output, 2 * outputSamples );
							av_freep( &output );
							size += 2 * outputSamples;
						}
						else {
							memcpy( audioBuffer + size, frame->data[0], dataSize );
							size += dataSize;
						}
						noNewRead = false;
						
						av_frame_unref( frame );
					}
				}
				
				av_packet_unref( packet );
			}
			songCompleted();
		}
		
		void close() {
			for(auto aCodecCtx : aCodecCtxs ) {
				if( aCodecCtx ) {
					avcodec_free_context( &aCodecCtx );
					aCodecCtx = NULL;
				}
			}
			for( auto pFormatCtx : pFormatCtxs ) {
				if( pFormatCtx ) {
					avformat_close_input( &pFormatCtx );
					pFormatCtx = NULL;
				}
			}
			for( auto conv : convs) {
				if( conv ) {
					delete conv;
				}
			}
			if( audioBuffer ) {
				free( audioBuffer );
			}
		}
};
