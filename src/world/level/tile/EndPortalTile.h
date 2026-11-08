#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__EndPortalTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__EndPortalTile_H__

// v22: End portal block (id 119, modern MC). The black starfield
// block (terrain.png 181) that fills a completed end portal frame
// in the Overworld and the exit portal in the End.
//
// Rendering follows PortalTile (v21): non-solid, no AABB, crossed
// quads (SHAPE_CROSS_TEXTURE), glows. Teleporting does NOT happen
// here: LocalPlayer::tick detects standing in an end portal block
// and asks Minecraft to switch dimensions (deferred to the tick
// start, like the Nether portal).
//
// Unlike PortalTile it has NO collapse logic: the exit portal's
// centre is ringed by bedrock, not obsidian, so a shared
// hasFrameNeighbour check would be wrong for it, and a frame-based
// check would need the ring geometry. Breaking an end portal frame
// therefore leaves a floating end portal behind — a known v22
// limitation, see ROADMAP.

#include "Tile.h"
#include "../material/Material.h"

class EndPortalTile: public Tile
{
	typedef Tile super;
public:
	EndPortalTile(int id, int tex)
	:	super(id, tex, Material::portal)
	{
	}

	bool isSolidRender() { return false; }
	AABB* getAABB(Level* level, int x, int y, int z) {
		(void) level; (void) x; (void) y; (void) z;
		return NULL;
	}
	int getRenderShape() { return Tile::SHAPE_CROSS_TEXTURE; }
	bool blocksLight() { return false; }
	bool isCubeShaped() { return false; }

	int getResource(int data, Random* random) {
		(void) data; (void) random;
		return -1; // no drop
	}
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_TILE__EndPortalTile_H__*/
