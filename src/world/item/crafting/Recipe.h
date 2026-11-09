#ifndef NET_MINECRAFT_WORLD_ITEM_CRAFTING__Recipe_H__
#define NET_MINECRAFT_WORLD_ITEM_CRAFTING__Recipe_H__

//package net.minecraft.world.item.crafting;

//#include "../../inventory/CraftingContainer.h"
#include "../ItemInstance.h"
#include <map>
#include <vector>

class CraftingContainer;

class ItemPack {
public:
	typedef std::map<int, int> Map;
	//typedef std::vector<std::pair<int, int> > TypeCountList;

	void add(int id, int count = 1);
	int  getCount(int id) const;
	void print() const;

	int getMaxMultipliesOf(ItemPack& v) const;
	std::vector<ItemInstance> getItemInstances() const;

	static int getIdForItemInstance(const ItemInstance* ii);
	static int getIdForItemInstanceAnyAux( const ItemInstance* ii );
	static ItemInstance getItemInstanceForId(int id);
private:
	Map items;
};


class Recipe
{
public:
	static const int SIZE_2X2;// = 0;
	static const int SIZE_3X3;// = 1;
	static const int ANY_AUX_VALUE = -1;

	virtual ~Recipe() {}
    virtual bool matches(CraftingContainer* craftSlots) = 0;
	virtual const ItemPack& getItemPack() { return myItems; }
	virtual int getMaxCraftCount(ItemPack& fromItems) = 0;

    virtual int size() = 0;

    virtual ItemInstance assemble(CraftingContainer* craftSlots) = 0;
    virtual ItemInstance getResultItem() const = 0;

	static bool isAnyAuxValue(const ItemInstance* ii) {
		return isAnyAuxValue(ii->id);
	}

	virtual int getCraftingSize() = 0;

	// v42: 3x3 grid layout for the recipe book (PaneCraftingScreen).
	// Shaped recipes return their authored cell (x, y); shapeless
	// recipes fill row-major. Base returns an empty grid.
	virtual int getGridWidth() const { return 0; }
	virtual int getGridHeight() const { return 0; }
	virtual ItemInstance getGridItem(int x, int y) const { return ItemInstance(); }

	// v44: same layout, but at a given display width. Shaped recipes
	// ignore gridW (authored cells); shapeless recipes reflow
	// row-major at gridW, so a 4-ingredient shapeless recipe
	// (SIZE_2X2) lays out 2x2 instead of spilling into a 3rd column
	// of a 2x2 display. Default = the v42 accessors above.
	virtual int getGridWidthFor(int gridW) const { return getGridWidth(); }
	virtual int getGridHeightFor(int gridW) const { return getGridHeight(); }
	virtual ItemInstance getGridItemFor(int x, int y, int gridW) const { return getGridItem(x, y); }
private:
	static bool isAnyAuxValue(int id);
protected:
	ItemPack myItems;
};

#endif /*NET_MINECRAFT_WORLD_ITEM_CRAFTING__Recipe_H__*/
