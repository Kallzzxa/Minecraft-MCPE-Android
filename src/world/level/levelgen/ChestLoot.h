#ifndef NET_MINECRAFT_WORLD_LEVEL_LEVELGEN__ChestLoot_H__
#define NET_MINECRAFT_WORLD_LEVEL_LEVELGEN__ChestLoot_H__

// v33: loot-chest filling shared by villages + the Nether fortress.
//
// The chest BLOCK is placed by the feature's own clipped setter
// (VillageFeature::setV / NetherFortressFeature::setF). Placing it
// calls EntityTile::onPlace, which creates the ChestTileEntity
// synchronously and registers it in Level::tileEntities (+ the chunk
// map) — so placing a chest during postProcess DOES have a precedent
// after all: it is the normal block-place path, not a special tile
// entity constructor call. These helpers only FILL the chest.
//
// They deliberately do NOT use Level::getTileEntity(): that goes
// through LevelChunk::getTileEntity, which LAZY-CREATES a fresh empty
// tile entity when the chunk map has none — and during a game tick
// the onPlace-created entity may still sit in
// Level::pendingTileEntities (updatingTileEntities), so the lazy
// twin would be created, filled with the loot, and then DELETED by
// the pending-flush dedup (which keeps the first, empty, entity).
// Instead the helpers scan Level::tileEntities +
// Level::pendingTileEntities directly for the entity at (x,y,z) —
// both lists are public (Level.h:306-307) — and fill only if one is
// found. If neither list has it (cannot happen server-side:
// onPlace always registers it), the chest stays empty rather than
// risking a duplicate tile entity.
//
// Save/load + eviction come free from the v17 machinery:
// ChestTileEntity::shouldSave() is true while any slot is non-empty,
// so a filled chest is written to the global entities.dat;
// TileEntity::loadStatic rebuilds it on load; ChunkCache::
// reattachTileEntities re-attaches the ORIGINAL entity (guarded by
// hasTileEntityAt, so no duplicate); disk-loaded chunks have
// terrainPopulated=true, so postProcess never re-runs and never
// places a second chest. Chests only exist in chunks generated AFTER
// v33 — villages/fortresses in old worlds do not gain one
// retroactively (same rule as the v25 swamp-hut cauldron).

class Level;
class Random;
class Item;
class Tile;

struct ChestLootEntry
{
	Item* item;     // non-NULL for item loot
	Tile* tile;     // non-NULL for block loot (e.g. obsidian)
	int minCount;
	int maxCount;
	int weight;     // relative weight, must be > 0
};

class ChestLoot
{
public:
	// Fill the chest tile entity at (x,y,z) with minStacks..maxStacks
	// randomly chosen stacks from table. No-ops (without touching the
	// Random) when the block at (x,y,z) is not a chest — i.e. when the
	// feature's clipped setter did not place it in this chunk — and
	// when no tile entity is registered for it yet.
	static void fillChest(Level* level, int x, int y, int z, Random* random,
		const ChestLootEntry* table, int tableSize,
		int minStacks, int maxStacks);

	// Village chest loot (v23 debt): bread/wheat/carrots from the
	// village farms, apples, coal, iron, string, and a rare emerald
	// — the trading currency, so a lucky chest seeds the trade loop.
	static void fillVillageChest(Level* level, int x, int y, int z, Random* random);

	// Fortress chest loot (v31 debt): Nether-flavoured — iron/gold,
	// coal, bread, obsidian, a rare diamond (the tree's "emerald"
	// gem is the diamond, see Item::emerald) and a rare flint &
	// steel.
	static void fillFortressChest(Level* level, int x, int y, int z, Random* random);
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_LEVELGEN__ChestLoot_H__*/
