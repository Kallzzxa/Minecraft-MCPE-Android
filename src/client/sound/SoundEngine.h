#ifndef NET_MINECRAFT_CLIENT_SOUND__SoundEngine_H__
#define NET_MINECRAFT_CLIENT_SOUND__SoundEngine_H__

//package net.minecraft.client.sound;

#if defined(ANDROID) && !defined(PRE_ANDROID23)
	#include "../../platform/audio/SoundSystemSL.h"
#elif defined(__APPLE__)
    #include "../../platform/audio/SoundSystemAL.h"
#elif defined(_WIN32)
    #include "../../platform/audio/SoundSystemFMOD.h"
#else
	#include "../../platform/audio/SoundSystem.h"
#endif
#include "SoundRepository.h"
#include "../../util/Random.h"

class Minecraft;
class Mob;
class Options;

class SoundEngine
{
    static const int SOUND_DISTANCE = 16;

	#if defined(ANDROID) && !defined(PRE_ANDROID23) && !defined(RPI)
		SoundSystemSL soundSystem;
    #elif defined(__APPLE__)
        SoundSystemAL soundSystem;
    #elif defined(_WIN32)
        SoundSystemFMOD soundSystem;
	#else
	    SoundSystem soundSystem;
	#endif

	Options* options;
	int idCounter;
	//static bool loaded;
	Random random;
	//int noMusicDelay = random.nextInt(20 * 60 * 10);
	float _x;
	float _y;
	float _z;
    float _yRot;
	float _invMaxDistance;

public:
	SoundEngine(float maxDistance);

	~SoundEngine();

    void init(Minecraft* mc, Options* options);
	void destroy();

	void enable(bool status);

	void updateOptions();
    void update(Mob* player, float a);

	void play(const std::string& name, float x, float y, float z, float volume, float pitch);
	void playUI(const std::string& name, float volume, float pitch);

	float _getVolumeMult(float x, float y, float z);
private:
	// v19: procedural music + ambience. No music/ambience audio files
	// exist in the tree (all 102 .pcm files are short SFX, <= ~1.5 s),
	// so the tracks below are synthesised in code (mono 16-bit PCM)
	// and played as one-shot buffers through the existing OpenSL path.
	void _generateMusic();
	void _updateMusic(Mob* player);
	void _playMusic(const SoundDesc& sound, float volume);
	SoundDesc _makeMusicTrack(int seed);
	SoundDesc _makeChirp(bool cricket);

	static const int MUSIC_RATE = 22050;
	static const int MUSIC_TRACK_SECONDS = 18;
	// update() runs once per rendered frame (~60 fps); the scheduler
	// counts frames, so these are approximate, not wall-clock exact.
	static const int MUSIC_TRACK_FRAMES = MUSIC_TRACK_SECONDS * 60;

	bool musicReady;
	SoundDesc musicTracks[2];
	SoundDesc birdChirp;
	SoundDesc cricketChirp;
	int musicIdleFrames;		// frames since the last track ended
	int musicGapFrames;			// frames to wait before the next track
	int musicTrackFramesLeft;	// frames left of the playing track
	int ambienceIdleFrames;
	int ambienceGapFrames;
	void loadLibrary() {}
    SoundDesc _pp(const std::string& fn);
    
	SoundRepository sounds;
	Minecraft* mc;
};

#endif /*NET_MINECRAFT_CLIENT_SOUND__SoundEngine_H__*/
