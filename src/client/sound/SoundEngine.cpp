#include "SoundEngine.h"
#include "../Options.h"
#include "../Minecraft.h"
#include "../../world/entity/Mob.h"
#include "../../world/level/Level.h"
#include <math.h>


SoundEngine::SoundEngine( float maxDistance )
:	idCounter(0),
	mc(0),
	_x(0),
	_y(0),
	_z(0),
	_yRot(0),
	_invMaxDistance(1.0f / maxDistance),
	musicReady(false),
	musicIdleFrames(0),
	musicGapFrames(10 * 60),
	musicTrackFramesLeft(0),
	ambienceIdleFrames(0),
	ambienceGapFrames(15 * 60)
{

}

SoundEngine::~SoundEngine()
{

}

void SoundEngine::init( Minecraft* mc, Options* options )
{
	this->mc = mc;
	this->options = options;

	if (/*!loaded && */(options == NULL || (options->sound != 0 || options->music != 0))) {
		loadLibrary();
	}

#if !defined(PRE_ANDROID23) && !defined(__APPLE__) && !defined(RPI)
	sounds.add("step.cloth", SA_cloth1);
	sounds.add("step.cloth", SA_cloth2);
	sounds.add("step.cloth", SA_cloth3);
	sounds.add("step.cloth", SA_cloth4);
	sounds.add("step.grass", SA_grass1);
	sounds.add("step.grass", SA_grass2);
	sounds.add("step.grass", SA_grass3);
	sounds.add("step.grass", SA_grass4);
	sounds.add("step.gravel", SA_gravel1);
	sounds.add("step.gravel", SA_gravel2);
	sounds.add("step.gravel", SA_gravel3);
	sounds.add("step.gravel", SA_gravel4);
	sounds.add("step.sand", SA_sand1);
	sounds.add("step.sand", SA_sand2);
	sounds.add("step.sand", SA_sand3);
	sounds.add("step.sand", SA_sand4);
	sounds.add("step.stone", SA_stone1);
	sounds.add("step.stone", SA_stone2);
	sounds.add("step.stone", SA_stone3);
	sounds.add("step.stone", SA_stone4);
	sounds.add("step.wood", SA_wood1);
	sounds.add("step.wood", SA_wood2);
	sounds.add("step.wood", SA_wood3);
	sounds.add("step.wood", SA_wood4);

	sounds.add("random.splash", SA_splash);
	sounds.add("random.explode", SA_explode);
	sounds.add("random.click", SA_click);

	sounds.add("random.door_open", SA_door_open);
	sounds.add("random.door_close", SA_door_close);
	sounds.add("random.pop", SA_pop);
	sounds.add("random.pop2", SA_pop2);
	sounds.add("random.hurt", SA_hurt);
	sounds.add("random.glass", SA_glass1);
	sounds.add("random.glass", SA_glass2);
	sounds.add("random.glass", SA_glass3);

	sounds.add("mob.sheep", SA_sheep1);
	sounds.add("mob.sheep", SA_sheep2);
	sounds.add("mob.sheep", SA_sheep3);
	sounds.add("mob.pig", SA_pig1);
	sounds.add("mob.pig", SA_pig2);
	sounds.add("mob.pig", SA_pig3);
	sounds.add("mob.pigdeath", SA_pigdeath);

	sounds.add("mob.cow", SA_cow1);
	sounds.add("mob.cow", SA_cow2);
	sounds.add("mob.cow", SA_cow3);
	sounds.add("mob.cow", SA_cow4);
	sounds.add("mob.cowhurt", SA_cowhurt1);
	sounds.add("mob.cowhurt", SA_cowhurt2);
	sounds.add("mob.cowhurt", SA_cowhurt3);

	sounds.add("mob.chicken", SA_chicken1);
	sounds.add("mob.chicken", SA_chicken2);
	sounds.add("mob.chicken", SA_chicken3);
	sounds.add("mob.chickenhurt", SA_chickenhurt1);
	sounds.add("mob.chickenhurt", SA_chickenhurt2);
	// v19: egg-lay sound; the file existed but was never compiled in
	// (include + definition were commented out) and its call site in
	// Chicken::tick was commented out too (re-enabled there).
	sounds.add("mob.chickenplop", SA_chickenplop);

	sounds.add("mob.zombie", SA_zombie1);
	sounds.add("mob.zombie", SA_zombie2);
	sounds.add("mob.zombie", SA_zombie3); 
	sounds.add("mob.zombiedeath", SA_zombiedeath);
	sounds.add("mob.zombiehurt", SA_zombiehurt1);
	sounds.add("mob.zombiehurt", SA_zombiehurt2);

	sounds.add("mob.skeleton", SA_skeleton1);
	sounds.add("mob.skeleton", SA_skeleton2);
	sounds.add("mob.skeleton", SA_skeleton3);
	sounds.add("mob.skeletonhurt", SA_skeletonhurt1);
	sounds.add("mob.skeletonhurt", SA_skeletonhurt2);
	sounds.add("mob.skeletonhurt", SA_skeletonhurt3);
	sounds.add("mob.skeletonhurt", SA_skeletonhurt4);
	// v19: the file + SoundDesc existed but the name was never
	// registered, and Skeleton::getDeathSound returned skeletonhurt,
	// so the skeleton death sound never played. Now registered and
	// returned by Skeleton::getDeathSound.
	sounds.add("mob.skeletondeath", SA_skeletondeath);

	sounds.add("mob.spider", SA_spider1);
	sounds.add("mob.spider", SA_spider2);
	sounds.add("mob.spider", SA_spider3);
	sounds.add("mob.spider", SA_spider4);
	sounds.add("mob.spiderdeath", SA_spiderdeath);

	sounds.add("mob.zombiepig.zpig", SA_zpig1);
	sounds.add("mob.zombiepig.zpig", SA_zpig2);
	sounds.add("mob.zombiepig.zpig", SA_zpig3);
	sounds.add("mob.zombiepig.zpig", SA_zpig4);
	sounds.add("mob.zombiepig.zpigangry", SA_zpigangry1);
	sounds.add("mob.zombiepig.zpigangry", SA_zpigangry2);
	sounds.add("mob.zombiepig.zpigangry", SA_zpigangry3);
	sounds.add("mob.zombiepig.zpigangry", SA_zpigangry4);
	sounds.add("mob.zombiepig.zpigdeath", SA_zpigdeath);
	sounds.add("mob.zombiepig.zpighurt", SA_zpighurt1);
	sounds.add("mob.zombiepig.zpighurt", SA_zpighurt2);

	sounds.add("damage.fallbig", SA_fallbig1);
	sounds.add("damage.fallbig", SA_fallbig2);
	sounds.add("damage.fallsmall", SA_fallsmall);

	sounds.add("random.bow", SA_bow);
	sounds.add("random.bowhit", SA_bowhit1);
	sounds.add("random.bowhit", SA_bowhit2);
	sounds.add("random.bowhit", SA_bowhit3);
	sounds.add("random.bowhit", SA_bowhit4);

	sounds.add("mob.creeper", SA_creeper1);
	sounds.add("mob.creeper", SA_creeper2);
	sounds.add("mob.creeper", SA_creeper3);
	sounds.add("mob.creeper", SA_creeper4);
	sounds.add("mob.creeperdeath", SA_creeperdeath);
	sounds.add("random.eat", SA_eat1);
	sounds.add("random.eat", SA_eat2);
	sounds.add("random.eat", SA_eat3);
	sounds.add("random.fuse", SA_fuse);

	// v19: names that live call sites play but no file/registration
	// ever existed for them (sounds.get() just logged "Couldn't find
	// a sound" and stayed silent). No new audio files are added;
	// each is mapped to the closest existing sound in the tree.
	// Chest open/close -> door creak; drink/burp -> eating sounds;
	// item fizz (lava/water) -> splash; flint & steel ignite -> fuse
	// hiss. Slime/enderman have no file either (see Slime.cpp for
	// the slime placeholder; enderman stays silent, see ROADMAP).
	sounds.add("random.chestopen", SA_door_open);
	sounds.add("random.chestclosed", SA_door_close);
	sounds.add("random.drink", SA_eat1);
	sounds.add("random.drink", SA_eat2);
	sounds.add("random.drink", SA_eat3);
	sounds.add("random.burp", SA_eat1);
	sounds.add("random.burp", SA_eat2);
	sounds.add("random.fizz", SA_splash);
	sounds.add("fire.ignite", SA_fuse);

	_generateMusic();

#endif
}

void SoundEngine::enable( bool status )
{
#if defined(__APPLE__) || defined(_WIN32)
	soundSystem.enable(status);
#endif
}

void SoundEngine::updateOptions()
{

}

void SoundEngine::destroy()
{
	//if (loaded) soundSystem.cleanup();
}

void SoundEngine::update( Mob* player, float a )
{
	// v19: the listener update is still gated by the SFX volume, but
	// the music/ambience scheduler below runs on its own (it is gated
	// by options->music), so muting SFX no longer kills the music.
	if (options->sound != 0 && player != NULL) {
		_x = player->xo + (player->x - player->xo) * a;
		_y = player->yo + (player->y - player->yo) * a;
		_z = player->zo + (player->z - player->zo) * a;
		_yRot = player->yRotO + (player->yRot - player->yRotO) * a;

		soundSystem.setListenerAngle(_yRot);
		soundSystem.setListenerPos(_x, _y, _z);
	}

	_updateMusic(player);
}

// v19: procedural music + ambience. All 102 .pcm files in the tree are
// short SFX (longest ~1.4 s, the creeper fuse hiss); there is no music
// track, bird, cricket or wind file, and no ogg/mp3 decoder in the
// tree, so real recorded music is not feasible in one batch. Instead
// the tracks are synthesised once at startup as mono 16-bit PCM and
// played as one-shot buffers through the existing OpenSL path.
// Two 18 s calm pentatonic tracks (C major / A minor pad + slow
// random-walk melody), played in random order with a 45-120 s gap
// like modern MC, first track ~10 s after entering a world.
// Ambience: a short bird-chirp burst by day, cricket pulses by night,
// every ~20-45 s. Only while actually in a world and not paused
// (update() is called every frame even on the pause screen, and with
// player == NULL on the title screen); music already playing when
// the game is paused plays out — the OpenSL backend has no stop call.
void SoundEngine::_generateMusic()
{
	if (musicReady) return;
	musicTracks[0] = _makeMusicTrack(0);
	musicTracks[1] = _makeMusicTrack(1);
	birdChirp = _makeChirp(false);
	cricketChirp = _makeChirp(true);
	musicReady = musicTracks[0].isValid() && musicTracks[1].isValid()
		&& birdChirp.isValid() && cricketChirp.isValid();
}

void SoundEngine::_playMusic(const SoundDesc& sound, float volume)
{
	if (!sound.isValid()) return;
	if (options == NULL || options->music <= 0) return;
	volume *= options->music;
	if (volume <= 0) return;
	volume = Mth::clamp(volume, 0.0f, 1.0f);
	// The Android OpenSL backend ignores the position (no 3D
	// location interface is set up in SoundSystemSL::playAt), so the
	// listener position is only a formality here.
	soundSystem.playAt(sound, _x, _y, _z, volume, 1.0f);
}

void SoundEngine::_updateMusic(Mob* player)
{
	if (!musicReady) return;
	if (options == NULL || options->music <= 0) return;
	if (mc == NULL || mc->level == NULL || player == NULL || mc->pause) return;

	if (musicTrackFramesLeft > 0) {
		musicTrackFramesLeft--;
	} else if (++musicIdleFrames >= musicGapFrames) {
		_playMusic(musicTracks[random.nextInt(2)], 0.5f);
		musicTrackFramesLeft = MUSIC_TRACK_FRAMES;
		musicIdleFrames = 0;
		musicGapFrames = (45 + random.nextInt(75)) * 60;
	}

	if (++ambienceIdleFrames >= ambienceGapFrames) {
		ambienceIdleFrames = 0;
		ambienceGapFrames = (20 + random.nextInt(25)) * 60;
		if (mc->level->isDay())
			_playMusic(birdChirp, 0.35f);
		else
			_playMusic(cricketChirp, 0.3f);
	}
}

SoundDesc SoundEngine::_makeMusicTrack(int seed)
{
	// C major pentatonic (track 0) / A minor pentatonic (track 1):
	// same five scale degrees, different root chord underneath.
	static const float scale[9] = {
		261.63f, 293.66f, 329.63f, 392.00f, 440.00f,
		523.25f, 587.33f, 659.25f, 783.99f
	};
	static const float pad0[3] = { 130.81f, 164.81f, 196.00f }; // C E G
	static const float pad1[3] = { 110.00f, 130.81f, 164.81f }; // A C E
	const float* pad = (seed == 0) ? pad0 : pad1;

	const int frames = MUSIC_TRACK_SECONDS * MUSIC_RATE;
	const float noteLen = 0.45f; // seconds per melody step
	const int steps = (int)(MUSIC_TRACK_SECONDS / noteLen);
	// Precompute the melody once (random walk on the scale, with
	// rests); deriving it inside the sample loop would re-seed the
	// generator for every one of the ~400k samples.
	float noteFreq[64];
	bool noteRest[64];
	{
		Random rng(12345 + seed * 777);
		int degree = 4;
		for (int s = 0; s < steps && s < 64; s++) {
			degree += rng.nextInt(5) - 2;
			if (degree < 0) degree = 0;
			if (degree > 8) degree = 8;
			noteFreq[s] = scale[degree];
			noteRest[s] = (rng.nextInt(4) == 0);
		}
	}
	short* samples = new short[frames];
	for (int i = 0; i < frames; i++) {
		const float t = (float)i / MUSIC_RATE;
		float v = 0;
		// Soft pad: three quiet sines with a slow swell.
		const float swell = 0.5f + 0.5f * sinf(2.0f * Mth::PI * t / 9.0f);
		for (int p = 0; p < 3; p++)
			v += 0.045f * swell * sinf(2.0f * Mth::PI * pad[p] * t);
		// Melody: sine + a quiet octave harmonic, attack + decay.
		const int step = (int)(t / noteLen);
		if (step < steps && !noteRest[step]) {
			const float nt = t - step * noteLen;
			const float f = noteFreq[step];
			float env = nt / 0.08f;
			if (env > 1.0f) env = 1.0f;
			env *= 1.0f - 0.75f * (nt / noteLen);
			// Short release so the note ends at silence instead of
			// cutting off mid-sine (audible click every step).
			if (nt > noteLen - 0.05f) env *= (noteLen - nt) / 0.05f;
			v += 0.28f * env * (sinf(2.0f * Mth::PI * f * t)
				+ 0.35f * sinf(4.0f * Mth::PI * f * t));
		}
		if (v > 0.95f) v = 0.95f;
		if (v < -0.95f) v = -0.95f;
		samples[i] = (short)(v * 32767.0f);
	}
	return SoundDesc((char*)samples, frames * 2, 1, 2, MUSIC_RATE);
}

SoundDesc SoundEngine::_makeChirp(bool cricket)
{
	const int seconds = 2;
	const int frames = seconds * MUSIC_RATE;
	short* samples = new short[frames];
	for (int i = 0; i < frames; i++) {
		const float t = (float)i / MUSIC_RATE;
		float v = 0;
		if (cricket) {
			// Night: rapid 4.2 kHz pulses, ~45 ms on / ~45 ms off.
			const float cycle = fmodf(t, 0.09f);
			if (cycle < 0.045f)
				v = 0.20f * sinf(2.0f * Mth::PI * 4200.0f * t);
		} else {
			// Day: three short descending bird chirps.
			static const float starts[3] = { 0.10f, 0.60f, 1.15f };
			for (int c = 0; c < 3; c++) {
				const float ct = t - starts[c];
				if (ct >= 0 && ct < 0.20f) {
					const float f = 4600.0f - 1400.0f * (ct / 0.20f);
					const float env = sinf(Mth::PI * ct / 0.20f);
					v += 0.30f * env * sinf(2.0f * Mth::PI * f * t);
				}
			}
		}
		if (v > 0.95f) v = 0.95f;
		if (v < -0.95f) v = -0.95f;
		samples[i] = (short)(v * 32767.0f);
	}
	return SoundDesc((char*)samples, frames * 2, 1, 2, MUSIC_RATE);
}

float SoundEngine::_getVolumeMult( float x, float y, float z )
{
	const float dx = x - _x;
	const float dy = y - _y;
	const float dz = z - _z;
	const float dist = Mth::sqrt(dx*dx + dy*dy + dz*dz);
	const float out =  Mth::clamp(1.1f - dist*_invMaxDistance, -1.0f, 1.0f);
	return out;
}

#if defined(PRE_ANDROID23)
void SoundEngine::play(const std::string& name, float x, float y, float z, float volume, float pitch) {
	//volume *= (2.0f * _getVolumeMult(x, y, z))
	if ((volume *= options->sound) <= 0) return;

	volume *= _getVolumeMult(x, y, z);
	mc->platform()->playSound(name, volume, pitch);
}
void SoundEngine::playUI(const std::string& name, float volume, float pitch) {
	if ((volume *= options->sound) <= 0) return;

	//volume *= 2.0f;
	mc->platform()->playSound(name, volume, pitch);
}
#elif defined(__APPLE__) || defined(_WIN32)
void SoundEngine::play(const std::string& name, float x, float y, float z, float volume, float pitch) {
	if ((volume *= options->sound) <= 0) return;

	volume = Mth::clamp(volume, 0.0f, 1.0f);

	SoundDesc sound;
	if (sounds.get(name, sound)) {
		soundSystem.playAt(sound, x-_x, y-_y, z-_z, volume, pitch);
	}
}
void SoundEngine::playUI(const std::string& name, float volume, float pitch) {
	if ((volume *= options->sound) <= 0) return;

	volume = Mth::clamp(volume, 0.0f, 1.0f);
	if (/*!loaded || */options->sound == 0 || volume <= 0) return;

	SoundDesc sound;
	if (sounds.get(name, sound)) {
		soundSystem.playAt(sound, 0, 0, 0, volume, pitch);
	}
}
#elif defined(RPI)
void SoundEngine::play(const std::string& name, float x, float y, float z, float volume, float pitch) {}
void SoundEngine::playUI(const std::string& name, float volume, float pitch) {}
#else
void SoundEngine::play(const std::string& name, float x, float y, float z, float volume, float pitch) {
	if ((volume *= options->sound) <= 0) return;

	volume = Mth::clamp( volume * _getVolumeMult(x, y, z), 0.0f, 1.0f);
	if (/*!loaded || */options->sound == 0 || volume <= 0) return;

	SoundDesc sound;
	if (sounds.get(name, sound)) {
		float dist = SOUND_DISTANCE;
		if (volume > 1) dist *= volume;
		soundSystem.playAt(sound, x, y, z, volume, pitch);
	}
}
void SoundEngine::playUI(const std::string& name, float volume, float pitch) {
	if ((volume *= options->sound) <= 0) return;

	volume = Mth::clamp(volume, 0.0f, 1.0f);
	if (/*!loaded || */options->sound == 0 || volume <= 0) return;

	SoundDesc sound;
	if (sounds.get(name, sound)) {
		soundSystem.playAt(sound, 0, 0, 0, volume, pitch);
	}
}
#endif
