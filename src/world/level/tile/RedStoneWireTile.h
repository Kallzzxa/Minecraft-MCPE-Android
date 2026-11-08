#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__RedStoneWireTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__RedStoneWireTile_H__

// v11: redstone dust wire (tile id 55, modern MC id). A flat, non-solid
// slab that carries a simple ON/OFF signal to its direct neighbours
// (same level only — no up/down steps, no 0-15 signal levels).
//
// Power model (uses the tree's existing signal framework):
//   * getSignal() returns the powered bit, so Level::hasNeighborSignal()
//     sees a powered wire; levers / redstone torches / other powered
//     wires next to this one power it.
//   * getDirectSignal() deliberately returns FALSE: a powered wire must
//     not power the solid block underneath it, otherwise it would see
//     its own power reflected back through that block and latch ON
//     forever (Level::getSignal on a solid block answers via
//     hasDirectSignal, which queries this wire itself).
//   * neighborChanged() + setPlacedBy() recompute the powered bit from
//     the neighbours; setData() only notifies neighbours when the bit
//     actually changes, so propagation terminates.
//   * Breaking the wire drops 1 redstone dust (Item::redStone).
//
// Textures: 199 = wire off, 200 = wire on (row-12 cells of terrain.png
// that were unused magenta placeholders, same trick as v9's crops).

#include "Tile.h"
#include "../Level.h"
#include "../material/Material.h"
#include "../../item/Item.h"
#include "../../../util/Random.h"

class RedStoneWireTile : public Tile
{
	typedef Tile super;
public:
	static const int POWER_BIT = 1;

	RedStoneWireTile(int id, int texOff, int texOn)
	:	super(id, texOff, Material::decoration),
		texOff(texOff),
		texOn(texOn)
	{
		// Flat slab like TopSnowTile (1/16 high), no collision.
		setShape(0, 0, 0, 1, 1 / 16.0f, 1);
	}

	AABB* getAABB(Level* level, int x, int y, int z) {
		return NULL;
	}

	bool isSolidRender() {
		return false;
	}

	bool isCubeShaped() {
		return false;
	}

	int getTexture(int face, int data) {
		return (data & POWER_BIT) ? texOn : texOff;
	}

	bool mayPlace(Level* level, int x, int y, int z) {
		int t = level->getTile(x, y - 1, z);
		if (t == 0 || !Tile::tiles[t]->isSolidRender()) return false;
		return level->getMaterial(x, y - 1, z)->blocksMotion();
	}

	bool isSignalSource() {
		return true;
	}

	bool getSignal(LevelSource* level, int x, int y, int z) {
		return (level->getData(x, y, z) & POWER_BIT) != 0;
	}

	bool getSignal(LevelSource* level, int x, int y, int z, int dir) {
		return (level->getData(x, y, z) & POWER_BIT) != 0;
	}

	// See class comment: never transmit power *through* a block.
	bool getDirectSignal(Level* level, int x, int y, int z, int dir) {
		return false;
	}

	void neighborChanged(Level* level, int x, int y, int z, int type) {
		if (!checkCanSurvive(level, x, y, z)) return;
		updatePower(level, x, y, z);
	}

	void setPlacedBy(Level* level, int x, int y, int z, Mob* by) {
		// TileItem placement does not call neighborChanged on the placed
		// tile itself, so compute the initial state here (e.g. wire
		// placed next to an already-on lever must start powered).
		updatePower(level, x, y, z);
	}

	int getResource(int data, Random* random) {
		return Item::redStone->id;
	}

	int getResourceCount(Random* random) {
		return 1;
	}

private:
	int texOff;
	int texOn;

	void updatePower(Level* level, int x, int y, int z) {
		bool powered = level->hasNeighborSignal(x, y, z);
		int data = level->getData(x, y, z);
		if (((data & POWER_BIT) != 0) != powered) {
			// setData notifies the neighbours (tileUpdated), so the
			// change propagates wire by wire; no-op when unchanged.
			level->setData(x, y, z, powered ? POWER_BIT : 0);
		}
	}

	bool checkCanSurvive(Level* level, int x, int y, int z) {
		if (!mayPlace(level, x, y, z)) {
			this->spawnResources(level, x, y, z, level->getData(x, y, z));
			level->setTile(x, y, z, 0);
			return false;
		}
		return true;
	}
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_TILE__RedStoneWireTile_H__*/
