#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__SkullTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__SkullTile_H__

// v34: wither skeleton skull tile (id 144, modern MC).
// Texture 252 (terrain.png): the cell was referenced ONLY by the
// never-generated placeholder tiles info_updateGame1/2 (248/249),
// so drawing the skull face over it changes nothing obtainable in
// normal play — same spirit as v9/v11 reusing placeholder cells.
// (Stage 2 polish: a proper small-head render shape instead of a
// full cube — SHAPE_BLOCK is what the ritual needs today.)
//
// Placement: the wither skull ITEM (id 397, WitherSkullItem) places
// this tile; the auto-registered TileItem (id 144) can place it too.
// Either way setPlacedBy runs the ritual check, so the wither
// summons no matter which item placed the last skull. Breaking the
// tile drops the skull item back (not the tile).

#include "Tile.h"
#include "../material/Material.h"
#include "../../item/Item.h"
#include "../../entity/monster/Wither.h"

class SkullTile: public Tile
{
	typedef Tile super;
public:
	SkullTile(int id, int tex)
	:	super(id, tex, Material::stone)
	{
	}

	// Breaking a placed skull drops the skull item back — the
	// default getResource would drop the tile itself (id 144).
	int getResource(int data, Random* random) {
		(void) data; (void) random;
		return Item::witherSkull->id;
	}

	// Ritual hook: runs when a skull is placed via any item path
	// (TileItem::useOn and WitherSkullItem::useOn both call
	// setPlacedBy after a successful placement).
	void setPlacedBy(Level* level, int x, int y, int z, Mob* by) {
		(void) by;
		Wither::trySummonFromSkull(level, x, y, z);
	}
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_TILE__SkullTile_H__*/
