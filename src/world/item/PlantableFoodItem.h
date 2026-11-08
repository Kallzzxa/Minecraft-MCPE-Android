#ifndef NET_MINECRAFT_WORLD_ITEM__PlantableFoodItem_H__
#define NET_MINECRAFT_WORLD_ITEM__PlantableFoodItem_H__

//package net.minecraft.world.item;

#include "FoodItem.h"
#include "../level/tile/Tile.h"

// v9: a food that can also be planted on farmland, like a carrot or a
// potato in modern MC (the item itself is the seed — no separate seed item).
// Eating is untouched: FoodItem::use / useTimeDepleted still handle it.
// Planting follows SeedItem::useOn exactly: top face of farmland, empty
// block above, consumes one item.
class PlantableFoodItem : public FoodItem {
	typedef FoodItem super;
public:
	PlantableFoodItem(int id, int nutrition, bool isMeat, int resultTileId)
	:	super(id, nutrition, isMeat),
		resultId(resultTileId)
	{}

	bool useOn(ItemInstance* itemInstance, Player* player, Level* level, int x, int y, int z, int face, float clickX, float clickY, float clickZ) {
		if (face != 1) return false;
		int targetType = level->getTile(x, y, z);
		if (targetType == Tile::farmland->id && level->isEmptyTile(x, y + 1, z)) {
			level->setTile(x, y + 1, z, resultId);
			itemInstance->count--;
			return true;
		}
		return false;
	}
private:
	int resultId;
};

#endif /* NET_MINECRAFT_WORLD_ITEM__PlantableFoodItem_H__ */
