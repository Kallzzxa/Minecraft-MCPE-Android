#include "NetherLevelSource.h"

#include <string.h>
#include "../Level.h"
#include "../chunk/LevelChunk.h"
#include "../tile/Tile.h"
#include "../../entity/MobCategory.h"
#include "../../../util/Mth.h"

// v21: Nether terrain. See the header for the design; the numbers:
//  - floor height 28..62 (value noise), lava sea at y<=31, ceiling
//    bedrock at y=127. The player therefore spawns (portal arrival)
//    on a floor around y~45 with lava lakes below y=32 and glowstone
//    overhead — the classic Nether silhouette.

NetherLevelSource::NetherLevelSource(Level* level, long seed)
:	level(level),
	seed(seed)
{
}

/*static*/
float NetherLevelSource::hash01(int x, int y, int z, long seed)
{
	uint32_t h = (uint32_t)(x * 374761393u + y * 2246822519u + z * 668265263u)
		+ (uint32_t)(seed & 0x7fffffff) * 974634211u;
	h = (h ^ (h >> 13)) * 1274126177u;
	h = h ^ (h >> 16);
	return (float)(h & 0xffffff) / 16777216.0f;
}

/*static*/
float NetherLevelSource::noise2D(long seed, int x, int z)
{
	// Two octaves of bilinearly interpolated lattice hash noise.
	float total = 0.0f;
	float amp = 0.65f;
	float cell = 24.0f;
	for (int o = 0; o < 2; o++) {
		float fx = (float)x / cell;
		float fz = (float)z / cell;
		int x0 = Mth::floor(fx);
		int z0 = Mth::floor(fz);
		float tx = fx - (float)x0;
		float tz = fz - (float)z0;
		// smoothstep
		tx = tx * tx * (3 - 2 * tx);
		tz = tz * tz * (3 - 2 * tz);
		float v00 = hash01(x0, z0, seed + o * 101);
		float v10 = hash01(x0 + 1, z0, seed + o * 101);
		float v01 = hash01(x0, z0 + 1, seed + o * 101);
		float v11 = hash01(x0 + 1, z0 + 1, seed + o * 101);
		float v = (v00 * (1 - tx) + v10 * tx) * (1 - tz)
			+ (v01 * (1 - tx) + v11 * tx) * tz;
		total += v * amp;
		amp = 0.35f;
		cell = 6.0f;
	}
	return total; // ~0..1
}

LevelChunk* NetherLevelSource::getChunk(int xOffs, int zOffs)
{
	unsigned char* blocks = new unsigned char[LevelChunk::ChunkBlockCount];
	memset(blocks, 0, LevelChunk::ChunkBlockCount);

	const int bedrock = Tile::unbreakable->id;
	const int rack = Tile::netherrack->id;
	const int soul = Tile::hellSand->id;
	const int quartz = Tile::quartzOre->id;
	const int glow = Tile::lightGem->id;
	const int lava = Tile::calmLava->id;

	for (int lx = 0; lx < 16; lx++) {
		for (int lz = 0; lz < 16; lz++) {
			int wx = xOffs * 16 + lx;
			int wz = zOffs * 16 + lz;
			float n = noise2D(seed, wx, wz);
			float n2 = noise2D(seed + 12345, wx, wz);
			int h = 28 + (int)(n * 34); // floor height 28..62
			bool soulPatch = n2 > 0.78f;
			bool glowCluster = hash01(wx, wz, seed ^ 0x51f15e) > 0.965f;

			for (int y = 0; y < 128; y++) {
				int id = 0;
				if (y == 0) {
					id = bedrock;
				} else if (y < 5 && hash01(wx, y, wz, seed) < (5 - y) * 0.18f) {
					id = bedrock;
				} else if (y == 127) {
					id = bedrock;
				} else if (y == 126 && hash01(wx, y, wz, seed) < 0.5f) {
					id = bedrock;
				} else if (glowCluster && y >= 120 && y <= 125
						&& hash01(wx, y, wz, seed ^ 0x61057e) < 0.55f) {
					id = glow;
				} else if (y < h) {
					if (soulPatch && y >= h - 3) {
						id = soul;
					} else if (y >= 10 && y <= 110
							&& hash01(wx, y, wz, seed ^ 0x9a472) < 0.012f) {
						id = quartz;
					} else {
						id = rack;
					}
				} else if (y <= 31 && h <= 31) {
					// Lava sea where the floor dips below the sea level.
					id = lava;
				}
				if (id != 0) {
					blocks[lx << 11 | lz << 7 | y] = (unsigned char) id;
				}
			}
		}
	}

	LevelChunk* levelChunk = new LevelChunk(level, blocks, xOffs, zOffs);
	levelChunk->recalcHeightmap();
	return levelChunk;
}

Biome::MobList NetherLevelSource::getMobsAt(const MobCategory& mobCategory, int x, int y, int z)
{
	(void) x; (void) y; (void) z;
	Biome::MobList list;
	if (&mobCategory == &MobCategory::monster) {
		// Weights are relative (MobSpawner normalises against the
		// category total): pig zombies common, blazes uncommon,
		// ghasts rare and always alone.
		list.push_back(Biome::MobSpawnerData(MobTypes::PigZombie, 10, 1, 4));
		list.push_back(Biome::MobSpawnerData(MobTypes::Blaze, 6, 1, 2));
		list.push_back(Biome::MobSpawnerData(MobTypes::Ghast, 4, 1, 1));
	}
	return list;
}
