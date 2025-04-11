#include <cstdio>
#include <iostream>
#include <iomanip>
#include <vector>

#include <unistd.h> //for usleep
#include <cmath>
#include <cstdarg>

#include <thread>
#include <mutex>
#include <condition_variable>

#include <stdexcept>

extern "C" {
//https://rodic.fr/blog/libavcodec-tutorial-decode-audio-file/
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include "OpenAL.h"
#include "Converter.hpp"
#include "Loader.hpp"
#include "Song.hpp"

const float T = 200;
const float PI = 3.14156;

//uses a second thread to refill the audio buffer when playing

std::mutex mutexLoader;
std::condition_variable condResumeLoader;
//InterThreadCommunication-Variable:
// - Main -> Sub:
//		+ 1: Refill Audio Buffer
//		+ 0: Exit
//	- Sub -> Main:
//		+ -1: EOF, Buffer refilled
//		+ -2: Buffer refilled
int threadState = 3;

void threadLoadAudioData(Loader& load, Song& song) {
	do{
		std::unique_lock<std::mutex> lck( mutexLoader );
		condResumeLoader.wait( lck, []() { return threadState >= 0; });
		
		switch( threadState ) {
			case 1:
				if( load.complete() ) {
					threadState = -1;
				} else {
					song.push();
					load.fillAudioBuffer();
					threadState = -2;
				}
				break;
			case 0:
			default:
				break;
		}
	} while( threadState != 0 );
}

int main(int argc, char** argv)  {
	if( argc < 2 ) {
		std::cerr << "Usage: " << argv[0] << " <filename(s)>" << std::endl;
		return EXIT_FAILURE;
	}
	
	//registers for all command line arguments a loader
	Loader load;
	for(int i = 1; i <= argc - 1; i++) {
		load.init( &(argv[i]) ).dumpFormat()
			.findStreamInfo().findAudioStream().createAudioContext()
			.findDecoder().openDecoder();
		//as the source audio may be different for each file, need a new one for each file
		Converter* conv = new Converter();
		try{
			conv->init( load.getAudioCodecContext(), AV_CH_LAYOUT_MONO, AV_SAMPLE_FMT_S16, -1 );
			load.registerConverter( conv );
		} catch(const std::runtime_error& e) {
			load.registerConverter( );
		}
	}

	//setup OpenAl with one listener and one source
	OpenAL al;
	al.createContext().makeCurrent();

	std::array<ALfloat,6> ori{{ 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f }};
	al.getListener().setPosition(0, 0, 0).setVelocity(0, 0, 0)
		.setOrientation(ori);
	al.makeCurrent().genSources(1).sources[0].setPitch(1).setGain(2)
		.setPosition(0, 0, 0).setVelocity(0, 0, 0).disableLooping();
	
	//3 small buffers to reduce loading time 
	//(not necessary any more for modern machines)
	al.genBuffers(3);
	Song song(load);
	for( uint i = 0; i < al.buffers.size() && !load.complete(); i++ ) {
		song.push();
		load.fillAudioBuffer();
		
		al.buffers[i].setData( AL_FORMAT_MONO16, load.audioBuffer, load.size, load.getFreq() );
		al.sources[0].attachBuffer(al.buffers[i]);
	}
	
	//start the 2nd thread to fill a larger buffer while already playing
	load.setAudioBufferSize(50 * 1048575);
	std::thread threadLoadAudio( threadLoadAudioData, std::ref(load), std::ref(song) );
	{
		std::unique_lock<std::mutex> lck( mutexLoader );
		
		song.updateUser().nextBuffer();
		al.sources[0].play();
		
		threadState = 1;
		condResumeLoader.notify_one();
	}
	
	//main loop; plays untill all file have been played
	//rotates the audio source around the listener for a certain effect
	double t = 0.f;
	ALfloat x, y, z;
	while (al.sources[0].getState() == AL_PLAYING) {
		al.sources[0].setPosition(
			1 * cos(2 * PI * t / T),
			1 * sin(2 * PI * t / T),
			0.0f
		).getPosition(&x, &y, &z);

		usleep(100000);
		printf("\rt = %02.0f:%02.0f:%04.1f ( % 4.2f % 4.2f % 4.2f ) [% 4.0f°]", 
			floor( t / (10 * 3600)), fmod(floor( t / (10*60)), 60) ,fmod(t / 10, 60), x, y, z, fmod(t, T) / T * 360
		);
		song.debugInfo();
		fflush(stdout);
		
		//when the current buffer has been played, get a new one
		//tell the 2nd thread to decode more audio
		if( al.sources[0].getProcessedBuffers() > 0 ) {
			std::unique_lock<std::mutex> lck( mutexLoader ); //threadState, load.audioBuffer/-size, song
			if( threadState == -2 ) {
				al.sources[0].attachBuffer(
					al.findBuffer(al.sources[0].detachBuffer()).setData(
						AL_FORMAT_MONO16, load.audioBuffer, load.size, load.getFreq()
					)
				);
				
				threadState = 1;
				condResumeLoader.notify_one();
			} else {
				al.sources[0].detachBuffer();
			}
			
			if( song.change() ) {
				t = 0;
			}
			song.updateSongInfo();
		}
	
		t++;
	}
	{
		std::unique_lock<std::mutex> lck( mutexLoader );
		threadState = 0;
		condResumeLoader.notify_all();
	}
	threadLoadAudio.join();

	printf("\n");
	return EXIT_SUCCESS;
}
