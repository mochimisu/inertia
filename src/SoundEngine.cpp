#include "SoundEngine.h"

SoundEngine::SoundEngine() {
}


void SoundEngine::start() {
		alSourcePlay(currentMusic);
}

void SoundEngine::playSelectSource1() {
		alSourcePlay(selectSource1);
}

void SoundEngine::playSelectSource2() {
		alSourcePlay(selectSource2);
}

void SoundEngine::playEngineSound() {
		ALint sourceState;
		alGetSourcei(currentNoise, AL_SOURCE_STATE, &sourceState);
		if (sourceState != AL_PLAYING) {
				alSourcePlay(currentNoise);
		}
}

void SoundEngine::stopEngineSound() {
		alSourceStop(currentNoise);
}

void SoundEngine::playDeathSound() { 
		alSourcePlay(currentDeath);
}

ALboolean SoundEngine::LoadALData()
{
		// Variables to load into.

		ALenum format;
		ALsizei size;
		ALvoid* data;
		ALsizei freq;
		ALboolean loop;
		// Load wav data into a buffer.
		alGenBuffers(1, &musicBuffer);
		if (alGetError() != AL_NO_ERROR)
				return AL_FALSE;

		alutLoadWAVFile("resources/sounds/race2.wav", &format, &data, &size, &freq, &loop);
		alBufferData(musicBuffer, format, data, size, freq);
		alutUnloadWAV(format, data, size, freq);
		// Bind buffer with a source.
		alGenSources(1, &musicSource);

		if (alGetError() != AL_NO_ERROR)
				return AL_FALSE;

		alSourcei (musicSource, AL_BUFFER,   musicBuffer   );
		alSourcef (musicSource, AL_PITCH,    1.0f     );
		alSourcef (musicSource, AL_GAIN,     1.0f     );
		alSourcefv(musicSource, AL_POSITION, SourcePos);
		alSourcefv(musicSource, AL_VELOCITY, SourceVel);
		alSourcei (musicSource, AL_LOOPING,  AL_TRUE     );
		// Do another error check and return.
		if (alGetError() == AL_NO_ERROR)
				return AL_TRUE;

		return AL_FALSE;
}

ALboolean SoundEngine::LoadALData2()
{
		// Variables to load into.

		ALenum format;
		ALsizei size;
		ALvoid* data;
		ALsizei freq;
		ALboolean loop;
		// Load wav data into a buffer.
		alGenBuffers(1, &noiseBuffer);
		if (alGetError() != AL_NO_ERROR)
				return AL_FALSE;

		alutLoadWAVFile("resources/sounds/Engine.wav", &format, &data, &size, &freq, &loop);
		alBufferData(noiseBuffer, format, data, size, freq);
		alutUnloadWAV(format, data, size, freq);
		// Bind buffer with a source.
		alGenSources(1, &noiseSource);

		if (alGetError() != AL_NO_ERROR)
				return AL_FALSE;

		alSourcei (noiseSource, AL_BUFFER,   noiseBuffer   );
		cout << alGetString(alGetError()) << endl;
		alSourcef (noiseSource, AL_PITCH,    1.0f     );
		alSourcef (noiseSource, AL_GAIN,     1.0f     );
		alSourcefv(noiseSource, AL_POSITION, SourcePos);
		alSourcefv(noiseSource, AL_VELOCITY, SourceVel);
		alSourcei (noiseSource, AL_LOOPING,  AL_TRUE     );
		// Do another error check and return.
		if (alGetError() == AL_NO_ERROR)
				return AL_TRUE;

		return AL_FALSE;
}

ALboolean SoundEngine::LoadALData3()
{
		// Variables to load into.

		ALenum format;
		ALsizei size;
		ALvoid* data;
		ALsizei freq;
		ALboolean loop;
		// Load wav data into a buffer.
		alGenBuffers(1, &musicBuffer2);
		if (alGetError() != AL_NO_ERROR)
				return AL_FALSE;

		alutLoadWAVFile("resources/sounds/pacman.wav", &format, &data, &size, &freq, &loop);
		alBufferData(musicBuffer2, format, data, size, freq);
		alutUnloadWAV(format, data, size, freq);
		// Bind buffer with a source.
		alGenSources(1, &musicSource2);

		if (alGetError() != AL_NO_ERROR)
				return AL_FALSE;

		alSourcei (musicSource2, AL_BUFFER,   musicBuffer2   );
		alSourcef (musicSource2, AL_PITCH,    1.0f     );
		alSourcef (musicSource2, AL_GAIN,     1.0f     );
		alSourcefv(musicSource2, AL_POSITION, SourcePos);
		alSourcefv(musicSource2, AL_VELOCITY, SourceVel);
		alSourcei (musicSource2, AL_LOOPING,  AL_FALSE     );
		// Do another error check and return.
		if (alGetError() == AL_NO_ERROR)
				return AL_TRUE;

		return AL_FALSE;
}

ALboolean SoundEngine::LoadALData4()
{
		// Variables to load into.

		ALenum format;
		ALsizei size;
		ALvoid* data;
		ALsizei freq;
		ALboolean loop;
		// Load wav data into a buffer.
		alGenBuffers(1, &noiseBuffer2);
		if (alGetError() != AL_NO_ERROR)
				return AL_FALSE;

		alutLoadWAVFile("resources/sounds/wakka.wav", &format, &data, &size, &freq, &loop);
		alBufferData(noiseBuffer2, format, data, size, freq);
		alutUnloadWAV(format, data, size, freq);
		// Bind buffer with a source.
		alGenSources(1, &noiseSource2);

		if (alGetError() != AL_NO_ERROR)
				return AL_FALSE;

		alSourcei (noiseSource2, AL_BUFFER,   noiseBuffer2   );
		alSourcef (noiseSource2, AL_PITCH,    1.0f     );
		alSourcef (noiseSource2, AL_GAIN,     1.0f     );
		alSourcefv(noiseSource2, AL_POSITION, SourcePos);
		alSourcefv(noiseSource2, AL_VELOCITY, SourceVel);
		alSourcei (noiseSource2, AL_LOOPING,  AL_TRUE     );
		// Do another error check and return.
		if (alGetError() == AL_NO_ERROR)
				return AL_TRUE;

		return AL_FALSE;
}

ALboolean SoundEngine::LoadALData5()
{
		// Variables to load into.

		ALenum format;
		ALsizei size;
		ALvoid* data;
		ALsizei freq;
		ALboolean loop;
		// Load wav data into a buffer.
		alGenBuffers(1, &deathBuffer);
		if (alGetError() != AL_NO_ERROR)
				return AL_FALSE;

		alutLoadWAVFile("resources/sounds/death.wav", &format, &data, &size, &freq, &loop);
		alBufferData(deathBuffer, format, data, size, freq);
		alutUnloadWAV(format, data, size, freq);
		// Bind buffer with a source.
		alGenSources(1, &deathSource);
		if (alGetError() != AL_NO_ERROR) {
				return AL_FALSE;
		}

		alSourcei (deathSource, AL_BUFFER,   deathBuffer   );
		alSourcef (deathSource, AL_PITCH,    1.0f     );
		alSourcef (deathSource, AL_GAIN,     1.0f     );
		alSourcefv(deathSource, AL_POSITION, SourcePos);
		alSourcefv(deathSource, AL_VELOCITY, SourceVel);
		alSourcei (deathSource, AL_LOOPING,  AL_FALSE     );
		// Do another error check and return.
		if (alGetError() == AL_NO_ERROR) {
				return AL_TRUE;
		}

		return AL_FALSE;
}

ALboolean SoundEngine::LoadALData6()
{
		// Variables to load into.

		ALenum format;
		ALsizei size;
		ALvoid* data;
		ALsizei freq;
		ALboolean loop;
		// Load wav data into a buffer.
		alGenBuffers(1, &deathBuffer2);
		if (alGetError() != AL_NO_ERROR)
				return AL_FALSE;

		alutLoadWAVFile("resources/sounds/pacdeath.wav", &format, &data, &size, &freq, &loop);
		alBufferData(deathBuffer2, format, data, size, freq);
		alutUnloadWAV(format, data, size, freq);
		// Bind buffer with a source.
		alGenSources(1, &deathSource2);

		if (alGetError() != AL_NO_ERROR)
				return AL_FALSE;

		alSourcei (deathSource2, AL_BUFFER,   deathBuffer2   );
		alSourcef (deathSource2, AL_PITCH,    1.0f     );
		alSourcef (deathSource2, AL_GAIN,     1.0f     );
		alSourcefv(deathSource2, AL_POSITION, SourcePos);
		alSourcefv(deathSource2, AL_VELOCITY, SourceVel);
		alSourcei (deathSource2, AL_LOOPING,  AL_FALSE     );
		// Do another error check and return.
		if (alGetError() == AL_NO_ERROR)
				return AL_TRUE;

		return AL_FALSE;
}

ALboolean SoundEngine::LoadALDataSelect1()
{
		// Variables to load into.

		ALenum format;
		ALsizei size;
		ALvoid* data;
		ALsizei freq;
		ALboolean loop;
		// Load wav data into a buffer.
		alGenBuffers(1, &selectBuffer1);
		if (alGetError() != AL_NO_ERROR)
				return AL_FALSE;

		alutLoadWAVFile("resources/sounds/select1.wav", &format, &data, &size, &freq, &loop);
		alBufferData(selectBuffer1, format, data, size, freq);
		alutUnloadWAV(format, data, size, freq);
		// Bind buffer with a source.
		alGenSources(1, &selectSource1);

		if (alGetError() != AL_NO_ERROR)
				return AL_FALSE;

		alSourcei (selectSource1, AL_BUFFER,   selectBuffer1   );
		alSourcef (selectSource1, AL_PITCH,    1.0f     );
		alSourcef (selectSource1, AL_GAIN,     1.0f     );
		alSourcefv(selectSource1, AL_POSITION, SourcePos);
		alSourcefv(selectSource1, AL_VELOCITY, SourceVel);
		alSourcei (selectSource1, AL_LOOPING,  AL_FALSE     );
		// Do another error check and return.
		if (alGetError() == AL_NO_ERROR)
				return AL_TRUE;

		return AL_FALSE;
}

ALboolean SoundEngine::LoadALDataSelect2()
{
		// Variables to load into.

		ALenum format;
		ALsizei size;
		ALvoid* data;
		ALsizei freq;
		ALboolean loop;
		// Load wav data into a buffer.
		alGenBuffers(1, &selectBuffer2);
		if (alGetError() != AL_NO_ERROR)
				return AL_FALSE;

		alutLoadWAVFile("resources/sounds/select2.wav", &format, &data, &size, &freq, &loop);
		alBufferData(selectBuffer2, format, data, size, freq);
		alutUnloadWAV(format, data, size, freq);
		// Bind buffer with a source.
		alGenSources(1, &selectSource2);

		if (alGetError() != AL_NO_ERROR)
				return AL_FALSE;

		alSourcei (selectSource2, AL_BUFFER,   selectBuffer2   );
		alSourcef (selectSource2, AL_PITCH,    1.0f     );
		alSourcef (selectSource2, AL_GAIN,     1.0f     );
		alSourcefv(selectSource2, AL_POSITION, SourcePos);
		alSourcefv(selectSource2, AL_VELOCITY, SourceVel);
		alSourcei (selectSource2, AL_LOOPING,  AL_FALSE     );
		// Do another error check and return.
		if (alGetError() == AL_NO_ERROR)
				return AL_TRUE;

		return AL_FALSE;
}

void SoundEngine::SetListenerValues()
{
		alListenerfv(AL_POSITION,    ListenerPos);
		alListenerfv(AL_VELOCITY,    ListenerVel);
		alListenerfv(AL_ORIENTATION, ListenerOri);
}

void SoundEngine::KillALData()
{
		alDeleteBuffers(1, &noiseBuffer);
		alDeleteSources(1, &noiseSource);
		alDeleteBuffers(1, &musicBuffer);
		alDeleteSources(1, &musicSource);
		alDeleteSources(1, &musicSource2);
		alDeleteBuffers(1, &musicBuffer2);
		alDeleteSources(1, &noiseSource2);
		alDeleteBuffers(1, &noiseBuffer2);
		alDeleteSources(1, &selectSource1);
		alDeleteBuffers(1, &selectBuffer1);
		alDeleteSources(1, &selectSource2);
		alDeleteBuffers(1, &selectBuffer2);
		alutExit();
}


int SoundEngine::initialise()
{
		//Initialize OpenAL
		char * nothing = "";
		int argc = 0;
		alutInit(&argc,&nothing);


		alGetError(); // zero-out the error status

		// Position of the source sound.
		ALfloat SourcePos[] = { 0.0, 0.0, 0.0 };

		// Velocity of the source sound.
		ALfloat SourceVel[] = { 0.0, 0.0, 0.0 };

		// Position of the listener.
		ALfloat ListenerPos[] = { 0.0, 0.0, 0.0 };

		// Velocity of the listener.
		ALfloat ListenerVel[] = { 0.0, 0.0, 0.0 };

		// Orientation of the listener. (first 3 elements are "at", second 3 are "up")
		ALfloat ListenerOri[] = { 0.0, 0.0, -1.0,  0.0, 1.0, 0.0 };


		// Position of the source sound.
		this->SourcePos = SourcePos;

		// Velocity of the source sound.
		this->SourceVel = SourceVel;

		// Position of the listener.
		this->ListenerPos = ListenerPos;

		// Velocity of the listener.
		this->ListenerVel = ListenerVel;

		// Orientation of the listener. (first 3 elements are "at", second 3 are "up")
		this->ListenerOri = ListenerOri;

		if (LoadALData() == AL_FALSE)
				return -1;
		SetListenerValues();
		if (LoadALData2() == AL_FALSE)
				return -2;
		SetListenerValues();
		if (LoadALData3() == AL_FALSE)
				return -3;
		SetListenerValues();
		if (LoadALData4() == AL_FALSE)
				return -4;
		SetListenerValues();
		if (LoadALData6() == AL_FALSE)
				return -6;
		SetListenerValues();
		if (LoadALData5() == AL_FALSE)
				return -5;
		if (LoadALDataSelect1() == AL_FALSE)
				return -7;
		if (LoadALDataSelect2() == AL_FALSE)
				return -7;
		SetListenerValues();

		// Setup an exit procedure.
		//atexit(KillALData);
		currentMusic = musicSource;
		currentNoise = noiseSource;
		currentDeath = deathSource;
}
