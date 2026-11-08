#ifndef NET_MINECRAFT_WORLD_INVENTORY__BrewingMenu_H__
#define NET_MINECRAFT_WORLD_INVENTORY__BrewingMenu_H__

// v20: brewing-stand container menu. Mirrors FurnaceMenu: the screen
// edits the tile entity through setSlot(), and brewTime / fuelCharges
// are synced to the client as container data ids 0 / 1.

#include "BaseContainerMenu.h"
#include <vector>

class BrewingStandTileEntity;
class ItemInstance;

class BrewingMenu: public BaseContainerMenu
{
	typedef BaseContainerMenu super;
public:
    BrewingMenu(BrewingStandTileEntity* stand);

    virtual ~BrewingMenu() {}

    virtual void setSlot(int slot, ItemInstance* item);
	virtual void setData(int id, int value);
	virtual std::vector<ItemInstance> getItems();

	virtual void broadcastChanges();
	virtual void setListener(IContainerListener* listener);

	virtual bool tileEntityDestroyedIsInvalid(int tileEntityId);

	BrewingStandTileEntity* stand;

	int brewingStandTileEntityId;
private:
	int lastBrewTime;
	int lastFuelCharges;
};

#endif /*NET_MINECRAFT_WORLD_INVENTORY__BrewingMenu_H__*/
