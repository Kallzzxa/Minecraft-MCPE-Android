#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__LeverTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__LeverTile_H__

// v11: lever (tile id 69, modern MC id). Tap it to toggle power.
// Data: low 3 bits = facing (1-5, same as TorchTile), bit 3 (value 8)
// = ON. Toggling uses Level::setData, which notifies the neighbours
// (tileUpdated), so adjacent wire / lamps / TNT / doors see the change
// through the existing hasNeighborSignal framework.
//
// Rendering reuses SHAPE_TORCH (the renderer draws a thin stick);
// texture 215 is a lever drawn into a row-13 terrain.png cell that
// was an unused magenta placeholder. TileRenderer::tesselateTorchInWorld
// now masks the data with & 7 so the ON bit does not break the
// wall-mount orientation (a no-op for normal torches, whose data is
// always 0-5).

#include "Tile.h"
#include "../Level.h"
#include "../material/Material.h"
#include "LevelEvent.h"

class LeverTile : public Tile
{
	typedef Tile super;
public:
	static const int ON_BIT = 8;

	LeverTile(int id, int tex)
	:	super(id, tex, Material::decoration)
	{
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

	int getRenderShape() {
		return Tile::SHAPE_TORCH;
	}

	int getRenderLayer() {
		return Tile::RENDERLAYER_ALPHATEST;
	}

	bool mayPlace(Level* level, int x, int y, int z) {
		if (level->isSolidBlockingTile(x - 1, y, z)) {
			return true;
		} else if (level->isSolidBlockingTile(x + 1, y, z)) {
			return true;
		} else if (level->isSolidBlockingTile(x, y, z - 1)) {
			return true;
		} else if (level->isSolidBlockingTile(x, y, z + 1)) {
			return true;
		} else if (isConnection(level, x, y - 1, z)) {
			return true;
		}
		return false;
	}

	// Facing values: 1 = up (floor), 2 = north wall, 3 = south wall,
	// 4 = west wall, 5 = east wall — same convention as TorchTile.
	int getPlacedOnFaceDataValue(Level* level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, int itemValue)
	{
		int dir = itemValue;

		if (face == 1 && isConnection(level, x, y - 1, z)) dir = 5;
		if (face == 2 && level->isSolidBlockingTile(x, y, z + 1)) dir = 4;
		if (face == 3 && level->isSolidBlockingTile(x, y, z - 1)) dir = 3;
		if (face == 4 && level->isSolidBlockingTile(x + 1, y, z)) dir = 2;
		if (face == 5 && level->isSolidBlockingTile(x - 1, y, z)) dir = 1;

		return dir;
	}

	bool use(Level* level, int x, int y, int z, Player* player) {
		int data = level->getData(x, y, z);
		level->setData(x, y, z, data ^ ON_BIT);
		level->levelEvent(player, LevelEvent::SOUND_CLICK, x, y, z, 0);
		return true;
	}

	void neighborChanged(Level* level, int x, int y, int z, int type) {
		checkCanSurvive(level, x, y, z);
	}

	bool isSignalSource() {
		return true;
	}

	bool getSignal(LevelSource* level, int x, int y, int z) {
		return (level->getData(x, y, z) & ON_BIT) != 0;
	}

	bool getSignal(LevelSource* level, int x, int y, int z, int dir) {
		return (level->getData(x, y, z) & ON_BIT) != 0;
	}

	bool getDirectSignal(Level* level, int x, int y, int z, int dir) {
		return (level->getData(x, y, z) & ON_BIT) != 0;
	}

private:
	bool isConnection(Level* level, int x, int y, int z) {
		if (level->isSolidBlockingTile(x, y, z)) {
			return true;
		}
		int tile = level->getTile(x, y, z);
		if (tile == Tile::fence->id || tile == Tile::glass->id) {
			return true;
		}
		return false;
	}

	bool checkCanSurvive(Level* level, int x, int y, int z) {
		if (!mayPlace(level, x, y, z)) {
			if (level->getTile(x, y, z) == id) {
				spawnResources(level, x, y, z, level->getData(x, y, z));
				level->setTile(x, y, z, 0);
			}
			return false;
		}
		return true;
	}
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_TILE__LeverTile_H__*/
