#include "CauldronTile.h"

#include "../Level.h"
#include "../../entity/player/Player.h"
#include "../../entity/player/Inventory.h"
#include "../../item/Item.h"
#include "../../item/ItemInstance.h"
#include "../../item/PotionItem.h"

// terrain.png cell 250 (10,15) — verified unused (see the header).
#define CAULDRON_TEX 250

CauldronTile::CauldronTile(int id)
:	super(id, CAULDRON_TEX, Material::stone)
{
}

int CauldronTile::getTexture(int face, int data)
{
	// Top face (face 1, EnchantTableTile precedent) shows the water
	// surface while the cauldron holds any water; a dry cauldron
	// shows iron on every face.
	if (face == 1 && data > 0)
		return Tile::water->tex;
	return tex;
}

bool CauldronTile::use(Level* level, int x, int y, int z, Player* player)
{
	if (level->isClientSide)
		return true;

	int waterLevel = level->getData(x, y, z);
	if (waterLevel < 0) waterLevel = 0;
	if (waterLevel > MAX_LEVEL) waterLevel = MAX_LEVEL;

	ItemInstance* held = player->inventory->getSelected();
	if (held == NULL || held->isNull()) return false;
	bool creative = player->abilities.instabuild;

	// Take water out with an empty glass bottle (modern MC).
	if (held->id == Item::glassBottle->id && waterLevel > 0) {
		if (!creative) held->count--;
		level->setData(x, y, z, waterLevel - 1);
		ItemInstance water(Item::potion, 1, PotionItem::WATER);
		if (!player->inventory->add(&water))
			player->drop(new ItemInstance(water), false);
		level->playSound(player, "random.splash", 1.0f, 1.0f);
		return true;
	}

	// Pour a water bottle in (drinkable or splash — only the base
	// type matters). One bottle = one level, up to full.
	if (held->id == Item::potion->id
		&& PotionItem::baseAux(held->getAuxValue()) == PotionItem::WATER
		&& waterLevel < MAX_LEVEL) {
		if (!creative) held->count--;
		level->setData(x, y, z, waterLevel + 1);
		ItemInstance empty(Item::glassBottle);
		if (!player->inventory->add(&empty))
			player->drop(new ItemInstance(empty), false);
		level->playSound(player, "random.splash", 1.0f, 1.0f);
		return true;
	}

	return false;
}
