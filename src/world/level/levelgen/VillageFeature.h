#ifndef NET_MINECRAFT_WORLD_LEVEL_LEVELGEN__VillageFeature_H__
#define NET_MINECRAFT_WORLD_LEVEL_LEVELGEN__VillageFeature_H__

// v23: villages + villagers.
//
// A village is placed deterministically: the world is divided into a
// grid of GRID_CELLS x GRID_CELLS chunks (24x24 = 384 blocks); each grid
// cell holds exactly one village, at a jittered chunk inside the cell
// (hash of world seed + grid cell). That makes villages findable (~one
// per 384 blocks in plains/desert) while keeping them rare enough not
// to pave the world, and — critically — it makes placement a pure
// function of (seed, chunk): every chunk that overlaps a village
// computes the SAME layout and places only the parts inside its own
// 16x16 area. No cross-chunk writes (so no generation recursion), and
// the village is saved with the chunks like any other terrain, so it
// survives quit/reload exactly like trees and ores do.
//
// A village has: a well (centre), gravel paths (cross + plaza ring),
// 5 houses (cobblestone or plank walls, log corners, plank roof, open
// doorway facing the centre, glass windows, glowstone floor lamp),
// 2 farm plots (wheat + carrots, mature, with a water channel) and
// 4 lamp posts (fence + glowstone). The ground inside the village
// radius is flattened to the centre height first, so houses sit level.
// Villages only generate in plains/desert, on dry land (centre
// heightmap >= 65, i.e. above the sea level of 64).
//
// Villagers are NOT placed by postProcess (entities are not part of
// chunk data — they live in the global entities.dat, and chunk
// eviction despawns them like every other mob). Instead,
// ensureVillagers() tops the village up to VILLAGERS_PER_VILLAGE
// whenever its centre chunk (re-)enters the resident set (ChunkCache
// getChunk / postProcess hooks). A villager killed or left behind is
// replaced on the next visit — deliberate, so villages stay populated.

class Level;
class Random;

class VillageFeature
{
public:
	static const int GRID_CELLS = 24;             // chunks per grid cell
	static const int VILLAGE_RADIUS = 26;         // blocks from centre
	static const int VILLAGERS_PER_VILLAGE = 4;

	// Deterministic village-centre chunk for grid cell (gx, gz).
	static void getVillageCenterChunk(long seed, int gx, int gz, int& outChunkX, int& outChunkZ);
	// True iff chunk (chunkX, chunkZ) is the centre chunk of its grid
	// cell's village; then (outCenterX, outCenterZ) is the well block.
	static bool isVillageCenterChunk(long seed, int chunkX, int chunkZ, int& outCenterX, int& outCenterZ);
	// Place every village part overlapping chunk (chunkX, chunkZ).
	// Called from RandomLevelSource::postProcess (overworld only).
	static void tryPlaceVillage(Level* level, int chunkX, int chunkZ);
	// Spawn/top-up villagers for the village centred in this chunk.
	// Called from ChunkCache when the chunk enters the resident set.
	static void ensureVillagers(Level* level, int chunkX, int chunkZ);
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_LEVELGEN__VillageFeature_H__*/
