#include "ChestLoot.h"

#include "../Level.h"
#include "../tile/Tile.h"
#include "../tile/entity/TileEntity.h"
#include "../tile/entity/ChestTileEntity.h"
#include "../../item/Item.h"
#include "../../item/ItemInstance.h"
#include "../../../util/Random.h"

// ---------------------------------------------------------------------------
// Chest lookup (no lazy creation — see the header)
// ---------------------------------------------------------------------------

static ChestTileEntity* findChestTileEntity(Level* level, int x, int y, int z)
{
	// Level::tileEntities first (the normal case: onPlace registered
	// it there synchronously), then the pending list (postProcess ran
	// during a game tick while updatingTileEntities was true).
	for (size_t i = 0; i < level->tileEntities.size(); ++i) {
		TileEntity* te = level->tileEntities[i];
		if (te != NULL && !te->isRemoved() && te->type == TileEntityType::Chest
			&& te->x == x && te->y == y && te->z == z)
			return (ChestTileEntity*)te;
	}
	for (size_t i = 0; i < level->pendingTileEntities.size(); ++i) {
		TileEntity* te = level->pendingTileEntities[i];
		if (te != NULL && !te->isRemoved() && te->type == TileEntityType::Chest
			&& te->x == x && te->y == y && te->z == z)
			return (ChestTileEntity*)te;
	}
	return NULL;
}

void ChestLoot::fillChest(Level* level, int x, int y, int z, Random* random,
	const ChestLootEntry* table, int tableSize,
	int minStacks, int maxStacks)
{
	if (level == NULL || random == NULL || table == NULL || tableSize <= 0) return;
	if (minStacks < 0) minStacks = 0;
	if (maxStacks < minStacks) maxStacks = minStacks;

	// The chest block must actually be here: the feature's clipped
	// setter only places it in the chunk being generated, and the
	// fill is a no-op in every other overlapping chunk. Returning
	// before touching the Random keeps the rolls identical no matter
	// which chunk triggers the fill.
	if (level->getTile(x, y, z) != Tile::chest->id) return;

	ChestTileEntity* chest = findChestTileEntity(level, x, y, z);
	if (chest == NULL) return;

	int totalWeight = 0;
	for (int i = 0; i < tableSize; ++i)
		if (table[i].weight > 0) totalWeight += table[i].weight;
	if (totalWeight <= 0) return;

	int stacks = minStacks;
	if (maxStacks > minStacks) stacks += random->nextInt(maxStacks - minStacks + 1);

	int size = chest->getContainerSize();
	for (int s = 0; s < stacks; ++s) {
		// Weighted pick.
		int roll = random->nextInt(totalWeight);
		const ChestLootEntry* entry = NULL;
		for (int i = 0; i < tableSize; ++i) {
			if (table[i].weight <= 0) continue;
			roll -= table[i].weight;
			if (roll < 0) { entry = &table[i]; break; }
		}
		if (entry == NULL) continue;

		int count = entry->minCount;
		if (entry->maxCount > entry->minCount)
			count += random->nextInt(entry->maxCount - entry->minCount + 1);

		// Random empty slot (bounded retries; a full chest just
		// skips the stack instead of looping forever).
		int slot = -1;
		for (int t = 0; t < 8; ++t) {
			int cand = random->nextInt(size);
			ItemInstance* cur = chest->getItem(cand);
			if (cur == NULL || cur->isNull()) { slot = cand; break; }
		}
		if (slot < 0) continue;

		// FillingContainer::setItem COPIES the instance, so a stack
		// temporary is safe (no ownership transfer, no leak).
		if (entry->item != NULL) {
			ItemInstance inst(entry->item, count);
			chest->setItem(slot, &inst);
		} else if (entry->tile != NULL) {
			ItemInstance inst(entry->tile, count);
			chest->setItem(slot, &inst);
		}
	}
	// No setChanged() needed: the chunk is already marked unsaved by
	// the setTile that placed the chest, and saveEntities() writes
	// every tile entity with shouldSave() (non-empty chest) into the
	// global entities.dat on evict/quit/periodic save regardless.
}

void ChestLoot::fillVillageChest(Level* level, int x, int y, int z, Random* random)
{
	// Bread/wheat/carrots from the village farms, apples, coal, iron,
	// string, and a rare emerald — the trading currency (v23), so a
	// lucky chest seeds the wheat/coal -> emerald -> bread/tools loop.
	static const ChestLootEntry table[] = {
		{ Item::bread,      NULL, 2, 4, 10 },
		{ Item::wheat,      NULL, 3, 6, 10 },
		{ Item::carrot,     NULL, 2, 4,  6 },
		{ Item::apple,      NULL, 1, 3,  6 },
		{ Item::coal,       NULL, 2, 4,  8 },
		{ Item::ironIngot,  NULL, 1, 3,  7 },
		{ Item::string,     NULL, 1, 3,  5 },
		{ Item::emeraldGem, NULL, 1, 2,  4 },
	};
	fillChest(level, x, y, z, random, table,
		sizeof(table) / sizeof(table[0]), 3, 5);
}

void ChestLoot::fillFortressChest(Level* level, int x, int y, int z, Random* random)
{
	// Nether-flavoured: iron/gold ingots, coal, bread, obsidian, a
	// rare diamond (Item::emerald IS the diamond gem in this tree's
	// old naming — item.emerald.name is "Diamond") and a rare flint
	// & steel. No blaze rods / nether wart: the fortress already has
	// blazes and a wart room next door.
	static const ChestLootEntry table[] = {
		{ Item::ironIngot,    NULL,           1, 3, 10 },
		{ Item::goldIngot,    NULL,           1, 3, 10 },
		{ Item::coal,         NULL,           1, 3,  7 },
		{ Item::bread,        NULL,           1, 3,  6 },
		{ NULL,               Tile::obsidian, 1, 2,  6 },
		{ Item::emerald,      NULL,           1, 1,  3 },
		{ Item::flintAndSteel, NULL,          1, 1,  3 },
	};
	fillChest(level, x, y, z, random, table,
		sizeof(table) / sizeof(table[0]), 3, 4);
}
