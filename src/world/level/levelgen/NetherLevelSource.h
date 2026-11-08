#ifndef NET_MINECRAFT_WORLD_LEVEL_LEVELGEN__NetherLevelSource_H__
#define NET_MINECRAFT_WORLD_LEVEL_LEVELGEN__NetherLevelSource_H__

// v21: Nether terrain generator (ChunkSource, like RandomLevelSource
// but a fresh generator — the overworld one would bring trees, water
// lakes, beaches and snow with it).
//
// Terrain, all deterministic from (seed, chunk, block) hashes:
//  - bedrock floor y=0 (+ scattered y=1..4) and bedrock ceiling y=127
//    (+ scattered y=126), so the Nether is a closed cave.
//  - rolling netherrack floor, height 28..62 (2-octave value noise).
//  - lava sea up to y=31 wherever the floor dips below 32.
//  - soul sand patches on the floor (noise > 0.78, top 3 layers).
//  - nether quartz ore veins inside netherrack (~1.2% of blocks).
//  - glowstone clusters hanging under the ceiling (y=120..125).
//  - no trees, no water, no ores from the overworld, no structures
//    (no Nether fortress in this batch — see ROADMAP).
//
// No chunkMap: unlike RandomLevelSource this source keeps no second
// store of generated chunks. ChunkCache owns every chunk it holds, so
// a cache miss generates fresh here and eviction deletes via the
// cache (ChunkSource::evictChunk default = false = "not mine, caller
// deletes"). That keeps the streaming window bounded exactly like the
// overworld's disk-loaded chunks.
//
// Mobs: getMobsAt is the hook MobSpawner uses (Level::
// getRandomMobSpawnAt -> ChunkSource::getMobsAt), so the Nether spawn
// list lives here: pig zombies, ghasts, blazes. Nothing friendly,
// nothing aquatic. isSpawnPositionOk (solid below + 2 air) works on
// the Nether floor as-is; Monster::canSpawn's darkness check passes
// because hasCeiling leaves sky light at 0.

#include "../chunk/ChunkSource.h"
#include "../chunk/LevelChunk.h"
#include "../Level.h"
#include "../tile/Tile.h"
#include "../../entity/EntityTypes.h"
#include "../biome/Biome.h"

class NetherLevelSource: public ChunkSource
{
public:
	NetherLevelSource(Level* level, long seed);
	virtual ~NetherLevelSource() {}

	bool hasChunk(int x, int y) { (void) x; (void) y; return true; }
	LevelChunk* create(int x, int z) { return getChunk(x, z); }
	LevelChunk* getChunk(int x, int z);
	void postProcess(ChunkSource* parent, int x, int z) {
		(void) parent; (void) x; (void) z;
		// All decoration (ore, glowstone, soul sand) is placed in
		// getChunk itself, so there is nothing left to post-process.
		// ChunkCache still marks the chunk terrainPopulated.
	}
	bool tick() { return false; }
	bool shouldSave() { return true; }
	std::string gatherStats() { return "Nether"; }

	Biome::MobList getMobsAt(const MobCategory& mobCategory, int x, int y, int z);

private:
	// Deterministic 0..1 hashes of integer coords (no RNG state, so a
	// chunk generates identically whenever it is (re)generated).
	static float hash01(int x, int y, int z, long seed);
	static float hash01(int x, int z, long seed) { return hash01(x, 0, z, seed); }
	// Smooth 2-octave value noise over block coords.
	static float noise2D(long seed, int x, int z);

	Level* level;
	long seed;
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_LEVELGEN__NetherLevelSource_H__*/
