#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__SkullTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__SkullTile_H__

// v34: wither skeleton skull tile (id 144, modern MC).
// Texture 252 (terrain.png): the cell was referenced ONLY by the
// never-generated placeholder tiles info_updateGame1/2 (248/249),
// so drawing the skull face over it changes nothing obtainable in
// normal play — same spirit as v9/v11 reusing placeholder cells.
//
// v35: the skull is no longer a full cube. It renders as a small
// head (8x8x8 px, centred on the floor or hugged against a wall)
// via updateShape (the SignTile precedent: TileRenderer calls
// updateShape before rendering a SHAPE_BLOCK tile, Tile::clip
// calls it before the hit test, and getTileAABB below calls it
// before the collision box is built — so one override covers
// render + hit + collision). It is no longer solid-render, so
// neighbouring block faces are not culled and it blocks no light
// (Tile::init derives lightBlock from isSolidRender).
//
// Data values (face the skull is mounted on):
//   0 = floor (on top of a block — the ONLY value the ritual
//       accepts), 1 = north wall, 2 = south wall, 3 = west wall,
//   4 = east wall. "North wall" means the skull hangs on the
//       north face of a block, i.e. the supporting block is at
//       z+1 of the skull cell (TileItem offsets the target cell
//       by face before getPlacedOnFaceDataValue runs).
// Placement accepts floor + all four side faces (TorchTile /
// LeverTile precedent for the face->data mapping and the
// support check). A skull whose support is broken pops off
// (TorchTile's checkCanSurvive shape).
//
// Placement: the wither skull ITEM (id 397, WitherSkullItem) and
// the auto-registered TileItem (id 144) both place this tile.
// Either way setPlacedBy runs the ritual check, so the wither
// summons no matter which item placed the last skull — but ONLY
// for floor skulls (data 0): a wall skull can never complete a
// ritual, and trySummonFromSkull additionally requires data 0 on
// all three skulls, so a wall-mounted row can never
// false-positive. Breaking the tile drops the skull item back
// (not the tile).

#include "Tile.h"
#include "../material/Material.h"
#include "../../item/Item.h"
#include "../../entity/monster/Wither.h"
#include "../../Facing.h"

class SkullTile: public Tile
{
	typedef Tile super;
public:
	// Data values: which face the skull is mounted on.
	static const int DATA_FLOOR = 0;
	static const int DATA_NORTH = 1; // support at z+1
	static const int DATA_SOUTH = 2; // support at z-1
	static const int DATA_WEST  = 3; // support at x+1
	static const int DATA_EAST  = 4; // support at x-1

	SkullTile(int id, int tex)
	:	super(id, tex, Material::stone)
	{
		// Default shape is the floor head; updateShape() refines
		// it per data value before every render / hit / collision.
		setShape(0.25f, 0.0f, 0.25f, 0.75f, 0.5f, 0.75f);
	}

	// A small head, not a cube: neighbours keep their faces and
	// light passes (Tile::init sets lightBlock 0 for this).
	bool isSolidRender() {
		return false;
	}

	bool isCubeShaped() {
		return false;
	}

	// Small head (8x8x8 px): floor = centred, bottom half of the
	// cell; wall = same size, hugged against its supporting wall
	// and floating mid-cell vertically (modern MC skulls sit
	// centred on the wall face).
	void updateShape(LevelSource* level, int x, int y, int z) {
		int data = level->getData(x, y, z) & 7;
		switch (data) {
			case DATA_NORTH: // support at z+1, hug the +z side
				setShape(0.25f, 0.25f, 0.5f, 0.75f, 0.75f, 1.0f);
				break;
			case DATA_SOUTH: // support at z-1, hug the -z side
				setShape(0.25f, 0.25f, 0.0f, 0.75f, 0.75f, 0.5f);
				break;
			case DATA_WEST: // support at x+1, hug the +x side
				setShape(0.5f, 0.25f, 0.25f, 1.0f, 0.75f, 0.75f);
				break;
			case DATA_EAST: // support at x-1, hug the -x side
				setShape(0.0f, 0.25f, 0.25f, 0.5f, 0.75f, 0.75f);
				break;
			default: // floor
				setShape(0.25f, 0.0f, 0.25f, 0.75f, 0.5f, 0.75f);
				break;
		}
	}

	// SignTile precedent: the collision box must use the
	// per-data shape, not whatever the shared Tile object
	// last rendered with.
	AABB getTileAABB(Level* level, int x, int y, int z) {
		updateShape(level, x, y, z);
		return super::getTileAABB(level, x, y, z);
	}

	// Face-aware support check (TorchTile / LeverTile shape).
	// (x, y, z) is the TARGET cell; the support is the block
	// the face was clicked on (below for UP, the face neighbour
	// for a side face).
	bool mayPlace(Level* level, int x, int y, int z, unsigned char face) {
		switch (face) {
			case Facing::UP:
				return level->isSolidBlockingTile(x, y - 1, z);
			case Facing::NORTH:
				return level->isSolidBlockingTile(x, y, z + 1);
			case Facing::SOUTH:
				return level->isSolidBlockingTile(x, y, z - 1);
			case Facing::WEST:
				return level->isSolidBlockingTile(x + 1, y, z);
			case Facing::EAST:
				return level->isSolidBlockingTile(x - 1, y, z);
			default:
				return false;
		}
	}

	// Face -> data value. The TileItem path and WitherSkullItem
	// both call this, so both items agree on the mapping.
	int getPlacedOnFaceDataValue(Level* level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, int itemValue) {
		(void) level; (void) x; (void) y; (void) z;
		(void) clickX; (void) clickY; (void) clickZ;
		switch (face) {
			case Facing::UP:    return DATA_FLOOR;
			case Facing::NORTH: return DATA_NORTH;
			case Facing::SOUTH: return DATA_SOUTH;
			case Facing::WEST:  return DATA_WEST;
			case Facing::EAST:  return DATA_EAST;
			default:            return itemValue;
		}
	}

	// Pop off (dropping the skull item) when the supporting
	// block goes away — same shape as TorchTile's
	// checkCanSurvive. Runs on any neighbour change; the ritual
	// is safe because it removes all three skulls BEFORE the
	// soul sand below them, so no floor skull is ever left
	// standing on removed support.
	void neighborChanged(Level* level, int x, int y, int z, int type) {
		(void) type;
		if (level->getTile(x, y, z) != id) return;
		int data = level->getData(x, y, z) & 7;
		bool supported = false;
		switch (data) {
			case DATA_NORTH: supported = level->isSolidBlockingTile(x, y, z + 1); break;
			case DATA_SOUTH: supported = level->isSolidBlockingTile(x, y, z - 1); break;
			case DATA_WEST:  supported = level->isSolidBlockingTile(x + 1, y, z); break;
			case DATA_EAST:  supported = level->isSolidBlockingTile(x - 1, y, z); break;
			default:         supported = level->isSolidBlockingTile(x, y - 1, z); break;
		}
		if (!supported) {
			spawnResources(level, x, y, z, data);
			level->setTile(x, y, z, 0);
		}
	}

	// Breaking a placed skull drops the skull item back — the
	// default getResource would drop the tile itself (id 144).
	int getResource(int data, Random* random) {
		(void) data; (void) random;
		return Item::witherSkull->id;
	}

	// Ritual hook: runs when a skull is placed via any item path
	// (TileItem::useOn and WitherSkullItem::useOn both call
	// setPlacedBy after a successful placement) — but ONLY for
	// floor skulls (data 0). A wall skull is decoration; it must
	// never complete a ritual.
	void setPlacedBy(Level* level, int x, int y, int z, Mob* by) {
		(void) by;
		if (level->getData(x, y, z) == DATA_FLOOR) {
			Wither::trySummonFromSkull(level, x, y, z);
		}
	}
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_TILE__SkullTile_H__*/
