#include "VillageFeature.h"

#include "../Level.h"
#include "../chunk/LevelChunk.h"
#include "../biome/Biome.h"
#include "../tile/Tile.h"
#include "../dimension/Dimension.h"
#include "../../entity/animal/Villager.h"
#include "../../entity/EntityTypes.h"
#include "../../phys/AABB.h"
#include "../../../util/Random.h"
#include "../../../util/Mth.h"
#include <stdint.h>

// ---------------------------------------------------------------------------
// Deterministic village placement
// ---------------------------------------------------------------------------

static int floorDiv(int a, int n)
{
	return a >= 0 ? a / n : -((-a + n - 1) / n);
}

// splitmix64 finaliser over (seed, a, b) — deterministic across runs
// and platforms (no std::hash / rand() involved).
static uint64_t villageHash(long seed, int a, int b)
{
	uint64_t h = (uint64_t)(seed + 0x9E3779B97F4A7C15ULL)
		^ ((uint64_t)(uint32_t)a * 0xBF58476D1CE4E5B9ULL)
		^ ((uint64_t)(uint32_t)b * 0x94D049BB133111EBULL);
	h ^= h >> 30;
	h *= 0xBF58476D1CE4E5B9ULL;
	h ^= h >> 27;
	h *= 0x94D049BB133111EBULL;
	h ^= h >> 31;
	return h;
}

void VillageFeature::getVillageCenterChunk(long seed, int gx, int gz, int& outChunkX, int& outChunkZ)
{
	uint64_t h = villageHash(seed, gx, gz);
	outChunkX = gx * GRID_CELLS + (int)(h % (uint64_t)GRID_CELLS);
	outChunkZ = gz * GRID_CELLS + (int)((h >> 32) % (uint64_t)GRID_CELLS);
}

bool VillageFeature::isVillageCenterChunk(long seed, int chunkX, int chunkZ, int& outCenterX, int& outCenterZ)
{
	int gx = floorDiv(chunkX, GRID_CELLS);
	int gz = floorDiv(chunkZ, GRID_CELLS);
	int vcx, vcz;
	getVillageCenterChunk(seed, gx, gz, vcx, vcz);
	if (vcx != chunkX || vcz != chunkZ) return false;
	outCenterX = vcx * 16 + 8;
	outCenterZ = vcz * 16 + 8;
	return true;
}

// ---------------------------------------------------------------------------
// Clipped block placement (never writes outside the chunk being generated)
// ---------------------------------------------------------------------------

static bool inChunk(int cx, int cz, int x, int z)
{
	return x >= cx * 16 && x < cx * 16 + 16 && z >= cz * 16 && z < cz * 16 + 16;
}

static void setV(Level* level, int cx, int cz, int x, int y, int z, int id)
{
	if (y < 1 || y >= 127) return;
	if (!inChunk(cx, cz, x, z)) return;
	level->setTile(x, y, z, id);
}

static void setVData(Level* level, int cx, int cz, int x, int y, int z, int id, int data)
{
	if (y < 1 || y >= 127) return;
	if (!inChunk(cx, cz, x, z)) return;
	level->setTileAndData(x, y, z, id, data);
}

// ---------------------------------------------------------------------------
// Village construction
// ---------------------------------------------------------------------------

// Flatten the village area to groundY (dirt fill / cut), per column.
// Only touches this chunk's columns; deterministic per column.
static void flattenGround(Level* level, int cx, int cz, int wx, int wz, int groundY)
{
	int R = VillageFeature::VILLAGE_RADIUS;
	int grassId = Tile::grass->id;
	int dirtId = Tile::dirt->id;
	for (int x = wx - R; x <= wx + R; ++x) {
		for (int z = wz - R; z <= wz + R; ++z) {
			int dx = x - wx, dz = z - wz;
			if (dx * dx + dz * dz > R * R) continue;
			if (!inChunk(cx, cz, x, z)) continue;
			int h = level->getHeightmap(x, z);
			if (h <= 63) continue; // water / beach: leave alone
			if (h < groundY) {
				for (int y = h; y < groundY; ++y) {
					if (level->getTile(x, y, z) == 0)
						setV(level, cx, cz, x, y, z, dirtId);
				}
				if (level->getTile(x, groundY, z) == 0)
					setV(level, cx, cz, x, groundY, z, grassId);
			} else if (h > groundY) {
				for (int y = groundY + 1; y <= h; ++y)
					setV(level, cx, cz, x, y, z, 0);
				int top = level->getTile(x, groundY, z);
				if (top != 0 && top != grassId)
					setV(level, cx, cz, x, groundY, z, grassId);
			}
		}
	}
}

static void placePaths(Level* level, int cx, int cz, int wx, int wz, int groundY)
{
	int R = VillageFeature::VILLAGE_RADIUS;
	int gravelId = Tile::gravel->id;
	int grassId = Tile::grass->id;
	int dirtId = Tile::dirt->id;
	// Cross paths (2 wide) through the centre.
	for (int x = wx - R; x <= wx + R; ++x) {
		for (int dz = 0; dz <= 1; ++dz) {
			int z = wz + dz;
			if (!inChunk(cx, cz, x, z)) continue;
			int top = level->getTile(x, groundY, z);
			if (top == grassId || top == dirtId)
				setV(level, cx, cz, x, groundY, z, gravelId);
		}
	}
	for (int z = wz - R; z <= wz + R; ++z) {
		for (int dx = 0; dx <= 1; ++dx) {
			int x = wx + dx;
			if (!inChunk(cx, cz, x, z)) continue;
			int top = level->getTile(x, groundY, z);
			if (top == grassId || top == dirtId)
				setV(level, cx, cz, x, groundY, z, gravelId);
		}
	}
	// Plaza ring around the well (radius 5..7).
	for (int dx = -7; dx <= 7; ++dx) {
		for (int dz = -7; dz <= 7; ++dz) {
			int r2 = dx * dx + dz * dz;
			if (r2 < 25 || r2 > 49) continue;
			int x = wx + dx, z = wz + dz;
			if (!inChunk(cx, cz, x, z)) continue;
			int top = level->getTile(x, groundY, z);
			if (top == grassId || top == dirtId)
				setV(level, cx, cz, x, groundY, z, gravelId);
		}
	}
}

static void placeWell(Level* level, int cx, int cz, int wx, int wz, int groundY)
{
	int waterId = Tile::calmWater->id;
	int stoneId = Tile::stoneBrick->id; // cobblestone in this tree's naming
	for (int dx = -1; dx <= 1; ++dx)
		for (int dz = -1; dz <= 1; ++dz)
			setV(level, cx, cz, wx + dx, groundY, wz + dz, waterId);
	for (int dx = -2; dx <= 2; ++dx) {
		for (int dz = -2; dz <= 2; ++dz) {
			if (dx < 2 && dx > -2 && dz < 2 && dz > -2) continue;
			setV(level, cx, cz, wx + dx, groundY + 1, wz + dz, stoneId);
		}
	}
	// Corner pillars.
	setV(level, cx, cz, wx - 2, groundY + 2, wz - 2, stoneId);
	setV(level, cx, cz, wx + 2, groundY + 2, wz - 2, stoneId);
	setV(level, cx, cz, wx - 2, groundY + 2, wz + 2, stoneId);
	setV(level, cx, cz, wx + 2, groundY + 2, wz + 2, stoneId);
}

// doorSide: 0 = door on the -z wall, 1 = +z wall, 2 = -x wall, 3 = +x wall
// (the side facing the village centre).
static void buildHouse(Level* level, int cx, int cz, int x0, int z0, int w, int d,
	int groundY, int doorSide, int wallId, bool workbench)
{
	int woodId = Tile::wood->id;
	int logId = Tile::treeTrunk->id;
	int glassId = Tile::glass->id;
	int lampId = Tile::lightGem->id;

	// Floor.
	for (int x = x0; x < x0 + w; ++x)
		for (int z = z0; z < z0 + d; ++z)
			setV(level, cx, cz, x, groundY, z, woodId);

	// Walls (3 high) + log corners.
	for (int y = groundY + 1; y <= groundY + 3; ++y) {
		for (int x = x0; x < x0 + w; ++x) {
			setV(level, cx, cz, x, y, z0, wallId);
			setV(level, cx, cz, x, y, z0 + d - 1, wallId);
		}
		for (int z = z0; z < z0 + d; ++z) {
			setV(level, cx, cz, x0, y, z, wallId);
			setV(level, cx, cz, x0 + w - 1, y, z, wallId);
		}
	}
	for (int y = groundY + 1; y <= groundY + 3; ++y) {
		setV(level, cx, cz, x0, y, z0, logId);
		setV(level, cx, cz, x0 + w - 1, y, z0, logId);
		setV(level, cx, cz, x0, y, z0 + d - 1, logId);
		setV(level, cx, cz, x0 + w - 1, y, z0 + d - 1, logId);
	}

	// Open doorway (1 wide x 2 high, no door block — door data is
	// fiddly and villagers have no door AI in this tree anyway).
	int doorX = x0 + w / 2, doorZ = z0 + d / 2;
	if (doorSide == 0) { doorX = x0 + w / 2; doorZ = z0; }
	else if (doorSide == 1) { doorX = x0 + w / 2; doorZ = z0 + d - 1; }
	else if (doorSide == 2) { doorX = x0; doorZ = z0 + d / 2; }
	else { doorX = x0 + w - 1; doorZ = z0 + d / 2; }
	setV(level, cx, cz, doorX, groundY + 1, doorZ, 0);
	setV(level, cx, cz, doorX, groundY + 2, doorZ, 0);

	// Windows (glass) on the two walls perpendicular to the door.
	if (doorSide == 0 || doorSide == 1) {
		setV(level, cx, cz, x0, groundY + 2, z0 + d / 2, glassId);
		setV(level, cx, cz, x0 + w - 1, groundY + 2, z0 + d / 2, glassId);
	} else {
		setV(level, cx, cz, x0 + w / 2, groundY + 2, z0, glassId);
		setV(level, cx, cz, x0 + w / 2, groundY + 2, z0 + d - 1, glassId);
	}

	// Flat plank roof.
	for (int x = x0; x < x0 + w; ++x)
		for (int z = z0; z < z0 + d; ++z)
			setV(level, cx, cz, x, groundY + 4, z, woodId);

	// Floor lamp (glowstone) + workbench in one house.
	setV(level, cx, cz, x0 + 1, groundY + 1, z0 + 1, lampId);
	if (workbench)
		setV(level, cx, cz, x0 + w - 2, groundY + 1, z0 + 1, Tile::workBench->id);
}

static void placeFarms(Level* level, int cx, int cz, int wx, int wz, int groundY)
{
	int farmId = Tile::farmland->id;
	int waterId = Tile::calmWater->id;
	// Plot A (wheat) at (+16, +12), plot B (carrots) at (-16, -12).
	// 9 wide x 5 deep, middle row is the water channel.
	const int plots[2][3] = {
		{ 16, 12, Tile::crops->id },
		{ -16, -12, Tile::carrots->id },
	};
	for (int p = 0; p < 2; ++p) {
		int pcx = wx + plots[p][0];
		int pcz = wz + plots[p][1];
		int cropId = plots[p][2];
		for (int dx = -4; dx <= 4; ++dx) {
			for (int dz = -2; dz <= 2; ++dz) {
				int x = pcx + dx, z = pcz + dz;
				if (dz == 0) {
					setV(level, cx, cz, x, groundY, z, waterId);
				} else {
					setVData(level, cx, cz, x, groundY, z, farmId, 7);
					setVData(level, cx, cz, x, groundY + 1, z, cropId, 7);
				}
			}
		}
	}
}

static void placeLamps(Level* level, int cx, int cz, int wx, int wz, int groundY)
{
	int fenceId = Tile::fence->id;
	int lampId = Tile::lightGem->id;
	const int offs[4][2] = { { -7, -7 }, { 7, -7 }, { -7, 7 }, { 7, 7 } };
	for (int i = 0; i < 4; ++i) {
		int x = wx + offs[i][0], z = wz + offs[i][1];
		setV(level, cx, cz, x, groundY + 1, z, fenceId);
		setV(level, cx, cz, x, groundY + 2, z, lampId);
	}
}

static void placeVillage(Level* level, int cx, int cz, int wx, int wz, int groundY, Random* vr)
{
	flattenGround(level, cx, cz, wx, wz, groundY);
	placePaths(level, cx, cz, wx, wz, groundY);
	placeWell(level, cx, cz, wx, wz, groundY);
	placeFarms(level, cx, cz, wx, wz, groundY);

	// 5 houses around the centre; door faces the centre. Jitter +-2
	// keeps the fixed layout from ever overlapping itself.
	struct House { int dx, dz, w, d, door; bool bench; };
	const House houses[5] = {
		{ -3, -18, 7, 5, 1, false },  // N (door on +z wall)
		{ -3, 13, 7, 5, 0, true },    // S (door on -z wall) + workbench
		{ 12, -2, 7, 5, 2, false },   // E (door on -x wall)
		{ -18, -2, 7, 5, 3, false },  // W (door on +x wall)
		{ 10, -15, 5, 4, 2, false },  // NE (door on -x wall)
	};
	for (int i = 0; i < 5; ++i) {
		int jx = vr->nextInt(5) - 2;
		int jz = vr->nextInt(5) - 2;
		int wallId = (vr->nextInt(2) == 0) ? Tile::stoneBrick->id : Tile::wood->id;
		buildHouse(level, cx, cz,
			wx + houses[i].dx + jx, wz + houses[i].dz + jz,
			houses[i].w, houses[i].d, groundY, houses[i].door, wallId,
			houses[i].bench);
	}

	placeLamps(level, cx, cz, wx, wz, groundY);
}

void VillageFeature::tryPlaceVillage(Level* level, int chunkX, int chunkZ)
{
	if (level == NULL) return;
	long seed = level->getSeed();
	int R = VILLAGE_RADIUS;
	int gx0 = floorDiv(chunkX, GRID_CELLS);
	int gz0 = floorDiv(chunkZ, GRID_CELLS);
	for (int gx = gx0 - 1; gx <= gx0 + 1; ++gx) {
		for (int gz = gz0 - 1; gz <= gz0 + 1; ++gz) {
			int vcx, vcz;
			getVillageCenterChunk(seed, gx, gz, vcx, vcz);
			int wx = vcx * 16 + 8;
			int wz = vcz * 16 + 8;
			// Does this village's footprint touch this chunk at all?
			if (wx + R < chunkX * 16 || wx - R > chunkX * 16 + 15) continue;
			if (wz + R < chunkZ * 16 || wz - R > chunkZ * 16 + 15) continue;
			// Plains / desert only, on dry land.
			Biome* biome = level->getBiome(wx, wz);
			if (biome != Biome::plains && biome != Biome::desert) continue;
			int groundY = level->getHeightmap(wx, wz);
			if (groundY < 65) continue;
			Random vr((long)villageHash(seed, gx, gz));
			placeVillage(level, chunkX, chunkZ, wx, wz, groundY, &vr);
		}
	}
}

// ---------------------------------------------------------------------------
// Villagers
// ---------------------------------------------------------------------------

void VillageFeature::ensureVillagers(Level* level, int chunkX, int chunkZ)
{
	if (level == NULL) return;
	// Single-player overworld only: the client cache must not spawn
	// entities (server-authoritative), and the Nether/End have their
	// own levels with no villages.
	if (level->isClientSide) return;
	if (level->dimension == NULL || level->dimension->id != Dimension::NORMAL) return;

	long seed = level->getSeed();
	int wx, wz;
	if (!isVillageCenterChunk(seed, chunkX, chunkZ, wx, wz)) return;
	Biome* biome = level->getBiome(wx, wz);
	if (biome != Biome::plains && biome != Biome::desert) return;
	// Only spawn once this chunk's terrain exists: on first generation
	// postProcess may be deferred (neighbours missing), and the village
	// blocks/flattening do not exist yet — spawning now would leave
	// villagers buried under the flattening. The ChunkCache postProcess
	// hook re-runs this once the terrain is populated.
	LevelChunk* chunk = level->getChunk(chunkX, chunkZ);
	if (chunk == NULL || !chunk->terrainPopulated) return;
	int groundY = level->getHeightmap(wx, wz);
	if (groundY < 65) return;

	// Count villagers already near this village and top up to 4.
	AABB box((float)(wx - 32), 0.0f, (float)(wz - 32),
		(float)(wx + 32), 128.0f, (float)(wz + 32));
	EntityList list;
	int have = level->getEntitiesOfType(MobTypes::Villager, box, list);
	if (have >= VILLAGERS_PER_VILLAGE) return;

	for (int i = have; i < VILLAGERS_PER_VILLAGE; ++i) {
		// Plaza ring around the well (radius ~6).
		float angle = (float)i * 1.5708f + 0.5f;
		int px = wx + (int)(Mth::cos(angle) * 6.0f);
		int pz = wz + (int)(Mth::sin(angle) * 6.0f);
		int y = level->getTopSolidBlock(px, pz);
		if (y < 1) continue;
		Villager* v = new Villager(level);
		v->moveTo((float)px + 0.5f, (float)y, (float)pz + 0.5f,
			level->random.nextFloat() * 360.0f, 0.0f);
		level->addEntity(v);
	}
}
