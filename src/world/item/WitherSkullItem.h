#ifndef NET_MINECRAFT_WORLD_ITEM__WitherSkullItem_H__
#define NET_MINECRAFT_WORLD_ITEM__WitherSkullItem_H__

// v34: placeable wither skull item (modern MC id 397). The v32
// skull was a plain Item (not placeable); this subclass places the
// skull tile (id 144) on the BedItem pattern — top face only, the
// cell above must be empty, the block below solid — and the ritual
// check runs from SkullTile::setPlacedBy, so it fires no matter
// which item placed the last skull.

#include "Item.h"

class WitherSkullItem: public Item
{
	typedef Item super;
public:
	WitherSkullItem(int id)
	:	super(id)
	{
	}

	bool useOn(ItemInstance* itemInstance, Player* player, Level* level, int x, int y, int z, int face, float clickX, float clickY, float clickZ);
};

#endif /*NET_MINECRAFT_WORLD_ITEM__WitherSkullItem_H__*/
