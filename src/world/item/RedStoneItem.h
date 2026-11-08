#ifndef NET_MINECRAFT_WORLD_ITEM__RedStoneItem_H__
#define NET_MINECRAFT_WORLD_ITEM__RedStoneItem_H__

// v11: redstone dust item (ctor id 75, full id 331 — the registration
// that was commented out in Item.cpp since the original tree).
// Using it on the TOP face of a block places a redstone wire tile on
// top of that block (same pattern as SeedItem/TilePlanterItem). The
// wire's setPlacedBy() computes its initial powered state, so dust
// placed next to a live lever / torch lights up immediately.
// Icon (8, 3) in gui/items.png is the vanilla redstone dust sprite.

#include "Item.h"
#include "ItemInstance.h"
#include "../entity/player/Player.h"
#include "../level/Level.h"
#include "../level/tile/Tile.h"
#include "../Facing.h"

class RedStoneItem : public Item
{
	typedef Item super;
public:
	RedStoneItem(int id)
	:	super(id)
	{
	}

	bool useOn(ItemInstance* instance, Player* player, Level* level, int x, int y, int z, int face, float clickX, float clickY, float clickZ) {
		if (face != Facing::UP) return false;
		if (instance->count == 0) return false;

		int tx = x, ty = y + 1, tz = z;
		if (!level->isEmptyTile(tx, ty, tz)) return false;
		if (!level->mayPlace(Tile::redStoneDust->id, tx, ty, tz, false, face)) return false;

		if (level->setTile(tx, ty, tz, Tile::redStoneDust->id)) {
			Tile::redStoneDust->setPlacedBy(level, tx, ty, tz, player);
			instance->count--;
			return true;
		}
		return false;
	}
};

#endif /*NET_MINECRAFT_WORLD_ITEM__RedStoneItem_H__*/
