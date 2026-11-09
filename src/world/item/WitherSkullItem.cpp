#include "WitherSkullItem.h"

#include "../level/Level.h"
#include "../level/tile/Tile.h"
#include "../entity/player/Player.h"
#include "../Facing.h"

bool WitherSkullItem::useOn(ItemInstance* itemInstance, Player* player, Level* level, int x, int y, int z, int face, float clickX, float clickY, float clickZ)
{
	(void) clickX; (void) clickY; (void) clickZ;
	// Skulls sit on TOP of a block (the ritual needs them on top
	// of soul sand); wall mounting is a later polish.
	if (face != Facing::UP) {
		return false;
	}
	y += 1;
	// BedItem pattern: the cell must be empty and the block below
	// solid. The ritual itself is checked by
	// SkullTile::setPlacedBy below (it also covers the
	// auto-registered TileItem path).
	if (level->isEmptyTile(x, y, z) && level->isSolidBlockingTile(x, y - 1, z)) {
		Tile* tile = Tile::tiles[Tile::skull->id];
		level->setTileAndData(x, y, z, tile->id, 0);
		// Double-check that the skull was successfully placed.
		if (level->getTile(x, y, z) == tile->id) {
			tile->setPlacedBy(level, x, y, z, player);
			level->playSound(x + 0.5f, y + 0.5f, z + 0.5f, tile->soundType->getStepSound(), (tile->soundType->getVolume() + 1) / 2, tile->soundType->getPitch() * 0.8f);
			itemInstance->count--;
			return true;
		}
	}
	return false;
}
