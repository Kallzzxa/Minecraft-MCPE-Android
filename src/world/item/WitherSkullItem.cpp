#include "WitherSkullItem.h"

#include "../level/Level.h"
#include "../level/tile/Tile.h"
#include "../entity/player/Player.h"
#include "../Facing.h"

bool WitherSkullItem::useOn(ItemInstance* itemInstance, Player* player, Level* level, int x, int y, int z, int face, float clickX, float clickY, float clickZ)
{
	// v35: skulls mount on walls too (modern MC), not just on top
	// of a block — the target cell is the neighbour in the clicked
	// face's direction (the TileItem offset pattern), the support
	// is the clicked block itself, and the data value comes from
	// SkullTile::getPlacedOnFaceDataValue so this item and the
	// auto-registered TileItem agree. DOWN is still rejected (a
	// skull cannot hang from a ceiling). The ritual itself is
	// checked by SkullTile::setPlacedBy below — it only fires for
	// floor skulls (data 0).
	if (face == Facing::DOWN) {
		return false;
	}
	int tx = x, ty = y, tz = z;
	switch (face) {
		case Facing::UP:    ty++; break;
		case Facing::NORTH: tz--; break;
		case Facing::SOUTH: tz++; break;
		case Facing::WEST:  tx--; break;
		case Facing::EAST:  tx++; break;
		default: return false;
	}
	Tile* tile = Tile::tiles[Tile::skull->id];
	if (level->isEmptyTile(tx, ty, tz) && tile->mayPlace(level, tx, ty, tz, face)) {
		int data = tile->getPlacedOnFaceDataValue(level, tx, ty, tz, face, clickX, clickY, clickZ, 0);
		level->setTileAndData(tx, ty, tz, tile->id, data);
		// Double-check that the skull was successfully placed.
		if (level->getTile(tx, ty, tz) == tile->id) {
			tile->setPlacedBy(level, tx, ty, tz, player);
			level->playSound(tx + 0.5f, ty + 0.5f, tz + 0.5f, tile->soundType->getStepSound(), (tile->soundType->getVolume() + 1) / 2, tile->soundType->getPitch() * 0.8f);
			itemInstance->count--;
			return true;
		}
	}
	return false;
}
