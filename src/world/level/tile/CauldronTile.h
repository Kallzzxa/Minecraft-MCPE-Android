#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__CauldronTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__CauldronTile_H__

// v25: cauldron block (tile id 118 — modern MC; pays the v20 debt,
// and the swamp hut finally gets its cauldron). A plain Tile, not an
// EntityTile: the water level (0..3) fits in the data nibble, so no
// tile entity is needed — the level survives save/load like any other
// tile data, and chunk eviction cannot duplicate or lose it.
// Rendering is a plain cube with the drawn cauldron texture
// (terrain.png cell 250 — verified unused: not a tile tex, not in the
// ItemRenderer mapper, not a destroy stage (240..249), not a dynamic
// texture target); the top face shows the water texture
// (Tile::water->tex, cell 205) while the cauldron holds any water,
// so a full cauldron reads as full from above. Same cube+texture
// precedent as the v10 enchanting table / v20 brewing stand.
//
// Interactions (all in use(), which GameMode::useItemOn calls BEFORE
// the held item's useOn — so the bottle logic lives here, not in
// GlassBottleItem):
//   empty glass bottle + level > 0 -> take one level of water out
//       (bottle becomes a water bottle, modern MC);
//   water bottle (drinkable or splash — base aux WATER) + level < 3
//       -> pour one level in (bottle becomes an empty glass bottle);
//   anything else / empty hand / full / empty -> false, so the tap
//       falls through to the held item (or does nothing).
// There are NO buckets in this tree (Item::bucket_* are all commented
// out), so bottles are the only way in and out — that is the honest
// v25 scope, not a missing feature to fake. No dyeing / washing
// (modern MC cauldron extras): out of scope for this batch.

#include "Tile.h"
#include "../material/Material.h"

class Level;
class Player;

class CauldronTile: public Tile
{
	typedef Tile super;
public:
	// Water levels: EMPTY = 0, FULL = 3 (one bottle = one level).
	static const int MAX_LEVEL = 3;

	CauldronTile(int id);

	// Top face shows water while any is held; sides/bottom always
	// show the cauldron texture.
	int getTexture(int face, int data);

	bool use(Level* level, int x, int y, int z, Player* player);
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_TILE__CauldronTile_H__*/
