#include "EndLevelSource.h"

#include <string.h>
#include "../Level.h"
#include "../chunk/LevelChunk.h"
#include "../tile/Tile.h"
#include "../../entity/MobCategory.h"
#include "../../../util/Mth.h"

// v22: End terrain. See the header for the design; the numbers:
//  - main island radius ~48, top y 62..66 (value noise), ~14 blocks
//    thick, void outside and below. The spawn platform (obsidian
//    5x5 at y=64 + bedrock ring at y=65) sits dead centre.

EndLevelSource::EndLevelSource(Level* level, long seed)
:	level(level),
	seed(seed)
{
}

/*static*/
const EndLevelSource::Pillar EndLevelSource::PILLARS[10] = {
	{ 20,   3, 10 }, { 19,  18, 16 }, {  6,  31, 22 },
	{ -9,  18, 12 }, { -23, 12, 18 }, { -32, -4, 24 },
	{ -14, -14,  9 }, { -5, -26, 14 }, { 14, -29, 20 },
	{ 18,  -9, 13 },
};

/*static*/
float EndLevelSource::hash01(int x, int y, int z, long seed)
{
	uint32_t h = (uint32_t)(x * 374761393u + y * 2246822519u + z * 668265263u)
		+ (uint32_t)(seed & 0x7fffffff) * 974634211u;
	h = (h ^ (h >> 13)) * 1274126177u;
	h = h ^ (h >> 16);
	return (float)(h & 0xffffff) / 16777216.0f;
}

/*static*/
float EndLevelSource::noise2D(long seed, int x, int z)
{
	// Two octaves of bilinearly interpolated lattice hash noise
	// (same shape as NetherLevelSource::noise2D, different scale).
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

/*static*/
bool EndLevelSource::isPillarTop(int x, int y, int z)
{
	for (int i = 0; i < 10; i++) {
		if (PILLARS[i].x == x && PILLARS[i].z == z
			&& 64 + PILLARS[i].h == y) return true;
	}
	return false;
}

/*static*/
bool EndLevelSource::getPillarTop(int i, int& x, int& y, int& z)
{
	if (i < 0 || i >= 10) return false;
	x = PILLARS[i].x;
	y = 64 + PILLARS[i].h;
	z = PILLARS[i].z;
	return true;
}

/*static*/
bool EndLevelSource::isExitRingCell(int x, int y, int z)
{
	if (y != EXIT_PORTAL_Y) return false;
	int ax = x < 0 ? -x : x;
	int az = z < 0 ? -z : z;
	int m = ax > az ? ax : az;
	return m == 1;
}

LevelChunk* EndLevelSource::getChunk(int xOffs, int zOffs)
{
	unsigned char* blocks = new unsigned char[LevelChunk::ChunkBlockCount];
	memset(blocks, 0, LevelChunk::ChunkBlockCount);

	const int endStone = Tile::endStone->id;
	const int obsidian = Tile::obsidian->id;
	const int bedrock = Tile::unbreakable->id;
	const int chorus = Tile::chorusPlant->id;

	for (int lx = 0; lx < 16; lx++) {
		for (int lz = 0; lz < 16; lz++) {
			int wx = xOffs * 16 + lx;
			int wz = zOffs * 16 + lz;
			float r = Mth::sqrt((float)(wx * wx + wz * wz));

			// Spawn platform: 5x5 obsidian at y=64, bedrock ring at
			// y=65, centre (0,65,0) air (the exit portal, filled by
			// the dragon's death). It overrides the island top.
			int ax = wx < 0 ? -wx : wx;
			int az = wz < 0 ? -wz : wz;
			bool inPlatform = ax <= 2 && az <= 2;
			int m = ax > az ? ax : az;
			// The 8 cells of the 3x3 ring around the centre.
			bool ringCell = inPlatform && m == 1;
			bool centreCell = (wx == 0 && wz == 0);
			(void) centreCell;

			int h;
			if (r > 48.0f) {
				// Outside the island: void. Pillars can still rise
				// here if one stands outside r=48 (none do: the
				// farthest pillar centre is at r=32 + 1 half-width).
				h = -1;
			} else if (inPlatform) {
				h = 64;
			} else {
				float n = noise2D(seed, wx, wz);
				// Gentle dome: full height to r=36, then the edge
				// falls away so the island reads as an island.
				float falloff = r > 36.0f ? (r - 36.0f) * 1.0f : 0.0f;
				h = 62 + (int)(n * 4) - (int)falloff;
			}

			// Pillar covering this column (3x3 around its centre).
			int pillarTop = -1;
			for (int i = 0; i < 10; i++) {
				int pdx = wx - PILLARS[i].x;
				int pdz = wz - PILLARS[i].z;
				if (pdx >= -1 && pdx <= 1 && pdz >= -1 && pdz <= 1) {
					pillarTop = 64 + PILLARS[i].h;
					break;
				}
			}

			for (int y = 0; y < 128; y++) {
				int id = 0;
				if (pillarTop > 0 && y >= 58 && y <= pillarTop) {
					// Obsidian shaft, single bedrock cap on top.
					id = (y == pillarTop) ? bedrock : obsidian;
				} else if (inPlatform && y == 64) {
					id = obsidian;
				} else if (ringCell && y == EXIT_PORTAL_Y) {
					id = bedrock;
				} else if (h >= 0 && y <= h && y >= h - 14) {
					id = endStone;
				}
				if (id != 0) {
					blocks[lx << 11 | lz << 7 | y] = (unsigned char) id;
				}
			}

			// Sparse chorus plant on the island top (not on the
			// platform, not at the island edge). Stage 0; it grows
			// by random tick.
			if (h >= 0 && !inPlatform && r >= 10.0f && r <= 44.0f
					&& hash01(wx, wz, seed ^ 0xc40a5) > 0.985f) {
				blocks[lx << 11 | lz << 7 | (h + 1)] = (unsigned char) chorus;
			}
		}
	}

	LevelChunk* levelChunk = new LevelChunk(level, blocks, xOffs, zOffs);
	levelChunk->recalcHeightmap();
	return levelChunk;
}

Biome::MobList EndLevelSource::getMobsAt(const MobCategory& mobCategory, int x, int y, int z)
{
	(void) x; (void) y; (void) z;
	Biome::MobList list;
	if (&mobCategory == &MobCategory::monster) {
		// Endermen only. The dragon is spawned once by
		// Minecraft::ensureEndDragon, never by the spawner.
		list.push_back(Biome::MobSpawnerData(MobTypes::Enderman, 10, 1, 1));
	}
	return list;
}
