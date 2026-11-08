#include "SwampHutFeature.h"

#include "../Level.h"
#include "../chunk/LevelChunk.h"
#include "../biome/Biome.h"
#include "../tile/Tile.h"
#include "../tile/CauldronTile.h"
#include "../dimension/Dimension.h"
#include "../../entity/monster/Witch.h"
#include "../../entity/EntityTypes.h"
#include "../../phys/AABB.h"
#include "../../../util/Random.h"
#include "../../../util/Mth.h"
#include <stdint.h>

// ---------------------------------------------------------------------------
// Deterministic hut placement (same pattern as VillageFeature)
// ---------------------------------------------------------------------------

static int floorDiv(int a, int n)
{
	return a >= 0 ? a / n : -((-a + n - 1) / n);
}

// splitmix64 finaliser over (seed, a, b) — deterministic across runs
// and platforms. Different salt from the village hash so hut centres
// do not line up with village centres.
static uint64_t hutHash(long seed, int a, int b)
{
	uint64_t h = (uint64_t)(seed + 0x2545F4914F6CDD1DULL)
		^ ((uint64_t)(uint32_t)a * 0xBF58476D1CE4E5B9ULL)
		^ ((uint64_t)(uint32_t)b * 0x94D049BB133111EBULL);
	h ^= h >> 30;
	h *= 0xBF58476D1CE4E5B9ULL;
	h ^= h >> 27;
	h *= 0x94D049BB133111EBULL;
	h ^= h >> 31;
	return h;
}

void SwampHutFeature::getHutCenterChunk(long seed, int gx, int gz, int& outChunkX, int& outChunkZ)
{
	uint64_t h = hutHash(seed, gx, gz);
	outChunkX = gx * GRID_CELLS + (int)(h % (uint64_t)GRID_CELLS);
	outChunkZ = gz * GRID_CELLS + (int)((h >> 32) % (uint64_t)GRID_CELLS);
}

bool SwampHutFeature::isHutCenterChunk(long seed, int chunkX, int chunkZ, int& outCenterX, int& outCenterZ)
{
	int gx = floorDiv(chunkX, GRID_CELLS);
	int gz = floorDiv(chunkZ, GRID_CELLS);
	int hcx, hcz;
	getHutCenterChunk(seed, gx, gz, hcx, hcz);
	if (hcx != chunkX || hcz != chunkZ) return false;
	outCenterX = hcx * 16 + 8;
	outCenterZ = hcz * 16 + 8;
	return true;
}

// ---------------------------------------------------------------------------
// Clipped block placement (never writes outside the chunk being generated)
// ---------------------------------------------------------------------------

static bool inChunk(int cx, int cz, int x, int z)
{
	return x >= cx * 16 && x < cx * 16 + 16 && z >= cz * 16 && z < cz * 16 + 16;
}

static void setH(Level* level, int cx, int cz, int x, int y, int z, int id)
{
	if (y < 1 || y >= 127) return;
	if (!inChunk(cx, cz, x, z)) return;
	level->setTile(x, y, z, id);
}

// Like setH, but also sets the tile data (used for the cauldron's
// water level — v25: the hut cauldron starts full, data 3).
static void setHD(Level* level, int cx, int cz, int x, int y, int z, int id, int data)
{
	if (y < 1 || y >= 127) return;
	if (!inChunk(cx, cz, x, z)) return;
	level->setTileAndData(x, y, z, id, data);
}

// ---------------------------------------------------------------------------
// The hut itself
// ---------------------------------------------------------------------------
//
// Layout (centre hx, hz; floor at floorY = groundY + 3):
//   stilts: logs at the 4 corners, ground -> floor
//   floor:  7x5 planks at floorY
//   walls:  logs at corners + planks, floorY+1 .. floorY+3,
//           doorway (1 wide, 2 high) on the south wall, one glass
//           window on the north wall
//   roof:   planks at floorY+4 with a 1-block overhang (9x7)
//   inside: a workbench on the floor + a full cauldron (v25 —
//           the v20 debt is paid, the hut finally has one).

static void placeHut(Level* level, int chunkX, int chunkZ, int hx, int hz, int groundY)
{
	int floorY = groundY + 3;
	int woodId = Tile::wood->id;
	int logId = Tile::treeTrunk->id;
	int glassId = Tile::glass->id;

	// Stilts: one log column per corner, from the local ground up
	// to just under the floor (per-corner top solid block, so a
	// stilt over water reaches the lakebed instead of floating).
	int corners[4][2] = { { -3, -2 }, { 3, -2 }, { -3, 2 }, { 3, 2 } };
	for (int i = 0; i < 4; i++) {
		int sx = hx + corners[i][0];
		int sz = hz + corners[i][1];
		int y0 = level->getTopSolidBlock(sx, sz);
		if (y0 < 1) y0 = groundY;
		if (y0 > floorY - 1) y0 = floorY - 1;
		for (int y = y0; y < floorY; y++) setH(level, chunkX, chunkZ, sx, y, sz, logId);
	}

	// Floor: 7x5 planks.
	for (int x = hx - 3; x <= hx + 3; x++) {
		for (int z = hz - 2; z <= hz + 2; z++) {
			setH(level, chunkX, chunkZ, x, floorY, z, woodId);
		}
	}

	// Walls: 3 high. Corners are logs, the rest planks; the south
	// wall has a doorway at x = hx (floorY+1..floorY+2) and the
	// north wall a glass window at x = hx (floorY+2).
	for (int y = floorY + 1; y <= floorY + 3; y++) {
		for (int x = hx - 3; x <= hx + 3; x++) {
			for (int z = hz - 2; z <= hz + 2; z++) {
				bool perimeter = (x == hx - 3 || x == hx + 3 || z == hz - 2 || z == hz + 2);
				if (!perimeter) continue;
				bool corner = ((x == hx - 3 || x == hx + 3) && (z == hz - 2 || z == hz + 2));
				if (z == hz + 2 && x == hx && (y == floorY + 1 || y == floorY + 2)) continue; // doorway
				if (z == hz - 2 && x == hx && y == floorY + 2) {
					setH(level, chunkX, chunkZ, x, y, z, glassId); // window
					continue;
				}
				setH(level, chunkX, chunkZ, x, y, z, corner ? logId : woodId);
			}
		}
	}

	// Roof: planks with a 1-block overhang.
	for (int x = hx - 4; x <= hx + 4; x++) {
		for (int z = hz - 3; z <= hz + 3; z++) {
			setH(level, chunkX, chunkZ, x, floorY + 4, z, woodId);
		}
	}

	// Workbench inside, against the west wall; cauldron (v25, full
	// of water) against the east wall.
	setH(level, chunkX, chunkZ, hx - 2, floorY + 1, hz - 1, Tile::workBench->id);
	setHD(level, chunkX, chunkZ, hx + 2, floorY + 1, hz - 1, Tile::cauldron->id, CauldronTile::MAX_LEVEL);
}

void SwampHutFeature::tryPlaceSwampHut(Level* level, int chunkX, int chunkZ)
{
	if (level == NULL) return;
	long seed = level->getSeed();
	int R = HUT_RADIUS;
	int gx0 = floorDiv(chunkX, GRID_CELLS);
	int gz0 = floorDiv(chunkZ, GRID_CELLS);
	for (int gx = gx0 - 1; gx <= gx0 + 1; ++gx) {
		for (int gz = gz0 - 1; gz <= gz0 + 1; ++gz) {
			int hcx, hcz;
			getHutCenterChunk(seed, gx, gz, hcx, hcz);
			int wx = hcx * 16 + 8;
			int wz = hcz * 16 + 8;
			// Does this hut's footprint touch this chunk at all?
			// (Roof overhang is 1 block wider than the radius, so
			// check with R + 1.)
			if (wx + R + 1 < chunkX * 16 || wx - R - 1 > chunkX * 16 + 15) continue;
			if (wz + R + 1 < chunkZ * 16 || wz - R - 1 > chunkZ * 16 + 15) continue;
			// Swampland only, on ground at height >= 62 (swamp
			// lowlands; the stilts cover shallow water).
			Biome* biome = level->getBiome(wx, wz);
			if (biome != Biome::swampland) continue;
			int groundY = level->getHeightmap(wx, wz);
			if (groundY < 62) continue;
			placeHut(level, chunkX, chunkZ, wx, wz, groundY);
		}
	}
}

// ---------------------------------------------------------------------------
// The hut's witch
// ---------------------------------------------------------------------------

void SwampHutFeature::ensureWitch(Level* level, int chunkX, int chunkZ)
{
	if (level == NULL) return;
	// Single-player overworld only: the client cache must not spawn
	// entities (server-authoritative), and the Nether/End have their
	// own levels with no huts.
	if (level->isClientSide) return;
	if (level->dimension == NULL || level->dimension->id != Dimension::NORMAL) return;

	long seed = level->getSeed();
	int wx, wz;
	if (!isHutCenterChunk(seed, chunkX, chunkZ, wx, wz)) return;
	Biome* biome = level->getBiome(wx, wz);
	if (biome != Biome::swampland) return;
	// Only spawn once this chunk's terrain exists: on first
	// generation postProcess may be deferred (neighbours missing),
	// and the hut blocks do not exist yet — spawning now would
	// leave the witch on the ground (or in the water) under the
	// hut instead of inside it. The ChunkCache postProcess hook
	// re-runs this once the terrain is populated.
	LevelChunk* chunk = level->getChunk(chunkX, chunkZ);
	if (chunk == NULL || !chunk->terrainPopulated) return;
	int groundY = level->getHeightmap(wx, wz);
	if (groundY < 62) return;
	int floorY = groundY + 3;

	// Count witches already near this hut and top up to 1.
	AABB box((float)(wx - 8), 0.0f, (float)(wz - 8),
		(float)(wx + 8), 128.0f, (float)(wz + 8));
	EntityList list;
	int have = level->getEntitiesOfType(MobTypes::Witch, box, list);
	if (have >= 1) return;

	Witch* w = new Witch(level);
	w->moveTo((float)wx + 0.5f, (float)floorY + 1.0f, (float)wz + 0.5f,
		level->random.nextFloat() * 360.0f, 0.0f);
	level->addEntity(w);
}
