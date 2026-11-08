#ifndef NET_MINECRAFT_WORLD_ITEM__GlassBottleItem_H__
#define NET_MINECRAFT_WORLD_ITEM__GlassBottleItem_H__

// v20: glass bottle (ctor 118, full id 374 — modern MC). Crafted from
// 3 glass (see Recipes.cpp). Fill it with water to get a water bottle
// (potion aux 0), the base of every brew. Two ways in, because either
// tap path may or may not hit the water block on this old client:
//  - useOn: tap directly on a water block (GameMode::useItemOn path);
//  - use: tap while standing in water (GameMode::useItem path).
// Filling does not remove the water (modern MC behaviour — water
// sources are infinite).

#include "Item.h"
#include "ItemInstance.h"
#include "PotionItem.h"
#include "../entity/player/Player.h"
#include "../level/Level.h"
#include "../level/tile/Tile.h"

class GlassBottleItem: public Item
{
	typedef Item super;
public:
	GlassBottleItem(int id)
	:	super(id)
	{
	}

	bool useOn(ItemInstance* instance, Player* player, Level* level, int x, int y, int z, int face, float clickX, float clickY, float clickZ) {
		int tile = level->getTile(x, y, z);
		if (tile == Tile::water->id || tile == Tile::calmWater->id)
			return fillBottle(instance, player);
		return false;
	}

	ItemInstance* use(ItemInstance* instance, Level* level, Player* player) {
		if (player->isInWater())
			fillBottle(instance, player);
		return instance;
	}

private:
	bool fillBottle(ItemInstance* instance, Player* player) {
		if (instance->count <= 0) return false;
		instance->count--;
		ItemInstance water(Item::potion, 1, PotionItem::WATER);
		if (!player->inventory->add(&water))
			player->drop(new ItemInstance(water), false);
		return true;
	}
};

#endif /*NET_MINECRAFT_WORLD_ITEM__GlassBottleItem_H__*/
