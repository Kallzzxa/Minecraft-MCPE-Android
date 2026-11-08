#include "BrewingStandTileEntity.h"

#include "../BrewingStandTile.h"
#include "../../Level.h"
#include "../../material/Material.h"
#include "../../../Container.h"
#include "../../../entity/player/Player.h"
#include "../../../item/ItemInstance.h"
#include "../../../item/Item.h"
#include "../../../item/PotionItem.h"
#include "../../../item/crafting/BrewingRecipes.h"
#include "../../../../nbt/ListTag.h"

BrewingStandTileEntity::BrewingStandTileEntity()
:	super(TileEntityType::BrewingStand),
	Container(ContainerType::BREWING_STAND),
	brewTime(0),
	fuelCharges(0)
{
}

BrewingStandTileEntity::~BrewingStandTileEntity() {
}

ItemInstance* BrewingStandTileEntity::getItem(int slot) {
	return &items[slot];
}

ItemInstance BrewingStandTileEntity::removeItem(int slot, int count) {
	if (!items[slot].isNull()) {
		if (items[slot].count <= count) {
			ItemInstance item = items[slot];
			items[slot].setNull();
			return item;
		} else {
			ItemInstance i = items[slot].remove(count);
			if (items[slot].count == 0) items[slot].setNull();
			return i;
		}
	}
	return ItemInstance();
}

void BrewingStandTileEntity::setItem(int slot, ItemInstance* item) {
	items[slot] = *item;
	if (item != NULL && item->count > getMaxStackSize())
		items[slot].count = getMaxStackSize();
}

std::string BrewingStandTileEntity::getName() const {
	return "Brewing Stand";
}

bool BrewingStandTileEntity::shouldSave() {
	if (brewTime > 0 || fuelCharges > 0) return true;
	for (int i = 0; i < NumItems; ++i)
		if (!items[i].isNull()) return true;
	return false;
}

void BrewingStandTileEntity::load(CompoundTag* base) {
	super::load(base);
	ListTag* inventoryList = base->getList("Items");
	for (int i = 0; i < NumItems; ++i)
		items[i].setNull();
	for (int i = 0; i < inventoryList->size(); i++) {
		Tag* tt = inventoryList->get(i);
		if (tt->getId() == Tag::TAG_Compound) {
			CompoundTag* tag = (CompoundTag*)tt;
			int slot = tag->getByte("Slot");
			if (slot >= 0 && slot < NumItems) {
				ItemInstance* loaded = ItemInstance::fromTag(tag);
				if (loaded) {
					items[slot] = *loaded;
					delete loaded;
				} else
					items[slot].setNull();
			}
		} else {
			LOGE("load @ BrewingStandTileEntity failed. item's not a compoundTag!\n");
		}
	}

	brewTime = base->getShort("BrewTime");
	fuelCharges = base->getShort("Fuel");
}

bool BrewingStandTileEntity::save(CompoundTag* base) {
	if (!super::save(base))
		return false;

	base->putShort("BrewTime", (short) (brewTime));
	base->putShort("Fuel", (short) (fuelCharges));
	ListTag* listTag = new ListTag();

	for (int i = 0; i < NumItems; i++) {
		if (!items[i].isNull()) {
			CompoundTag* tag = new CompoundTag();
			tag->putByte("Slot", (char) i);
			items[i].save(tag);
			listTag->add(tag);
		}
	}
	base->put("Items", listTag);
	return true;
}

int BrewingStandTileEntity::getMaxStackSize() const {
	return Container::LARGE_MAX_STACK_SIZE;
}

int BrewingStandTileEntity::getContainerSize() const {
	return NumItems;
}

int BrewingStandTileEntity::getBrewProgress(int max) {
	return brewTime * max / BREW_TIME;
}

int BrewingStandTileEntity::getFuelProgress(int max) {
	return fuelCharges * max / FUEL_PER_POWDER;
}

bool BrewingStandTileEntity::stillValid(Player* player) {
	if (level->getTileEntity(x, y, z) != this) return false;
	if (player->distanceToSqr(x + 0.5f, y + 0.5f, z + 0.5f) > 8 * 8) return false;
	return true;
}

void BrewingStandTileEntity::startOpen() {
}

void BrewingStandTileEntity::stopOpen() {
}

bool BrewingStandTileEntity::isSlotEmpty(int slot) {
	return items[slot].isNull();
}

void BrewingStandTileEntity::tick() {
	if (level->isClientSide) return;

	bool changed = false;

	// Refuel: one blaze powder = 20 brew operations (modern MC).
	if (fuelCharges <= 0 && !items[SLOT_FUEL].isNull()
		&& BrewingRecipes::isFuel(items[SLOT_FUEL].id)) {
		if (--items[SLOT_FUEL].count == 0) items[SLOT_FUEL].setNull();
		fuelCharges = FUEL_PER_POWDER;
		changed = true;
	}

	if (canBrew()) {
		if (++brewTime >= BREW_TIME) {
			brewTime = 0;
			brew();
			changed = true;
		}
	} else {
		brewTime = 0;
	}

	if (changed) setChanged();
}

bool BrewingStandTileEntity::canBrew() {
	if (items[SLOT_INGREDIENT].isNull()) return false;
	if (fuelCharges <= 0) return false;
	int ingredientId = items[SLOT_INGREDIENT].id;
	for (int slot = SLOT_BOTTLE_0; slot <= SLOT_BOTTLE_2; ++slot) {
		if (items[slot].isNull()) continue;
		if (items[slot].id != Item::potion->id) continue;
		if (BrewingRecipes::getResult(items[slot].getAuxValue(), ingredientId) != -1)
			return true;
	}
	return false;
}

void BrewingStandTileEntity::brew() {
	if (!canBrew()) return;
	int ingredientId = items[SLOT_INGREDIENT].id;
	for (int slot = SLOT_BOTTLE_0; slot <= SLOT_BOTTLE_2; ++slot) {
		if (items[slot].isNull()) continue;
		if (items[slot].id != Item::potion->id) continue;
		int result = BrewingRecipes::getResult(items[slot].getAuxValue(), ingredientId);
		if (result == -1) continue;
		items[slot].setAuxValue(result);
	}
	if (--items[SLOT_INGREDIENT].count <= 0) items[SLOT_INGREDIENT].setNull();
	if (fuelCharges > 0) fuelCharges--;
}
