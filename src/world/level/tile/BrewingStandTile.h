#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__BrewingStandTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__BrewingStandTile_H__

// v20: brewing stand block (tile id 117 — modern MC). An EntityTile
// like the furnace: using it opens the brewing screen. Rendering is a
// plain cube with the drawn stand texture (terrain.png cell (4,11)) —
// there is no custom non-cube tile renderer in this tree to build the
// modern 3D stand model with, so the cube + texture is the honest
// v20 shape (same precedent as the v10 enchanting table, which reuses
// bookshelf/obsidian textures on a cube).

#include "EntityTile.h"
#include "../../../util/Random.h"

class Level;
class Player;

class BrewingStandTile: public EntityTile
{
	typedef EntityTile super;
public:
	BrewingStandTile(int id);

	bool use(Level* level, int x, int y, int z, Player* player);
	int getResource(int data, Random* random);
	TileEntity* newTileEntity();
	void onRemove(Level* level, int x, int y, int z);
private:
	// Like FurnaceTile: a per-tile Random for the onRemove drop scatter.
	Random random;
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_TILE__BrewingStandTile_H__*/
