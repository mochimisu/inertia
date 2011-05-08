#include "main.h"
#include "functions.h"
#include "GameMode.h"
// Buffers hold sound data.
ALuint noiseBuffer;
ALuint musicBuffer;

// Sources are points emitting sound.
ALuint noiseSource;
ALuint musicSource;

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


//===WINDOW PROPERTIES
Viewport viewport;
ALboolean LoadALData()
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

    alutLoadWAVFile("race2.wav", &format, &data, &size, &freq, &loop);
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

ALboolean LoadALData2()
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

    alutLoadWAVFile("White_Noise.wav", &format, &data, &size, &freq, &loop);
    alBufferData(noiseBuffer, format, data, size, freq);
    alutUnloadWAV(format, data, size, freq);
    // Bind buffer with a source.
    alGenSources(1, &noiseSource);

    if (alGetError() != AL_NO_ERROR)
        return AL_FALSE;

    alSourcei (noiseSource, AL_BUFFER,   noiseBuffer   );
    cout << alGetString(alGetError()) << endl;
    alSourcef (noiseSource, AL_PITCH,    1.0f     );
    alSourcef (noiseSource, AL_GAIN,     0.5f     );
    alSourcefv(noiseSource, AL_POSITION, SourcePos);
    alSourcefv(noiseSource, AL_VELOCITY, SourceVel);
    alSourcei (noiseSource, AL_LOOPING,  AL_TRUE     );
    // Do another error check and return.
    if (alGetError() == AL_NO_ERROR)
        return AL_TRUE;

    return AL_FALSE;
}

void SetListenerValues()
{
    alListenerfv(AL_POSITION,    ListenerPos);
    alListenerfv(AL_VELOCITY,    ListenerVel);
    alListenerfv(AL_ORIENTATION, ListenerOri);
}

void KillALData()
{
    alDeleteBuffers(2, &noiseBuffer);
    alDeleteSources(2, &noiseSource);
    alDeleteBuffers(1, &musicBuffer);
    alDeleteSources(1, &musicSource);
    alutExit();
}


// Game Modes
extern GameMode raceMode;

/*
 * Main
 */
int main(int argc,char** argv) {
  //Initialize OpenGL
  glutInit(&argc, argv);

  //Initialize OpenAL
  alutInit(&argc, argv);
  alGetError(); // zero-out the error status
  // Load the wav data.
  if (LoadALData() == AL_FALSE)
    return -1;
  SetListenerValues();
  if (LoadALData2() == AL_FALSE)
    return -2;

  SetListenerValues();

  // Setup an exit procedure.
  atexit(KillALData);


  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);
  viewport.w = 1024;
  viewport.h = 768;
  glutInitWindowSize(viewport.w,viewport.h);
  glutInitWindowPosition(0,0);
  glutCreateWindow("Inertia Alpha");
  
  glewInit();

  raceMode.activate();

  FreeImage_Initialise();

  //And Go!
  alSourcePlay(musicSource);
  glutMainLoop();
}
