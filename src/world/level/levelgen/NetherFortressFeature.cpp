#include "NetherFortressFeature.h"
#include "ChestLoot.h"

#include "NetherLevelSource.h"
#include "../Level.h"
#include "../chunk/LevelChunk.h"
#include "../dimension/Dimension.h"
#include "../tile/Tile.h"
#include "../../entity/monster/Blaze.h"
#include "../../entity/monster/WitherSkeleton.h"
#include "../../entity/EntityTypes.h"
#include "../../phys/AABB.h"
#include "../../../util/Mth.h"
#include <stdint.h>

// ---------------------------------------------------------------------------
// Deterministic fortress placement
// ---------------------------------------------------------------------------

static int floorDiv(int a, int n)
{
	return a >= 0 ? a / n : -((-a + n - 1) / n);
}

// splitmix64 finaliser over (seed, a, b) — deterministic across runs
// and platforms. Different salt from VillageFeature so the fortress
// grid never correlates with the village grid.
static uint64_t fortressHash(long seed, int a, int b)
{
	uint64_t h = (uint64_t)(seed + 0x6A09E667F3BCC908ULL)
		^ ((uint64_t)(uint32_t)a * 0xBF58476D1CE4E5B9ULL)
		^ ((uint64_t)(uint32_t)b * 0x94D049BB133111EBULL);
	h ^= h >> 30;
	h *= 0xBF58476D1CE4E5B9ULL;
	h ^= h >> 27;
	h *= 0x94D049BB133111EBULL;
	h ^= h >> 31;
	return h;
}

void NetherFortressFeature::getFortressCenterChunk(long seed, int gx, int gz, int& outChunkX, int& outChunkZ)
{
	uint64_t h = fortressHash(seed, gx, gz);
	outChunkX = gx * GRID_CELLS + (int)(h % (uint64_t)GRID_CELLS);
	outChunkZ = gz * GRID_CELLS + (int)((h >> 32) % (uint64_t)GRID_CELLS);
}

bool NetherFortressFeature::isFortressCenterChunk(long seed, int chunkX, int chunkZ, int& outCenterX, int& outCenterZ)
{
	int gx = floorDiv(chunkX, GRID_CELLS);
	int gz = floorDiv(chunkZ, GRID_CELLS);
	int fcx, fcz;
	getFortressCenterChunk(seed, gx, gz, fcx, fcz);
	if (fcx != chunkX || fcz != chunkZ) return false;
	outCenterX = fcx * 16 + 8;
	outCenterZ = fcz * 16 + 8;
	return true;
}

// Footprint half-extents (blocks from the centre).
static const int FORT_RX = 24;
static const int FORT_RZ = 16;

int NetherFortressFeature::getDeckY(long seed, int wx, int wz)
{
	// Max Nether floor height over the footprint: the deck sits one
	// above the highest netherrack in the area, so it is never buried
	// where the terrain rises. Pure noise — no chunk reads, so every
	// chunk overlapping the fortress agrees on the same deck Y.
	int deckY = 0;
	for (int x = wx - FORT_RX; x <= wx + FORT_RX; ++x) {
		for (int z = wz - FORT_RZ; z <= wz + FORT_RZ; ++z) {
			int h = NetherLevelSource::getFloorHeight(seed, x, z);
			if (h > deckY) deckY = h;
		}
	}
	return deckY;
}

// ---------------------------------------------------------------------------
// Clipped block placement (never writes outside the chunk being generated)
// ---------------------------------------------------------------------------

static bool inChunk(int cx, int cz, int x, int z)
{
	return x >= cx * 16 && x < cx * 16 + 16 && z >= cz * 16 && z < cz * 16 + 16;
}

static void setF(Level* level, int cx, int cz, int x, int y, int z, int id)
{
	if (y < 1 || y >= 127) return;
	if (!inChunk(cx, cz, x, z)) return;
	level->setTile(x, y, z, id);
}

static void setFData(Level* level, int cx, int cz, int x, int y, int z, int id, int data)
{
	if (y < 1 || y >= 127) return;
	if (!inChunk(cx, cz, x, z)) return;
	level->setTileAndData(x, y, z, id, data);
}

// ---------------------------------------------------------------------------
// Fortress construction
// ---------------------------------------------------------------------------

// Nether-brick support pillar from just under the deck down to solid
// ground. Runs through air AND lava (the floor often dips below the
// lava sea) and stops at the first solid block — so a bridge over a
// lava lake stands on real pillars instead of floating.
static void placePillar(Level* level, int cx, int cz, int x, int z, int deckY)
{
	int brickId = Tile::netherBrick->id;
	int lavaId = Tile::calmLava->id;
	for (int y = deckY - 1; y > 1; --y) {
		int t = level->getTile(x, y, z);
		if (t == 0 || t == lavaId) {
			setF(level, cx, cz, x, y, z, brickId);
		} else {
			break;
		}
	}
}

static void placeFortress(Level* level, int cx, int cz, int wx, int wz, int deckY, long fortHash)
{
	int D = deckY;
	int brickId = Tile::netherBrick->id;
	int soulId = Tile::hellSand->id;
	int wartId = Tile::netherWart->id;
	int glowId = Tile::lightGem->id;

	// Bridge E-W: deck + 1-high curbs on the outer rows.
	for (int x = wx - 16; x <= wx + 16; ++x) {
		for (int z = wz - 2; z <= wz + 2; ++z)
			setF(level, cx, cz, x, D, z, brickId);
		setF(level, cx, cz, x, D + 1, wz - 2, brickId);
		setF(level, cx, cz, x, D + 1, wz + 2, brickId);
	}

	// Corridor N-S: floor, side walls (D+1..D+3), flat roof (D+4).
	for (int z = wz - 14; z <= wz + 14; ++z) {
		for (int x = wx - 1; x <= wx + 1; ++x)
			setF(level, cx, cz, x, D, z, brickId);
		for (int y = D + 1; y <= D + 3; ++y) {
			setF(level, cx, cz, wx - 2, y, z, brickId);
			setF(level, cx, cz, wx + 2, y, z, brickId);
		}
		for (int x = wx - 2; x <= wx + 2; ++x)
			setF(level, cx, cz, x, D + 4, z, brickId);
	}

	// Blaze room (east): 7x7, x wx+17..wx+23, z wz-3..wz+3.
	for (int x = wx + 17; x <= wx + 23; ++x) {
		for (int z = wz - 3; z <= wz + 3; ++z)
			setF(level, cx, cz, x, D, z, brickId);
	}
	for (int y = D + 1; y <= D + 3; ++y) {
		for (int x = wx + 17; x <= wx + 23; ++x) {
			setF(level, cx, cz, x, y, wz - 3, brickId);
			setF(level, cx, cz, x, y, wz + 3, brickId);
		}
		for (int z = wz - 3; z <= wz + 3; ++z) {
			setF(level, cx, cz, wx + 17, y, z, brickId);
			setF(level, cx, cz, wx + 23, y, z, brickId);
		}
	}
	for (int x = wx + 17; x <= wx + 23; ++x)
		for (int z = wz - 3; z <= wz + 3; ++z)
			setF(level, cx, cz, x, D + 4, z, brickId);
	// Doorway on the west wall (faces the bridge).
	setF(level, cx, cz, wx + 17, D + 1, wz, 0);
	setF(level, cx, cz, wx + 17, D + 2, wz, 0);
	// Glowstone in the 4 inner ceiling corners.
	setF(level, cx, cz, wx + 18, D + 3, wz - 2, glowId);
	setF(level, cx, cz, wx + 18, D + 3, wz + 2, glowId);
	setF(level, cx, cz, wx + 22, D + 3, wz - 2, glowId);
	setF(level, cx, cz, wx + 22, D + 3, wz + 2, glowId);
	// v33: one loot chest in the blaze room (east wall, middle —
	// clear of the glowstone corners, the west doorway and the
	// blaze spawn spots). The block place creates the
	// ChestTileEntity via EntityTile::onPlace; ChestLoot fills it.
	// Guarded by inChunk so only the owning chunk touches it.
	if (inChunk(cx, cz, wx + 22, wz)) {
		setF(level, cx, cz, wx + 22, D + 1, wz, Tile::chest->id);
		Random chestR((long)(fortHash ^ 0xF0E57A11ULL));
		ChestLoot::fillFortressChest(level, wx + 22, D + 1, wz, &chestR);
	}
	// v39: one blaze spawner in the middle of the blaze room floor
	// (wx+20, wz — the old first blaze spawn spot, clear of the
	// glowstone corners, the west doorway and the loot chest).
	// Placing the block creates the MobSpawnerTileEntity via
	// EntityTile::onPlace (LevelChunk::setTile calls onPlace), the
	// same mechanism as the chest above; the entity then spawns
	// blazes around itself while a player is near. Guarded by
	// inChunk so only the owning chunk touches it.
	if (inChunk(cx, cz, wx + 20, wz)) {
		setF(level, cx, cz, wx + 20, D + 1, wz, Tile::mobSpawner->id);
	}

	// Wart room (west): 7x5, x wx-23..wx-17, z wz-2..wz+2.
	// Soul sand floor, mature wart in two rows.
	for (int x = wx - 23; x <= wx - 17; ++x) {
		for (int z = wz - 2; z <= wz + 2; ++z)
			setF(level, cx, cz, x, D, z, soulId);
	}
	for (int y = D + 1; y <= D + 3; ++y) {
		for (int x = wx - 23; x <= wx - 17; ++x) {
			setF(level, cx, cz, x, y, wz - 2, brickId);
			setF(level, cx, cz, x, y, wz + 2, brickId);
		}
		for (int z = wz - 2; z <= wz + 2; ++z) {
			setF(level, cx, cz, wx - 23, y, z, brickId);
			setF(level, cx, cz, wx - 17, y, z, brickId);
		}
	}
	for (int x = wx - 23; x <= wx - 17; ++x)
		for (int z = wz - 2; z <= wz + 2; ++z)
			setF(level, cx, cz, x, D + 4, z, brickId);
	// Doorway on the east wall (faces the bridge).
	setF(level, cx, cz, wx - 17, D + 1, wz, 0);
	setF(level, cx, cz, wx - 17, D + 2, wz, 0);
	// 2 glowstone inside, above the wart rows.
	setF(level, cx, cz, wx - 22, D + 3, wz - 1, glowId);
	setF(level, cx, cz, wx - 22, D + 3, wz + 1, glowId);
	// Mature wart (data 3) on the soul sand, two rows of five.
	for (int x = wx - 22; x <= wx - 18; ++x) {
		setFData(level, cx, cz, x, D + 1, wz - 1, wartId, 3);
		setFData(level, cx, cz, x, D + 1, wz + 1, wartId, 3);
	}

	// Support pillars: bridge every 4 blocks, room corners +
	// midpoints, corridor ends. Each runs down to solid ground.
	for (int x = wx - 16; x <= wx + 16; x += 4)
		placePillar(level, cx, cz, x, wz, D);
	const int pillarSpots[][2] = {
		{ wx + 17, wz - 3 }, { wx + 17, wz + 3 },
		{ wx + 23, wz - 3 }, { wx + 23, wz + 3 },
		{ wx + 20, wz - 3 }, { wx + 20, wz + 3 },
		{ wx - 23, wz - 2 }, { wx - 23, wz + 2 },
		{ wx - 17, wz - 2 }, { wx - 17, wz + 2 },
		{ wx - 20, wz - 2 }, { wx - 20, wz + 2 },
		{ wx, wz - 14 }, { wx, wz + 14 },
	};
	for (int i = 0; i < 14; ++i)
		placePillar(level, cx, cz, pillarSpots[i][0], pillarSpots[i][1], D);
}

void NetherFortressFeature::tryPlaceFortress(Level* level, int chunkX, int chunkZ)
{
	if (level == NULL) return;
	long seed = level->getSeed();
	int gx0 = floorDiv(chunkX, GRID_CELLS);
	int gz0 = floorDiv(chunkZ, GRID_CELLS);
	for (int gx = gx0 - 1; gx <= gx0 + 1; ++gx) {
		for (int gz = gz0 - 1; gz <= gz0 + 1; ++gz) {
			int fcx, fcz;
			getFortressCenterChunk(seed, gx, gz, fcx, fcz);
			int wx = fcx * 16 + 8;
			int wz = fcz * 16 + 8;
			// Does this fortress's footprint touch this chunk at all?
			if (wx + FORT_RX < chunkX * 16 || wx - FORT_RX > chunkX * 16 + 15) continue;
			if (wz + FORT_RZ < chunkZ * 16 || wz - FORT_RZ > chunkZ * 16 + 15) continue;
			int deckY = getDeckY(seed, wx, wz);
			if (deckY < 36) continue; // lava-sea terrain: skip it
			placeFortress(level, chunkX, chunkZ, wx, wz, deckY,
				(long)fortressHash(seed, gx, gz));
		}
	}
}

// ---------------------------------------------------------------------------
// Blazes
// ---------------------------------------------------------------------------

void NetherFortressFeature::ensureBlazes(Level* level, int chunkX, int chunkZ)
{
	if (level == NULL) return;
	// Single-player Nether only: the client cache must not spawn
	// entities (server-authoritative), and the overworld/End levels
	// have no fortresses.
	if (level->isClientSide) return;
	if (level->dimension == NULL || level->dimension->id != Dimension::NETHER) return;

	long seed = level->getSeed();
	int wx, wz;
	if (!isFortressCenterChunk(seed, chunkX, chunkZ, wx, wz)) return;
	int deckY = getDeckY(seed, wx, wz);
	if (deckY < 36) return;
	// Only spawn once this chunk's terrain exists: on first generation
	// postProcess may be deferred (neighbours missing), and the
	// fortress blocks do not exist yet — spawning now would leave
	// blazes floating over lava where the deck will be. The ChunkCache
	// postProcess hook re-runs this once the terrain is populated.
	LevelChunk* chunk = level->getChunk(chunkX, chunkZ);
	if (chunk == NULL || !chunk->terrainPopulated) return;

	// v39: if the blaze room already has a spawner block, the
	// spawner owns the blaze supply — no top-up. The top-up stays
	// ONLY as the old-world fallback: fortresses generated before
	// v39 have no spawner block, and without the top-up their blaze
	// rooms would go empty forever once the blazes die. (Same
	// fallback covers a spawner the player broke: on the next
	// centre-chunk reload the room tops up to 3 again.) This check
	// runs AFTER the terrainPopulated guard above: reading the
	// spawner block before the fortress exists would top up blazes
	// over lava where the deck will be.
	// Spawner pos: blaze room centre (wx+20, wz), deck + 1 — the
	// same spot placeFortress puts Tile::mobSpawner.
	if (level->getTile(wx + 20, deckY + 1, wz) == Tile::mobSpawner->id) return;

	// Count blazes already near this fortress and top up to 3.
	AABB box((float)(wx - 26), (float)deckY, (float)(wz - 18),
		(float)(wx + 26), (float)(deckY + 9), (float)(wz + 18));
	EntityList list;
	int have = level->getEntitiesOfType(MobTypes::Blaze, box, list);
	if (have >= BLAZES_PER_FORTRESS) return;

	// Spawn spots inside the blaze room (deck + 1 standing level).
	const int spots[3][2] = {
		{ wx + 20, wz }, { wx + 18, wz - 2 }, { wx + 22, wz + 2 },
	};
	for (int i = have; i < BLAZES_PER_FORTRESS; ++i) {
		Blaze* b = new Blaze(level);
		b->moveTo((float)spots[i][0] + 0.5f, (float)(deckY + 1),
			(float)spots[i][1] + 0.5f,
			level->random.nextFloat() * 360.0f, 0.0f);
		level->addEntity(b);
	}
}

// ---------------------------------------------------------------------------
// Wither skeletons (v32)
// ---------------------------------------------------------------------------

void NetherFortressFeature::ensureWitherSkeletons(Level* level, int chunkX, int chunkZ)
{
	// Same guards as ensureBlazes (single-player Nether only,
	// centre chunk only, deck above the lava sea, terrain
	// populated) — the two top-ups share one fortress, so the
	// checks are identical by design.
	if (level == NULL) return;
	if (level->isClientSide) return;
	if (level->dimension == NULL || level->dimension->id != Dimension::NETHER) return;

	long seed = level->getSeed();
	int wx, wz;
	if (!isFortressCenterChunk(seed, chunkX, chunkZ, wx, wz)) return;
	int deckY = getDeckY(seed, wx, wz);
	if (deckY < 36) return;
	LevelChunk* chunk = level->getChunk(chunkX, chunkZ);
	if (chunk == NULL || !chunk->terrainPopulated) return;

	// Count wither skeletons already near this fortress and top up
	// to 2. Same box as the blazes (the whole fortress footprint).
	AABB box((float)(wx - 26), (float)deckY, (float)(wz - 18),
		(float)(wx + 26), (float)(deckY + 9), (float)(wz + 18));
	EntityList list;
	int have = level->getEntitiesOfType(MobTypes::WitherSkeleton, box, list);
	if (have >= WITHER_SKELETONS_PER_FORTRESS) return;

	// Spawn spots on the walkable fortress, away from the blaze
	// room: one in the corridor south of the centre, one on the
	// bridge east of the centre (both deck + 1 standing level).
	const int spots[2][2] = {
		{ wx, wz - 7 }, { wx + 8, wz },
	};
	for (int i = have; i < WITHER_SKELETONS_PER_FORTRESS; ++i) {
		WitherSkeleton* w = new WitherSkeleton(level);
		w->moveTo((float)spots[i][0] + 0.5f, (float)(deckY + 1),
			(float)spots[i][1] + 0.5f,
			level->random.nextFloat() * 360.0f, 0.0f);
		level->addEntity(w);
	}
}
