#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE_ENTITY__BrewingStandTileEntity_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE_ENTITY__BrewingStandTileEntity_H__

// v20: brewing stand tile entity (tile 117). Mirrors FurnaceTileEntity:
// a Container with items + a tick() that advances brewing. Slots:
//   0-2  potion bottles (input + output in place, like modern MC)
//   3    ingredient (nether wart, sugar, blaze powder, ...)
//   4    fuel (blaze powder — 1 powder = 20 brew operations)
// One brew takes BREW_TIME ticks (400 = 20 s, modern MC). The
// ingredient is consumed once per brew, the fuel charge once per brew
// (a powder refills 20 charges when the stand runs dry).

#include "TileEntity.h"
#include "../../../Container.h"
#include "../../../item/ItemInstance.h"

class CompoundTag;
class Player;

class BrewingStandTileEntity: public TileEntity,
							  public Container
{
	typedef TileEntity super;
	static const int NumItems = 5;
public:
	static const int BREW_TIME = 400;
	static const int FUEL_PER_POWDER = 20;

	static const int SLOT_BOTTLE_0 = 0;
	static const int SLOT_BOTTLE_1 = 1;
	static const int SLOT_BOTTLE_2 = 2;
	static const int SLOT_INGREDIENT = 3;
	static const int SLOT_FUEL = 4;

	BrewingStandTileEntity();
	~BrewingStandTileEntity();

	// Container
	ItemInstance* getItem(int slot);
	void setItem(int slot, ItemInstance* item);
	ItemInstance removeItem(int slot, int count);

	std::string getName() const;
	int getMaxStackSize() const;
	int getContainerSize() const;

	bool stillValid(Player* player);

	void startOpen();
	void stopOpen();

	void setContainerChanged();

	// Brewing
	void load(CompoundTag* base);
	bool save(CompoundTag* base);
	bool shouldSave();

	int getBrewProgress(int max);
	int getFuelProgress(int max);

	bool isBrewing() const { return brewTime > 0; }
	bool isSlotEmpty(int slot);

	void tick();

private:
	bool canBrew();
	void brew();

public:
	int brewTime;
	int fuelCharges;
	ItemInstance items[NumItems];
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_TILE_ENTITY__BrewingStandTileEntity_H__*/
