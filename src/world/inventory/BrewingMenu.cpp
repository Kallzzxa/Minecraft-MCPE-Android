#include "BrewingMenu.h"
#include "../entity/player/Player.h"
#include "../item/ItemInstance.h"
#include "../level/tile/entity/BrewingStandTileEntity.h"

BrewingMenu::BrewingMenu(BrewingStandTileEntity* stand)
:	super(ContainerType::BREWING_STAND),
	stand(stand),
	lastBrewTime(0),
	lastFuelCharges(0)
{
	brewingStandTileEntityId = stand->runningId;
}

void BrewingMenu::setSlot(int slot, ItemInstance* item) {
	stand->setItem(slot, item);
}

void BrewingMenu::setData(int id, int value) {
	if (id == 0) stand->brewTime = value;
	if (id == 1) stand->fuelCharges = value;
}

std::vector<ItemInstance> BrewingMenu::getItems() {
	std::vector<ItemInstance> out;
	for (int i = 0; i < stand->getContainerSize(); ++i)
		out.push_back(*stand->getItem(i));
	return out;
}

void BrewingMenu::broadcastChanges() {
	super::broadcastChanges();

	if (!listener)
		return;

	if (stand->brewTime != lastBrewTime) {
		listener->setContainerData(this, 0, stand->brewTime);
		lastBrewTime = stand->brewTime;
	}
	if (stand->fuelCharges != lastFuelCharges) {
		listener->setContainerData(this, 1, stand->fuelCharges);
		lastFuelCharges = stand->fuelCharges;
	}
}

void BrewingMenu::setListener(IContainerListener* listener) {
	super::setListener(listener);

	if (listener) {
		listener->setContainerData(this, 0, stand->brewTime);
		listener->setContainerData(this, 1, stand->fuelCharges);
	}
}

bool BrewingMenu::tileEntityDestroyedIsInvalid(int tileEntityId) {
	return (tileEntityId == brewingStandTileEntityId);
}
