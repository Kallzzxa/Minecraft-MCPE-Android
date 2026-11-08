#include "BrewingStandTile.h"
#include "entity/BrewingStandTileEntity.h"
#include "../material/Material.h"
#include "../../entity/player/Player.h"
#include "../../entity/item/ItemEntity.h"
#include "../../item/ItemInstance.h"
#include "../Level.h"

// terrain.png cell (4,11) — drawn stand texture (see tools note in
// the v20 ROADMAP entry; the cell was a blank placeholder).
#define BREWING_STAND_TEX (4 + 11 * 16)

BrewingStandTile::BrewingStandTile(int id)
:	super(id, BREWING_STAND_TEX, Material::stone)
{
}

int BrewingStandTile::getResource(int data, Random* random) {
	return Tile::brewingStand->id;
}

bool BrewingStandTile::use(Level* level, int x, int y, int z, Player* player) {
	if (level->isClientSide)
		return true;

	BrewingStandTileEntity* stand = static_cast<BrewingStandTileEntity*>(level->getTileEntity(x, y, z));
	if (stand != NULL) player->openBrewingStand(stand);
	return true;
}

TileEntity* BrewingStandTile::newTileEntity() {
	return TileEntityFactory::createTileEntity(TileEntityType::BrewingStand);
}

void BrewingStandTile::onRemove(Level* level, int x, int y, int z) {
	if (!level->isClientSide) {
		BrewingStandTileEntity* container = (BrewingStandTileEntity*) level->getTileEntity(x, y, z);
		if (container != NULL) {
			// Drop the stand's contents, like the furnace does.
			for (int i = 0; i < container->getContainerSize(); i++) {
				ItemInstance* item = container->getItem(i);
				if (item) {
					float xo = random.nextFloat() * 0.8f + 0.1f;
					float yo = random.nextFloat() * 0.8f + 0.1f;
					float zo = random.nextFloat() * 0.8f + 0.1f;

					while (item->count > 0) {
						int count = random.nextInt(21) + 10;
						if (count > item->count) count = item->count;
						item->count -= count;

						ItemEntity* itemEntity = new ItemEntity(level, x + xo, y + yo, z + zo, ItemInstance(item->id, count, item->getAuxValue()));
						float pow = 0.05f;
						itemEntity->xd = random.nextGaussian() * pow;
						itemEntity->yd = random.nextGaussian() * pow + 0.2f;
						itemEntity->zd = random.nextGaussian() * pow;
						level->addEntity(itemEntity);
					}
				}
			}
		}
	}
	super::onRemove(level, x, y, z);
}
