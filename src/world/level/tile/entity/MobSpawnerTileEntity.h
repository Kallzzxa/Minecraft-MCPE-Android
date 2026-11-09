#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE_ENTITY__MobSpawnerTileEntity_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE_ENTITY__MobSpawnerTileEntity_H__

// v39: blaze spawner tile entity (tile 52, TileEntityType::MobSpawner).
// Pays the v31 debt: Tile::mobSpawner was declared in Tile.h but never
// defined, and no spawner tile entity existed — the fortress blaze
// room refilled by chunk-reload top-up (ensureBlazes) instead of by a
// spawner block. This is the spawner half: a ticking tile entity that
// spawns blazes around itself while a player is near.
//
// Rules (modern MC, tuned for this tree):
//   - ACTIVE_RADIUS 16 blocks: with no player within 16 blocks the
//     spawner does nothing and its delay does not count down.
//   - On an attempt it spawns ONE blaze at a random air spot within
//     +-4 blocks horizontally and at the spawner level or one above,
//     unless MAX_NEARBY (4) blazes are already inside the 9x4x9 box
//     around it — then it waits and retries.
//   - After every attempt (spawned or not) the delay re-rolls to
//     200-400 ticks (10-20 s, modern MC is 200-800; the shorter end
//     keeps a cleared blaze room refilling on a testable timescale).
//     The first delay after placement/load is 200 ticks.
//   - The global MobSpawner monster cap is deliberately NOT consulted
//     (modern spawners ignore it, and ensureBlazes also bypasses it
//     via addEntity) — the per-spawner cap above is the limit.
//   - Server-side only (level->isClientSide returns early), like the
//     furnace/brewing-stand ticks.
//
// Save/load: the delay persists ("Delay") and shouldSave() is always
// true (Sign precedent), so the tile entity survives quit/reload via
// entities.dat. Even so, a spawner whose tile entity is ever missing
// is recreated empty by LevelChunk::getTileEntity's lazy path (any
// EntityTile), and chunk eviction detaches/re-attaches the ORIGINAL
// object (ChunkCache::reattachTileEntities) — no duplicate, no loss.

#include "TileEntity.h"

class CompoundTag;

class MobSpawnerTileEntity: public TileEntity
{
	typedef TileEntity super;
public:
	// Player must be within this many blocks for the spawner to run.
	static const int ACTIVE_RADIUS = 16;
	// Blazes already inside this box around the spawner block the
	// next spawn (box is 9 wide, 4 high: x/z +-4, y -1..+2).
	static const int SPAWN_RADIUS = 4;
	static const int MAX_NEARBY = 4;
	// Delay re-roll after each attempt, in ticks (20 ticks = 1 s).
	static const int MIN_DELAY = 200;
	static const int MAX_DELAY = 400;

	MobSpawnerTileEntity();

	void load(CompoundTag* base);
	bool save(CompoundTag* base);
	bool shouldSave();

	void tick();

	int delay;
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_TILE_ENTITY__MobSpawnerTileEntity_H__*/
