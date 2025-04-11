#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>

#include <stdexcept>
#include <sstream>
#include <array>

#include <string.h>
#include <assert.h>

//Classes representing important concepts from OpenAL

//Base class for error checking
class OpenALError {
	protected:
		ALint error;
		
		void resetErrorStack() { alGetError(); }
		void errorCheck(std::string errorMsg) {
			if((error = alGetError()) != AL_NO_ERROR) {
				std::stringstream msg;
				msg << errorMsg << " 0x" << std::hex << error << std::endl;
				throw std::runtime_error(msg.str());
			}
		}
};

//implementing the listener
class Listener: OpenALError {
	public:
		Listener() {};
		~Listener() {};
		
		Listener& setPosition(ALfloat x, ALfloat y, ALfloat z) {
			resetErrorStack();
			alListener3f(AL_POSITION, x, y, z);
			errorCheck("Couldn't set Listener-Position.");
			
			return *this;
		}
		Listener& setPosition(std::vector<ALfloat> pos) {
			assert(pos.size() == 3);
			resetErrorStack();
			alListenerfv(AL_POSITION, pos.data());
			errorCheck("Couldn't set Listener-Position.");
			
			return *this;
		}
		void getPosition(ALfloat* x, ALfloat* y, ALfloat* z) {
			resetErrorStack();
			alGetListener3f(AL_POSITION, x, y, z);
			errorCheck("Couldn't retrieve position.");
		}
		std::vector<ALfloat> getPosition() {
			std::vector<ALfloat> pos(3);
			resetErrorStack();
			alGetListenerfv(AL_POSITION, pos.data());
			errorCheck("Couldn't retrieve position.");
			return pos;
		}
		
		Listener& setVelocity(ALfloat vx, ALfloat vy, ALfloat vz) {
			resetErrorStack();
			alListener3f(AL_VELOCITY, vx, vy, vz);
			errorCheck("Couldn't set Listener-Velocity.");
			
			return *this;
		}
		Listener& setVelocity(std::array<ALfloat,3> v) {
			resetErrorStack();
			alListenerfv(AL_VELOCITY, v.data());
			errorCheck("Couldn't set Listener-Velocity.");
			return *this;
		}
		void getVelocity(ALfloat* x, ALfloat* y, ALfloat* z) {
			resetErrorStack();
			alGetListener3f(AL_VELOCITY, x, y, z);
			errorCheck("Couldn't retrieve velocity.");
		}
		std::array<ALfloat,3> getVelocity() {
			std::array<ALfloat,3> v;
			resetErrorStack();
			alGetListenerfv(AL_VELOCITY, v.data());
			errorCheck("Couldn't retrieve velocity.");
			
			return v;
		}
		
		
		Listener& setOrientation(std::array<ALfloat,6> ori) {
			resetErrorStack();
			alListenerfv(AL_ORIENTATION, ori.data());
			errorCheck("Couldn't set orientation.");
			
			return *this;
		}
		std::array<ALfloat,6> getOrientation() {
			std::array<ALfloat,6> ori;
			resetErrorStack();
			alGetListenerfv(AL_ORIENTATION, ori.data());
			errorCheck("Couldn't retrieve orientation.");
			
			return ori;
		}
};

//implementing a buffer
class Buffer: OpenALError {
	private:
	
	public:
		ALuint buffer;
		
		void genBuffer() {
			resetErrorStack();
			alGenBuffers(1, &buffer);
			errorCheck("Coudln't create buffer");
		}
		~Buffer() {
			alDeleteBuffers(1, &buffer);
		}
		
		Buffer& setData(ALenum format, ALvoid* data, ALsizei size, ALsizei freq) {
			resetErrorStack();
			alBufferData(buffer, format, data, size, freq);
			errorCheck("Coudln't load data to buffer.");
			
			return *this;
		}
};

//implementing the source
class Source: OpenALError {
	protected:
		
	public:
		ALuint source;
		
		void genSource() {
			resetErrorStack();
			alGenSources(1, &source);
			errorCheck("Couldn't create source");
		}
		~Source() {
			alDeleteSources(1, &source);
		}
		
		Source& setPitch(ALfloat pitch) {
			resetErrorStack();
			alSourcef(source, AL_PITCH, pitch);
			errorCheck("Couldn't set pitch");
			
			return *this;
		}
		ALfloat getPitch() {
			ALfloat pitch;
			resetErrorStack();
			alGetSourcef(source, AL_PITCH, &pitch);
			errorCheck("Couldn't retrieve pitch.");
			
			return pitch;
		}
		
		Source& setGain(ALfloat gain) {
			resetErrorStack();
			alSourcef(source, AL_GAIN, gain);
			errorCheck("Couldn't set source-gain.");
			
			return *this;
		}
		ALfloat getGain() {
			ALfloat gain;
			resetErrorStack();
			alGetSourcef(source, AL_GAIN, &gain);
			errorCheck("Couldn't get source-gain.");
			
			return gain;
		}
		
		Source& setPosition(ALfloat x, ALfloat y, ALfloat z) {
			resetErrorStack();
			alSource3f(source, AL_POSITION, x, y, z);
			errorCheck("Couldn't set source-Position.");
			
			return *this;
		}
		Source& setPosition(std::array<ALfloat,3> pos) {
			resetErrorStack();
			alSourcefv(source, AL_POSITION, pos.data());
			errorCheck("Couldn't set source-Position.");
			
			return *this;
		}
		Source& getPosition(ALfloat* x, ALfloat* y, ALfloat* z) {
			resetErrorStack();
			alGetSource3f(source, AL_POSITION, x, y, z);
			errorCheck("Couldn't retrieve position.");
			
			return *this;
		}
		std::array<ALfloat,3> getPosition() {
			std::array<ALfloat,3> pos;
			resetErrorStack();
			alGetSourcefv(source, AL_POSITION, pos.data());
			errorCheck("Couldn't retrieve position.");
			return pos;
		}
		
		Source& setVelocity(ALfloat vx, ALfloat vy, ALfloat vz) {
			resetErrorStack();
			alSource3f(source, AL_VELOCITY, vx, vy, vz);
			errorCheck("Couldn't set source-Velocity.");
			
			return *this;
		}
		Source& setVelocity(std::array<ALfloat,3> v) {
			resetErrorStack();
			alSourcefv(source, AL_VELOCITY, v.data());
			errorCheck("Couldn't set source-Velocity.");
			
			return *this;
		}
		Source& getVelocity(ALfloat* x, ALfloat* y, ALfloat* z) {
			resetErrorStack();
			alGetSource3f(source, AL_VELOCITY, x, y, z);
			errorCheck("Couldn't retrieve velocity.");
			
			return *this;
		}
		std::array<ALfloat,3> getVelocity() {
			std::array<ALfloat,3> v;
			resetErrorStack();
			alGetSourcefv(source, AL_VELOCITY, v.data());
			errorCheck("Couldn't retrieve velocity.");
			
			return v;
		}
		
		Source& setLooping(ALint loop) {
			resetErrorStack();
			alSourcei(source, AL_LOOPING, loop);
			errorCheck("Couldn't set source-looping.");
			
			return *this;
		}
		ALint getLooping() {
			ALint loop;
			resetErrorStack();
			alGetSourcei(source, AL_LOOPING, &loop);
			errorCheck("Couldn't retrieve source-looping.");
			
			return loop;
		}
		Source& enableLooping() {
			resetErrorStack();
			alSourcei(source, AL_LOOPING, AL_TRUE);
			errorCheck("Couldn't retrieve source-looping.");
			
			return *this;
		}
		Source& disableLooping() {
			resetErrorStack();
			alSourcei(source, AL_LOOPING, AL_FALSE);
			errorCheck("Couldn't retrieve source-looping.");
			
			return *this;
		}
		
		ALint getState() {
			ALint sourceState;
			resetErrorStack();
			alGetSourcei(source, AL_SOURCE_STATE, &sourceState);
			errorCheck("Couldn't retrieve source-state");
			
			return sourceState;
		}
		
		Source& play() {
			alSourcePlay(source);
			
			return *this;
		}
		
		Source& setBuffer( Buffer buf ) {
			resetErrorStack();
			alSourcei( source, AL_BUFFER, buf.buffer );
			errorCheck("Couldn't attach buffer to source.");
			
			return *this;
		}
		
		Source& attachBuffer( Buffer buf ) {
			resetErrorStack();
			alSourceQueueBuffers(source, 1, &buf.buffer);
			errorCheck("Couldn't attach buffer to source.");
			
			return *this;
		}
		Source& attachBuffers( std::vector<Buffer> attachBuffs ) {
			std::vector<ALuint> bufs(attachBuffs.size());
			for(ulong i = 0; i < attachBuffs.size(); i++) {
				bufs[i] = attachBuffs[i].buffer;
			}
			
			resetErrorStack();
			alSourceQueueBuffers(source, bufs.size(), bufs.data());
			errorCheck("Couldn't attach buffers to source.");
			
			return *this;			
		}
		
		ALint getAttachedBuffers() {
			ALint num;
			resetErrorStack();
			alGetSourcei( source, AL_BUFFERS_QUEUED, &num );
			errorCheck("Couldn't fetch number of attached buffers");
			
			return num;
		}
		ALint getProcessedBuffers() {
			ALint num;
			resetErrorStack();
			alGetSourcei( source, AL_BUFFERS_PROCESSED, &num );
			errorCheck("Couldn't fetch number of processed buffers.");
			
			return num;
		}
		std::vector<ALuint> detachBuffers(int n) {
			std::vector<ALuint> bufs(n);
			resetErrorStack();
			alSourceUnqueueBuffers( source, n, bufs.data() );
			errorCheck( "Couldn't detach buffers." );
			
			return bufs;
		}
		ALuint detachBuffer() {
			ALuint detached;
			resetErrorStack();
			alSourceUnqueueBuffers( source, 1, &detached );
			errorCheck("Couldn't detach buffer.");
			
			return detached;
		}
};

//very basic class for global info, and some book-keeping
class OpenAL: OpenALError {
	private:
		ALCdevice* device;
		ALCcontext* context;
		ALint error;
		
	public:
		std::vector<Source> sources;
		std::vector<Buffer> buffers;
		Listener listener;
		
		LPALGENEFFECTS alGenEffects;
		LPALDELETEEFFECTS alDeleteEffects;
		LPALISEFFECT alIsEffect;
		
		Listener& getListener() {
			return listener;
		}
	
	
		std::vector<Source> getSources() {
			return sources;
		}
	
		std::vector<Buffer> getBuffers() {
			return buffers;
		}
	
		OpenAL& genSources(int num) {
			uint sizes = sources.size();
			sources.resize( sizes + num);
			for(uint i = sizes; i < sizes + num; i++) {
				sources[i].genSource();
			}
			
			return *this;
		}
		
		OpenAL& genBuffers(int num) {
			uint sizes = buffers.size();
			buffers.resize( sizes + num);
			for(uint i = sizes; i < sizes + num; i++) {
				buffers[i].genBuffer();
			}
			
			return *this;
		}
		
		Buffer& findBuffer(ALuint buf) {
			for( uint i = 0; i < buffers.size(); i++ ) {
				if( buffers[i].buffer == buf ) {
					return buffers[i];
				}
			}
			throw std::runtime_error("Couldn't find corresponding buffer.");
		}
		
		OpenAL& getEFXFuncPointers() {
			alGenEffects 	= (LPALGENEFFECTS) 		alGetProcAddress("alGenEffects");
			alDeleteEffects = (LPALDELETEEFFECTS) 	alGetProcAddress("alDeleteEffects");
			alIsEffect 		= (LPALISEFFECT) 		alGetProcAddress("alIsEffect");
			
			if( !(alGenEffects && alDeleteEffects && alIsEffect)) {
				throw std::runtime_error("Didn't found EFX-funktion pointers.");
			}
			
			return *this;
		}
	
		OpenAL& makeCurrent() {
			resetErrorStack();
			alcMakeContextCurrent( context );
			errorCheck("Couldn't make context current.");
			
			return *this;
		}
	
		OpenAL& createContext() {
			std::vector<ALint> auxSends{ALC_MAX_AUXILIARY_SENDS, 4,0,0};
			resetErrorStack();
			context = alcCreateContext( device, auxSends.data() );
			if(!context) {
				throw std::runtime_error("Couldn't create context.");
			}
			#ifdef DEBUG
			ALCint actualAuxSends;
			alcGetIntegerv(device, ALC_MAX_AUXILIARY_SENDS, 1, &actualAuxSends);
			std::cout << "Found " << actualAuxSends << " auxilary sends per source"
				<< std::endl;
			#endif
			
			return *this;
		}
	
		OpenAL() {
			device = alcOpenDevice( NULL );
			if(!device) {
				throw std::runtime_error("Couldn't find suitable device.");
			}	
			sources.clear();
			buffers.clear();
			
			#ifdef DEBUG
			if (alcIsExtensionPresent(device, "ALC_EXT_EFX") == AL_FALSE) {
				std::cerr << "Didn't found EFX-Extension!" << std::endl;
			} else {
				std::cout << "Found EFX-Extension." << std::endl;
			}
			#endif
		}
		OpenAL(std::string name) {
			device = alcOpenDevice( name.c_str() );
			if( !device ) {
				throw std::runtime_error("Couldn't open device.");
			}
			sources.clear();
			buffers.clear();
		}
		
		~OpenAL() {
			//~ device = alcGetContextsDevice(context);
			//~ alcMakeContextCurrent(NULL);
			if( context ) {
				alcDestroyContext(context);
			}
			if( device ) {
				alcCloseDevice(device);
			}
		}
};
