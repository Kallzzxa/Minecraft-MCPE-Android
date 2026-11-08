#ifndef NET_MINECRAFT_WORLD_LEVEL_LEVELGEN__SwampHutFeature_H__
#define NET_MINECRAFT_WORLD_LEVEL_LEVELGEN__SwampHutFeature_H__

// v24: swamp hut (the witch's hut). One hut per 16x16-chunk grid
// cell, at a jittered chunk inside it (same deterministic
// splitmix64-hash pattern as VillageFeature, so every chunk
// overlapping a hut computes the SAME layout and places only the
// parts inside its own 16x16 area: no cross-chunk writes, no
// generation recursion, saved with the chunks like any other
// terrain). Swampland only, on ground at height >= 62 (swamp
// lowlands; the hut stands on log stilts over the wet ground).
// The hut is 7x5 planks on stilts with a log frame, plank roof
// with overhang, a doorway, a glass window and a workbench inside.
// NO cauldron: no cauldron tile exists in this tree (v20 debt), so
// the hut has no cauldron yet. One witch lives in each hut
// (ensureWitch, same top-up pattern as VillageFeature::
// ensureVillagers — entities are not chunk data, so the hut's witch
// is topped back up to 1 whenever the hut chunk (re-)enters the
// resident set).

class Level;

class SwampHutFeature
{
public:
	static const int GRID_CELLS = 16;      // chunks per grid cell
	static const int HUT_RADIUS = 6;       // blocks from centre

	// Deterministic hut-centre chunk for grid cell (gx, gz).
	static void getHutCenterChunk(long seed, int gx, int gz, int& outChunkX, int& outChunkZ);
	// True iff chunk (chunkX, chunkZ) is the centre chunk of its grid
	// cell's hut; then (outCenterX, outCenterZ) is the hut centre block.
	static bool isHutCenterChunk(long seed, int chunkX, int chunkZ, int& outCenterX, int& outCenterZ);
	// Place every hut part overlapping chunk (chunkX, chunkZ).
	// Called from RandomLevelSource::postProcess (overworld only).
	static void tryPlaceSwampHut(Level* level, int chunkX, int chunkZ);
	// Spawn/top-up the hut's witch for the hut centred in this chunk.
	// Called from ChunkCache when the chunk enters the resident set.
	static void ensureWitch(Level* level, int chunkX, int chunkZ);
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_LEVELGEN__SwampHutFeature_H__*/
