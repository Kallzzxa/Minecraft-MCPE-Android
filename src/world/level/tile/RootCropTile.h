#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__RootCropTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__RootCropTile_H__

#include "CropTile.h"
#include "../../entity/item/ItemEntity.h"

// v9: a wheat-like crop (CropTile growth on farmland, 8 data values) whose
// harvest is the plant itself — carrots and potatoes. Unlike wheat it drops
// no extra seeds: the crop IS the seed, like modern MC.
//
// Textures: only 4 visual stages (8 data values map 2:1 onto tex..tex+3).
// The cells used (carrots 184-187, potatoes 188-191, row 11 of terrain.png)
// were unused magenta placeholders — no tile, ItemRenderer mapper entry or
// dynamic texture references them.
class RootCropTile : public CropTile
{
	typedef CropTile super;
public:
	// yieldItemId is the FULL item id (256 + ctor id): carrot = 427,
	// potato = 428. Passed as a plain int because Tile::initTiles runs
	// before Item::initItems, so Item::carrot / Item::potato are still NULL
	// here (same reason CropTile only touches Item::seeds_wheat at harvest
	// time, never in its ctor).
	RootCropTile(int id, int tex, int yieldItemId)
	:	super(id, tex),
		yieldItemId(yieldItemId)
	{}

	int getTexture(LevelSource* level, int x, int y, int z, int face) {
		int data = level->getData(x, y, z);
		if (data < 0) data = 7;
		return tex + (data >> 1);
	}

	int getTexture(int face, int data) {
		if (data < 0) data = 7;
		return tex + (data >> 1);
	}

	// Do NOT inherit CropTile::spawnResources: it also drops wheat seeds.
	// A root crop drops only itself: 1-3 when mature (modern MC: 1-4),
	// 1 when broken immature so the plant is not lost.
	void spawnResources(Level* level, int x, int y, int z, int data, float odds) {
		if (level->isClientSide) {
			return;
		}
		int count = (data >= 7) ? 1 + level->random.nextInt(3) : 1;
		for (int i = 0; i < count; i++) {
			if (level->random.nextFloat() > odds) continue;
			float s = 0.7f;
			float xo = level->random.nextFloat() * s + (1 - s) * 0.5f;
			float yo = level->random.nextFloat() * s + (1 - s) * 0.5f;
			float zo = level->random.nextFloat() * s + (1 - s) * 0.5f;
			ItemEntity* item = new ItemEntity(level, float(x) + xo, float(y) + yo, float(z) + zo, ItemInstance(yieldItemId, 1, 0));
			item->throwTime = 10;
			level->addEntity(item);
		}
	}

	int getResource(int data, Random* random) {
		return yieldItemId;
	}

	int getResourceCount(Random* random) {
		return 1;
	}

private:
	int yieldItemId;
};

#endif /* NET_MINECRAFT_WORLD_LEVEL_TILE__RootCropTile_H__ */
