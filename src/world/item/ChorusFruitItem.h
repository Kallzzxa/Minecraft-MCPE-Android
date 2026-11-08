#ifndef NET_MINECRAFT_WORLD_ITEM__ChorusFruitItem_H__
#define NET_MINECRAFT_WORLD_ITEM__ChorusFruitItem_H__

// v22: chorus fruit (modern MC id 432). Food (+4 hunger) that also
// plants the chorus crop: use on the top face of end stone to plant
// a chorus plant above it (SeedItem pattern — PlantableFoodItem
// hardcodes farmland, so this item carries its own useOn).
//
// Eating teleports the eater to a random nearby spot (up to 8
// blocks horizontally, 3 vertically), like modern MC: up to 8 tries
// for a spot with 2 air blocks above solid ground. If no spot is
// found the fruit is still eaten, just without a teleport.

#include "FoodItem.h"
#include "../level/Level.h"
#include "../level/tile/Tile.h"
#include "../entity/player/Player.h"
#include "../../util/Mth.h"

class ChorusFruitItem: public FoodItem
{
	typedef FoodItem super;
public:
	ChorusFruitItem(int id)
	:	super(id, 4, false)
	{
	}

	bool useOn(ItemInstance* itemInstance, Player* player, Level* level, int x, int y, int z, int face, float clickX, float clickY, float clickZ) {
		(void) player; (void) clickX; (void) clickY; (void) clickZ;
		if (face != 1) return false;
		if (level->getTile(x, y, z) != Tile::endStone->id) return false;
		if (!level->isEmptyTile(x, y + 1, z)) return false;
		level->setTile(x, y + 1, z, Tile::chorusPlant->id);
		itemInstance->count--;
		return true;
	}

	ItemInstance useTimeDepleted(ItemInstance* instance, Level* level, Player* player) {
		ItemInstance result = super::useTimeDepleted(instance, level, player);
		teleportRandomly(level, player);
		return result;
	}

private:
	static void teleportRandomly(Level* level, Player* player) {
		for (int i = 0; i < 8; i++) {
			int nx = Mth::floor(player->x) + level->random.nextInt(17) - 8;
			int ny = Mth::floor(player->y) + level->random.nextInt(7) - 3;
			int nz = Mth::floor(player->z) + level->random.nextInt(17) - 8;
			if (ny < 1 || ny > 126) continue;
			int below = level->getTile(nx, ny - 1, nz);
			if (below == 0 || !Tile::tiles[below]->isSolidRender()) continue;
			if (level->getTile(nx, ny, nz) != 0) continue;
			if (level->getTile(nx, ny + 1, nz) != 0) continue;
			player->setPos(nx + 0.5f, (float)ny, nz + 0.5f);
			level->playSound(player, "random.pop", 1.0f, 1.0f);
			return;
		}
	}
};

#endif /*NET_MINECRAFT_WORLD_ITEM__ChorusFruitItem_H__*/
