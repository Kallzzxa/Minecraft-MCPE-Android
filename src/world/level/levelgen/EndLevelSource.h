#ifndef NET_MINECRAFT_WORLD_LEVEL_LEVELGEN__EndLevelSource_H__
#define NET_MINECRAFT_WORLD_LEVEL_LEVELGEN__EndLevelSource_H__

// v22: End terrain generator (ChunkSource, same pattern as
// NetherLevelSource in v21 — a fresh generator, no chunkMap: the
// ChunkCache owns every chunk, eviction deletes via the cache).
//
// Terrain, all deterministic from (seed, chunk, block) hashes:
//  - ONE floating island of end stone centred on (0, 0), radius ~48
//    blocks, top y ~62-66, ~14 blocks thick, void below and outside.
//    Falling off the edge is a real fall into the void (the player
//    takes void damage in LocalPlayer::tick — Entity::outOfWorld
//    would just remove() the player with no death screen).
//  - 10 obsidian pillars (3x3) in a ring at radius 20-32, heights
//    9-24 above y=64, each capped with a single bedrock block. The
//    dragon orbits at radius 22 / y=82, so it clears every pillar
//    top that sits inside its orbit (all tops above y=82 are at
//    radius >= 26).
//  - Spawn platform at the island centre: 5x5 obsidian at y=64,
//    an 8-block bedrock ring at y=65, centre (0,65,0) left as air —
//    the exit portal. The dragon's death fills the centre with the
//    end portal tile and places the dragon egg on the rim.
//  - Sparse chorus plants on the island top (hash > 0.985), planted
//    at growth stage 0; they grow by random tick like nether wart.
//
// Mobs: getMobsAt returns endermen only (the dragon is spawned once
// by Minecraft::ensureEndDragon, never by the spawner). No friendly
// or aquatic mobs. isSpawnPositionOk (solid below + 2 air) works on
// the island as-is; Monster::canSpawn's darkness check passes
// because the End is permanent dusk (timeOfDay 0.75).

#include "../chunk/ChunkSource.h"
#include "../chunk/LevelChunk.h"
#include "../Level.h"
#include "../tile/Tile.h"
#include "../../entity/EntityTypes.h"
#include "../biome/Biome.h"

class EndLevelSource: public ChunkSource
{
public:
	EndLevelSource(Level* level, long seed);
	virtual ~EndLevelSource() {}

	bool hasChunk(int x, int y) { (void) x; (void) y; return true; }
	LevelChunk* create(int x, int z) { return getChunk(x, z); }
	LevelChunk* getChunk(int x, int z);
	void postProcess(ChunkSource* parent, int x, int z) {
		(void) parent; (void) x; (void) z;
		// All decoration (pillars, platform, chorus) is placed in
		// getChunk itself, so there is nothing left to post-process.
	}
	bool tick() { return false; }
	bool shouldSave() { return true; }
	std::string gatherStats() { return "End"; }

	Biome::MobList getMobsAt(const MobCategory& mobCategory, int x, int y, int z);

	// Exit-portal centre (the dragon fills this with the end portal
	// tile when it dies; Minecraft checks it to know the dragon is
	// already defeated). Shared so the dragon and Minecraft agree.
	static const int EXIT_PORTAL_Y = 65;

	// v27: respawn-spot tests for end crystals. A pillar cap is the
	// single bedrock block on a pillar top (y = 64 + height); an
	// exit-ring cell is one of the 8 bedrock blocks around the exit
	// portal centre at y = 65.
	static bool isPillarTop(int x, int y, int z);
	static bool isExitRingCell(int x, int y, int z);

	// v28: pillar-top coordinates for the initial end crystals
	// (Minecraft::ensureEndDragon). Pillar i's bedrock cap is at
	// (PILLARS[i].x, 64 + PILLARS[i].h, PILLARS[i].z); returns false
	// for an out-of-range index.
	static int getNumPillars() { return 10; }
	static bool getPillarTop(int i, int& x, int& y, int& z);

private:
	// Deterministic 0..1 hashes of integer coords (no RNG state, so a
	// chunk generates identically whenever it is (re)generated).
	static float hash01(int x, int y, int z, long seed);
	static float hash01(int x, int z, long seed) { return hash01(x, 0, z, seed); }
	// Smooth 2-octave value noise over block coords.
	static float noise2D(long seed, int x, int z);

	// Obsidian pillars: (centre x, centre z, height above y=64).
	// Hand-computed ring positions (radius 20/26/32, tops y=73..88);
	// the dragon's orbit (r=22, y=82) clears every pillar whose top
	// is above 82 because those all sit at radius >= 26.
	struct Pillar { int x, z, h; };
	static const Pillar PILLARS[10];

	Level* level;
	long seed;
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_LEVELGEN__EndLevelSource_H__*/
