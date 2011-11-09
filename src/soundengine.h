#ifndef SOUNDENGINE_H_
#define SOUNDENGINE_H_

#include "global.h"


class SoundEngine {
		public:
				SoundEngine();
				ALboolean LoadALData();
				ALboolean LoadALData2();
				ALboolean LoadALData3();
				ALboolean LoadALData4();
				ALboolean LoadALData5();
				ALboolean LoadALData6();
				ALboolean LoadALDataSelect1();
				ALboolean LoadALDataSelect2();
				void SetListenerValues();
				void KillALData();
				int initialise();
				void playSelectSource1();
				void playSelectSource2();
				void playEngineSound();
				void stopEngineSound();
				void playDeathSound();
				void start();
		private:
				// Buffers hold sound data.
				ALuint noiseBuffer;
				ALuint musicBuffer;
				ALuint deathBuffer;
				ALuint noiseBuffer2;
				ALuint musicBuffer2;
				ALuint deathBuffer2;
				ALuint selectBuffer1;
				ALuint selectBuffer2;

				// Sources are points emitting sound.
				ALuint noiseSource;
				ALuint musicSource;
				ALuint deathSource;
				ALuint noiseSource2;
				ALuint musicSource2;
				ALuint deathSource2;
				ALuint selectSource1;
				ALuint selectSource2;

				// For switching soundtracks
				ALuint currentMusic;
				ALuint currentNoise;
				ALuint currentDeath;

				// Position of the source sound.
				ALfloat * SourcePos;

				// Velocity of the source sound.
				ALfloat * SourceVel;

				// Position of the listener.
				ALfloat * ListenerPos;

				// Velocity of the listener.
				ALfloat * ListenerVel;

				// Orientation of the listener. (first 3 elements are "at", second 3 are "up")
				ALfloat * ListenerOri;
};

#endif
