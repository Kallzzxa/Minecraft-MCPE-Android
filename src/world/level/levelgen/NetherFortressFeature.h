#ifndef NET_MINECRAFT_WORLD_LEVEL_LEVELGEN__NetherFortressFeature_H__
#define NET_MINECRAFT_WORLD_LEVEL_LEVELGEN__NetherFortressFeature_H__

// v31: Nether fortress.
//
// The v21 Nether shipped terrain, lava, ghasts/blazes and wart farming
// but no fortress — the Nether had no landmark to explore for. This
// is the first fortress stage: structure + blaze home + wart room.
// (No blaze-spawner tile yet: Tile::mobSpawner is declared in Tile.h
// but never defined/registered, and no spawner tile entity exists in
// this tree — see ROADMAP. Blaze presence is the village/witch
// top-up pattern instead. No wither skeleton either — see ROADMAP.)
//
// Placement reuses the VillageFeature pattern exactly: the Nether is
// divided into a grid of GRID_CELLS x GRID_CELLS chunks (20x20 = 320
// blocks); each grid cell holds exactly one fortress, at a jittered
// chunk inside it (splitmix64 hash of world seed + grid cell, a
// different salt from villages so the two grids never correlate).
// Every chunk overlapping a fortress computes the SAME layout and
// places only the parts inside its own 16x16 area: no cross-chunk
// writes, no generation recursion, saved with the chunks like any
// other terrain, so a fortress survives quit/reload exactly like a
// village does.
//
// Deck height: the Nether has no usable heightmap (the bedrock
// ceiling at y=127 would read as the "top"), so the deck Y is a pure
// function of the terrain noise — the MAXIMUM Nether floor height
// over the fortress footprint (NetherLevelSource::getFloorHeight,
// the exact formula getChunk uses), +1 standing level. The deck is
// therefore always above the highest netherrack in its footprint and
// never buried. A fortress is skipped when that deck would sit in
// or just above the lava sea (deckY < 36; the sea tops out at y=31).
// Support pillars of nether brick run from the deck down to solid
// ground under the bridge and rooms, through lava where the floor
// dips, so nothing floats over the void/lava.
//
// Layout (centre wx,wz, deck deckY = D), all nether brick unless
// noted, ~49 x 33 blocks:
//  - bridge E-W: deck x wx-16..wx+16, z wz-2..wz+2, 1-high curbs
//    on the outer rows (walkable middle 3 wide).
//  - corridor N-S: x wx-1..wx+1, z wz-14..wz+14, side walls at
//    x=wx+-2 (D+1..D+3) + flat roof at D+4, open at both ends.
//  - blaze room (east): 7x7 at x wx+17..wx+23, z wz-3..wz+3, walls
//    D+1..D+3, roof D+4, doorway on the west wall, glowstone in the
//    4 inner ceiling corners. Blazes live here (ensureBlazes).
//  - wart room (west): 7x5 at x wx-23..wx-17, z wz-2..wz+2, soul
//    sand floor, 10 mature nether wart (data 3) in two rows, walls
//    D+1..D+3, roof D+4, doorway on the east wall, 2 glowstone.
// Blazes are NOT placed by postProcess (entities are not chunk
// data — they live in the global entities.dat, and chunk eviction
// despawns them like every other mob). Instead, ensureBlazes() tops
// the fortress up to BLAZES_PER_FORTRESS whenever its centre chunk
// (re-)enters the resident set (ChunkCache getChunk / postProcess
// hooks, same as ensureVillagers / ensureWitch).

class Level;
class Random;

class NetherFortressFeature
{
public:
	static const int GRID_CELLS = 20;             // chunks per grid cell
	static const int BLAZES_PER_FORTRESS = 3;
	static const int WITHER_SKELETONS_PER_FORTRESS = 2;

	// Deterministic fortress-centre chunk for grid cell (gx, gz).
	static void getFortressCenterChunk(long seed, int gx, int gz, int& outChunkX, int& outChunkZ);
	// True iff chunk (chunkX, chunkZ) is the centre chunk of its grid
	// cell's fortress; then (outCenterX, outCenterZ) is the centre block.
	static bool isFortressCenterChunk(long seed, int chunkX, int chunkZ, int& outCenterX, int& outCenterZ);
	// Deck Y for the fortress centred at (wx, wz): max Nether floor
	// height over the footprint. < 36 means "skip this fortress"
	// (deck would sit in/just above the lava sea).
	static int getDeckY(long seed, int wx, int wz);
	// Place every fortress part overlapping chunk (chunkX, chunkZ).
	// Called from NetherLevelSource::postProcess (Nether only).
	static void tryPlaceFortress(Level* level, int chunkX, int chunkZ);
	// Spawn/top-up blazes for the fortress centred in this chunk.
	// Called from ChunkCache when the chunk enters the resident set.
	static void ensureBlazes(Level* level, int chunkX, int chunkZ);
	// v32: same top-up for wither skeletons (2 per fortress, in the
	// corridor / on the bridge — the blaze room is the blazes').
	static void ensureWitherSkeletons(Level* level, int chunkX, int chunkZ);
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_LEVELGEN__NetherFortressFeature_H__*/
