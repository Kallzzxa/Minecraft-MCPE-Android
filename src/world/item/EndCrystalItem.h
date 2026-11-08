#ifndef NET_MINECRAFT_WORLD_ITEM__EndCrystalItem_H__
#define NET_MINECRAFT_WORLD_ITEM__EndCrystalItem_H__

// v27: end crystal item (modern MC id 426 = ctor 170 + 256).
// Crafted shaped: 7 glass + 1 eye of ender + 1 ghast tear
// (Recipes.cpp — "GGG" / "GEG" / "GTG"). Placed only in the End,
// only on top (face 1) of bedrock or obsidian, with 2 air above —
// the pillar bedrock caps and the exit-portal bedrock ring are the
// respawn spots (see EndCrystal::isOnRespawnSpot). One crystal per
// spot (a second placement within 1 block of an existing crystal is
// refused). The crystal itself is the EndCrystal entity; breaking
// it (punch / arrow) detonates it, so there is no block form and no
// pickup.

#include "Item.h"
#include "../level/Level.h"
#include "../level/tile/Tile.h"
#include "../level/dimension/Dimension.h"
#include "../entity/player/Player.h"
#include "../entity/item/EndCrystal.h"
#include "../entity/EntityTypes.h"

class EndCrystalItem: public Item
{
	typedef Item super;
public:
	EndCrystalItem(int id)
	:	super(id)
	{
	}

	bool useOn(ItemInstance* instance, Player* player, Level* level, int x, int y, int z, int face, float clickX, float clickY, float clickZ) {
		(void) clickX; (void) clickY; (void) clickZ;
		if (level->isClientSide) return false;
		if (level->dimension == NULL
			|| level->dimension->id != Dimension::THE_END) return false;
		if (face != 1) return false; // only on top of a block
		int base = level->getTile(x, y, z);
		if (base != Tile::unbreakable->id
			&& base != Tile::obsidian->id) return false;
		// The crystal is 2 blocks tall: both blocks above must be air.
		if (level->getTile(x, y + 1, z) != 0
			|| level->getTile(x, y + 2, z) != 0) return false;
		// One crystal per spot.
		for (unsigned int i = 0; i < level->entities.size(); i++) {
			Entity* e = level->entities[i];
			if (e == NULL || e->removed
				|| e->getEntityTypeId() != EntityTypes::IdEndCrystal) continue;
			float dx = e->x - (x + 0.5f);
			float dz = e->z - (z + 0.5f);
			if (dx * dx + dz * dz < 4.0f) return false;
		}
		level->addEntity(new EndCrystal(level, x + 0.5f, (float)(y + 1), z + 0.5f));
		if (!player->abilities.instabuild) instance->count--;
		level->playSound(x + 0.5, y + 1.5, z + 0.5, "random.pop", 1, 1.0f);
		return true;
	}
};

#endif /*NET_MINECRAFT_WORLD_ITEM__EndCrystalItem_H__*/
