#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__ChorusPlantTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__ChorusPlantTile_H__

// v22: chorus plant (id 240, modern MC). A CropTile that grows on
// end stone instead of farmland, in 5 stages (data 0..4) instead of
// wheat's 8. Like NetherWartTile it overrides the light gate:
// Bush::canSurvive / CropTile::tick demand brightness >= 8/9, and
// the End is permanent dusk — chorus grows slowly in the dark, like
// modern MC.
//
// Texture: a single cross texture (terrain.png 14) for every stage
// — only one free terrain cell was left, so stages differ in data
// (and drops) only, not in looks. Planted by using a chorus fruit
// on end stone (ChorusFruitItem::useOn, SeedItem pattern) and eaten
// like any food (FoodItem path + a random teleport on finishing).
//
// Drops: nothing while immature; a mature plant (data 4) drops 1-2
// chorus fruit. Modern MC grows chorus upward as a stalk with a
// chorus flower on top; this batch is a flat crop — no vertical
// growth, no chorus flower (see ROADMAP debts).

#include "CropTile.h"
#include "Bush.h"
#include "../../item/Item.h"

class ChorusPlantTile: public CropTile
{
	typedef CropTile super;
public:
	ChorusPlantTile(int id, int tex)
	:	super(id, tex)
	{
	}

	bool mayPlaceOn(int tile) {
		return tile == Tile::endStone->id;
	}

	// Bush::canSurvive demands light or sky; chorus needs only end
	// stone below it.
	bool canSurvive(Level* level, int x, int y, int z) {
		return mayPlaceOn(level->getTile(x, y - 1, z));
	}

	void tick(Level* level, int x, int y, int z, Random* random) {
		// Bush::tick only (checkAlive): NOT CropTile::tick, which
		// gates growth on brightness >= MAX-6.
		Bush::tick(level, x, y, z, random);
		int age = level->getData(x, y, z);
		if (age < 4 && random->nextInt(6) == 0) {
			level->setData(x, y, z, age + 1);
		}
	}

	void growCropsToMax(Level* level, int x, int y, int z) {
		level->setData(x, y, z, 4);
	}

	int getTexture(LevelSource* level, int x, int y, int z, int face) {
		(void) level; (void) x; (void) y; (void) z; (void) face;
		return tex;
	}

	int getTexture(int face, int data) {
		(void) face; (void) data;
		return tex;
	}

	void spawnResources(Level* level, int x, int y, int z, int data, float odds) {
		(void) odds;
		// Immature plants drop nothing; a mature plant drops 1-2
		// chorus fruit. Deliberately NOT Tile::spawnResources (that
		// would drop the plant block itself via getResource).
		if (data >= 4) {
			int count = 1 + level->random.nextInt(2);
			for (int i = 0; i < count; i++) {
				popResource(level, x, y, z,
					ItemInstance(Item::chorusFruit->id, 1, 0));
			}
		}
	}

	int getResource(int data, Random* random) {
		(void) data; (void) random;
		return Item::chorusFruit->id;
	}

	int getResourceCount(Random* random) {
		(void) random;
		return 0;
	}
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_TILE__ChorusPlantTile_H__*/
