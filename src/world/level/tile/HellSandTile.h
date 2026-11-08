#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__HellSandTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__HellSandTile_H__

// v21: soul sand (id 88, modern MC).
// The original tree had the registration COMMENTED OUT in Tile.cpp
// ("hellSand = (new HellSandTile(88, ...)") and no HellSandTile class
// at all; Tile::hellSand was declared in Tile.h but never defined.
// Texture 171 (terrain.png). Slows anything walking on it to 40%,
// like modern MC; nether wart grows on it (NetherWartTile).

#include "Tile.h"
#include "../material/Material.h"
#include "../../entity/Entity.h"

class HellSandTile: public Tile
{
	typedef Tile super;
public:
	HellSandTile(int id, int tex)
	:	super(id, tex, Material::sand)
	{
	}

	void entityInside(Level* level, int x, int y, int z, Entity* entity) {
		(void) level; (void) x; (void) y; (void) z;
		entity->xd *= 0.4f;
		entity->zd *= 0.4f;
	}
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_TILE__HellSandTile_H__*/
