#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__EndPortalFrameTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__EndPortalFrameTile_H__

// v22: end portal frame (id 120, modern MC). A solid cube the
// player builds as a flat 5x5 ring (corners empty, 12 frames) and
// fills with eyes of ender — see EyeOfEnderItem::useOn, which sets
// data bit 0 on the tapped frame and fills the ring with the end
// portal tile once all 12 frames have eyes.
//
// Textures: data bit 0 clear -> end stone (terrain.png 30), so an
// empty frame reads as plain end stone; bit set -> the drawn frame
// texture with the green eye (terrain.png 153). Only two free
// terrain cells were left, so there is no separate empty-frame art.

#include "Tile.h"
#include "../material/Material.h"

class EndPortalFrameTile: public Tile
{
	typedef Tile super;
public:
	EndPortalFrameTile(int id, int tex)
	:	super(id, tex, Material::stone)
	{
	}

	static const int EYE_BIT = 1;

	int getTexture(int face, int data) {
		(void) face;
		return (data & EYE_BIT) ? 153 : 30;
	}

	int getTexture(LevelSource* level, int x, int y, int z, int face) {
		(void) face;
		return getTexture(0, level->getData(x, y, z));
	}
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_TILE__EndPortalFrameTile_H__*/
