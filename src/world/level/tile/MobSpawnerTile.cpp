#include "MobSpawnerTile.h"
#include "entity/MobSpawnerTileEntity.h"
#include "../material/Material.h"
#include "../../../util/Random.h"

// terrain.png cell (11,15) — drawn cage texture (dark steel bars
// over near-black, orange blaze glow in the middle); the cell was a
// black/white checker referenced by no tile, mapper or dynamic
// texture (see the v39 ROADMAP entry).
#define MOB_SPAWNER_TEX (11 + 15 * 16)

MobSpawnerTile::MobSpawnerTile(int id)
:	super(id, MOB_SPAWNER_TEX, Material::stone)
{
}

int MobSpawnerTile::getResource(int data, Random* random)
{
	(void) data;
	(void) random;
	// Modern MC: breaking a spawner drops nothing (silk touch
	// only, which does not exist in this tree). Returning 0 makes
	// Tile::spawnResources skip the drop.
	return 0;
}

TileEntity* MobSpawnerTile::newTileEntity()
{
	return TileEntityFactory::createTileEntity(TileEntityType::MobSpawner);
}
