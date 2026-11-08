#ifndef _BREWINGSCREEN_H__
#define _BREWINGSCREEN_H__

// v20: brewing-stand screen. Cloned from FurnaceScreen (same touch UX:
// tap a slot to select it, hold an inventory item to move it in, tap
// the selected slot again to take items back out) and re-laid-out for
// the stand's 5 slots: 3 bottle slots in a column on the right,
// ingredient top-left, fuel below it, brew-progress arrow between the
// columns, fuel flame next to the fuel slot. The preview under the
// bottles shows what the ingredient would brew the first brewable
// bottle into (ghost icon + name, like the furnace's result preview).

#include "BaseContainerScreen.h"

#include "../components/InventoryPane.h"
#include "../components/Button.h"

class Font;
class CItem;
class Textures;
class NinePatchLayer;
class Tesselator;

class BrewingScreen:	public BaseContainerScreen,
						public Touch::IInventoryPaneCallback
{
	typedef BaseContainerScreen super;
	typedef std::vector<CItem*> ItemList;
public:
    BrewingScreen(Player* player, BrewingStandTileEntity* stand);
	~BrewingScreen();

    void init();
	void setupPositions();

	void tick();
	void render(int xm, int ym, float a);
	bool renderGameBehind();
	void buttonClicked(Button* button);

	// IInventoryPaneCallback
	bool addItem(const Touch::InventoryPane* pane, int itemId);
	bool isAllowed( int slot );
	std::vector<const ItemInstance*> getItems( const Touch::InventoryPane* forPane );
private:
	void recheckRecipes();

	void clearItems();
	void updateResult();
	void setupInventoryPane();
	void updateItems();

	void drawSlotItemAt(Tesselator& t, const ItemInstance* item, int x, int y, bool selected);
	ItemInstance moveOver(const ItemInstance* item, int maxCount);
	void takeAndClearSlot( int slot );
	bool handleAddItem( int slot, const ItemInstance* item );
	void handleRenderPane(Touch::InventoryPane* pane, Tesselator& t, int xm, int ym, float a);
	bool canMoveToBrewing(int inventorySlot, const ItemInstance* item);
	ItemList _items;

	std::string currentItemDesc;
	ItemInstance brewResult;
	float descWidth;
	ImageButton btnClose;
	BlankButton btnIngredient;
	BlankButton btnFuel;
	BlankButton btnBottle0;
	BlankButton btnBottle1;
	BlankButton btnBottle2;
	Touch::THeader bHeader;

	Touch::InventoryPane* inventoryPane;
	IntRectangle inventoryPaneRect;

	ItemList listFuel;
	ItemList listIngredient;
	ItemList listBottles;
	std::vector<int> inventorySlots;
	std::vector<const ItemInstance*> inventoryItems;
	bool doRecreatePane;

	int selectedSlot;
	int lastBrewTypeId;

	// GUI elements such as 9-Patches
	NinePatchLayer* guiBackground;
	NinePatchLayer* guiSlot;
	NinePatchLayer* guiSlotMarked;
	NinePatchLayer* guiSlotMarker;
	NinePatchLayer* guiPaneFrame;
	Player* player;
	BrewingStandTileEntity* stand;
};

#endif /*_BREWINGSCREEN_H__*/
