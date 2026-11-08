#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__RedstoneLampTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__RedstoneLampTile_H__

// v11: redstone lamp (tiles 123 off / 124 on, modern MC ids), following
// the furnace / redstone-ore two-tile pattern: the OFF tile swaps
// itself for the ON tile when it gains power and back when it loses
// it. State is recomputed in neighborChanged() and in setPlacedBy()
// (TileItem placement does not call neighborChanged on the placed tile
// itself, so a lamp placed next to live wire must light up at once).
//
// The lamp is a consumer, not a source, so swapping tiles cannot
// feed power back into the circuit. Breaking a lit lamp drops the
// unlit lamp (getResource always returns the OFF id).
//
// Textures: 201 = lamp off, 202 = lamp on (row-12 terrain.png cells
// that were unused magenta placeholders, same trick as v9).

#include "Tile.h"
#include "../Level.h"
#include "../material/Material.h"
#include "../../../util/Random.h"

class RedstoneLampTile : public Tile
{
	typedef Tile super;
public:
	RedstoneLampTile(int id, int tex, bool lit)
	:	super(id, tex, Material::stone),
		lit(lit)
	{
	}

	void neighborChanged(Level* level, int x, int y, int z, int type) {
		updateLit(level, x, y, z);
	}

	void setPlacedBy(Level* level, int x, int y, int z, Mob* by) {
		updateLit(level, x, y, z);
	}

	int getResource(int data, Random* random) {
		return Tile::redstoneLamp->id;
	}

private:
	bool lit;

	void updateLit(Level* level, int x, int y, int z) {
		bool powered = level->hasNeighborSignal(x, y, z);
		if (powered && !lit) {
			level->setTile(x, y, z, Tile::redstoneLamp_lit->id);
		} else if (!powered && lit) {
			level->setTile(x, y, z, Tile::redstoneLamp->id);
		}
	}
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_TILE__RedstoneLampTile_H__*/
