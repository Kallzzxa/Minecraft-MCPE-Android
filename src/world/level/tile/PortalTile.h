#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__PortalTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__PortalTile_H__

// v21: Nether portal block (id 90, modern MC).
// Tile::portalTile was DECLARED in Tile.h since the original tree but
// never defined or registered — there was no portal at all.
//
// Rendering follows WebTile: non-solid, no AABB, crossed quads
// (SHAPE_CROSS_TEXTURE) with the swirl texture (terrain.png 170), so
// it reads as a shimmering vertical plane, not a solid block.
// Material::portal already existed in Material.h.
//
// Teleporting does NOT happen here: Tile has no pointer to Minecraft,
// so LocalPlayer::tick detects standing in a portal block and asks
// Minecraft to switch dimensions (deferred to the tick start, so the
// level swap never happens mid-tick). This tile only (a) exists,
// (b) glows, (c) collapses when its obsidian frame is broken.

#include "Tile.h"
#include "../material/Material.h"

class PortalTile: public Tile
{
	typedef Tile super;
public:
	PortalTile(int id, int tex)
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
		return -1; // no drop: relight the frame instead
	}

	// If the frame is broken, the portal collapses. A portal block
	// survives if, in either the X or the Z plane, it has an obsidian
	// or portal neighbour horizontally AND vertically (edge blocks
	// touch obsidian; interior blocks touch portal on both sides).
	void neighborChanged(Level* level, int x, int y, int z, int type) {
		(void) type;
		if (!hasFrameNeighbour(level, x, y, z)) {
			level->setTile(x, y, z, 0);
		}
	}

private:
	static bool isFrameOrPortal(Level* level, int x, int y, int z) {
		int t = level->getTile(x, y, z);
		return t == Tile::obsidian->id || t == Tile::portalTile->id;
	}
	bool hasFrameNeighbour(Level* level, int x, int y, int z) {
		// X plane: left/right + up/down.
		if ((isFrameOrPortal(level, x - 1, y, z) || isFrameOrPortal(level, x + 1, y, z))
			&& (isFrameOrPortal(level, x, y - 1, z) || isFrameOrPortal(level, x, y + 1, z)))
			return true;
		// Z plane.
		if ((isFrameOrPortal(level, x, y, z - 1) || isFrameOrPortal(level, x, y, z + 1))
			&& (isFrameOrPortal(level, x, y - 1, z) || isFrameOrPortal(level, x, y + 1, z)))
			return true;
		return false;
	}
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_TILE__PortalTile_H__*/
