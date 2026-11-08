#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__NetherWartTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__NetherWartTile_H__

// v21: nether wart crop (id 115, modern MC). The item existed (v20,
// from the reactor); farming it did not — there was no soul sand and
// no crop tile for it.
//
// A CropTile that grows on soul sand instead of farmland, in 4 stages
// (data 0..3, textures 173/216/217/231) instead of wheat's 8. Two
// deliberate deviations from CropTile:
//  - growth ignores light (CropTile::tick needs brightness >= 9 and
//    Bush::canSurvive needs brightness >= 8 or sky — the Nether has
//    neither). Wart grows slowly in the dark, like modern MC.
//  - spawnResources is Tile's plain version: CropTile's adds wheat
//    seeds, which would be wrong here.
// Drops: 1 wart when immature, 2-4 when mature (data 3). Planted by
// using a nether wart item on soul sand (SeedItem, see Item.cpp).

#include "CropTile.h"
#include "Bush.h"
#include "../../item/Item.h"

class NetherWartTile: public CropTile
{
	typedef CropTile super;
public:
	NetherWartTile(int id, int tex)
	:	super(id, tex)
	{
	}

	bool mayPlaceOn(int tile) {
		return tile == Tile::hellSand->id;
	}

	// Bush::canSurvive demands light or sky; wart needs only soul
	// sand below it.
	bool canSurvive(Level* level, int x, int y, int z) {
		return mayPlaceOn(level->getTile(x, y - 1, z));
	}

	void tick(Level* level, int x, int y, int z, Random* random) {
		// Bush::tick only (checkAlive): NOT CropTile::tick, which
		// gates growth on brightness >= MAX-6.
		Bush::tick(level, x, y, z, random);
		int age = level->getData(x, y, z);
		if (age < 3 && random->nextInt(6) == 0) {
			level->setData(x, y, z, age + 1);
		}
	}

	void growCropsToMax(Level* level, int x, int y, int z) {
		level->setData(x, y, z, 3);
	}

	int getTexture(LevelSource* level, int x, int y, int z, int face) {
		(void) face;
		int data = level->getData(x, y, z);
		if (data < 0) data = 3;
		if (data > 3) data = 3;
		return tex + data;
	}

	int getTexture(int face, int data) {
		(void) face;
		if (data < 0) data = 3;
		if (data > 3) data = 3;
		return tex + data;
	}

	void spawnResources(Level* level, int x, int y, int z, int data, float odds) {
		// Plain Tile drops (1 wart); no wheat seeds. Mature plants
		// (data 3) drop 1-3 extra warts, like modern MC's 2-4 total.
		Tile::spawnResources(level, x, y, z, data, odds);
		if (data >= 3) {
			int extra = 1 + level->random.nextInt(3);
			for (int i = 0; i < extra; i++) {
				popResource(level, x, y, z,
					ItemInstance(Item::netherWart->id, 1, 0));
			}
		}
	}

	int getResource(int data, Random* random) {
		(void) data; (void) random;
		return Item::netherWart->id;
	}

	int getResourceCount(Random* random) {
		(void) random;
		return 1;
	}
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_TILE__NetherWartTile_H__*/
