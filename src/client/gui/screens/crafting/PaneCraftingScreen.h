#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS_CRAFT_PaneCraftingScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS_CRAFT_PaneCraftingScreen_H__

#include "../../Screen.h"
#include "../../components/ImageButton.h"
#include "../../components/ItemPane.h"
#include "../../components/SmallButton.h"
#include "../../../../world/item/crafting/Recipes.h"
#include "../../../../world/item/ItemInstance.h"

class Font;
class CItem;
class Textures;
class NinePatchLayer;

// v42: modern recipe-book layout (see image-22.png): left dark panel
// with a search bar, category tabs, a 5x4 recipe grid + pagination;
// right light panel with a 3x3 crafting grid (auto-filled from the
// selected recipe), an arrow + result slot (click to craft), and the
// player inventory grid below. Replaces the old scrolling text list
// (ItemPane) — the click-a-recipe-to-craft flow itself is unchanged.
class PaneCraftingScreen: public Screen
{
	typedef Screen super;
	typedef std::vector<CItem*> ItemList;
public:
    PaneCraftingScreen(int craftingSize);
	~PaneCraftingScreen();

    void init();
	void setupPositions();

	void render(int xm, int ym, float a);
	bool renderGameBehind();
    bool closeOnPlayerHurt();
	void buttonClicked(Button* button);
	void keyPressed( int eventKey );
	void keyboardNewChar(char inputChar);
	bool handleBackEvent(bool isDown);
	void mouseReleased(int x, int y, int buttonNum);
	void removed();
protected:
	void setSingleCategoryAndIcon(int categoryBitmask, int categoryIcon);
private:
	/// Filter out non craftable recipes.
	/// The default implementation calls bool filterRecipe(r) for every
	/// Recipe r and keeps the ones that returned true.
	/// A crafting size filter has already been applied.
	virtual void filterRecipes(RecipeList& recipes);
	virtual bool filterRecipe(const Recipe& recipe) = 0;

	void initCategories();

	void addItem(Recipe* recipe);
	void recheckRecipes();
	void onItemSelected(int buttonIndex, CItem* item);
	void buildInventoryPack(ItemPack& ip);
	void applyFilter();
	void fillCraftGrid();
	int inventoryCountFor(const ItemInstance& item, ItemPack& ip);

	void craftSelectedItem();
	std::vector<ImageButton*> _categoryButtons;

	ItemList _items;
	std::vector<ItemList> _categories;

	// v42: filtered view of the current tab (search + pagination).
	ItemList _filteredItems;
	int currentCategory;
	int currentPage;
	int totalPages;
	CItem* currentItem;
	std::string currentItemDesc;
	ImageButton btnClose;
	SmallButton btnPagePrev;
	SmallButton btnPageNext;

	// v42: live search (soft keyboard via TextEditScreen pattern).
	std::string searchQuery;
	bool searchFocused;

	// v42: auto-filled 3x3 grid for the selected recipe.
	ItemInstance gridCells[9];
	int gridHas[9];
	bool gridFilled;

	int craftingSize;

	int numCategories;
	std::vector<int> categoryBitmasks;
	// Spritesheet icon per tab (-1 = draw the compass item instead).
	std::vector<int> categorySpriteIcons;
	ImageButton* selectedCategoryButton;

	// v42: layout (computed in setupPositions from width/height).
	static const int GridCols = 5;
	static const int GridRows = 4;
	static const int GridPageSize = GridCols * GridRows;
	static const int SlotSize = 20;
	IntRectangle leftPanelRect;
	IntRectangle rightPanelRect;
	IntRectangle searchRect;
	IntRectangle recipeGridRect;
	IntRectangle craftGridRect;
	IntRectangle resultRect;
	IntRectangle invGridRect;
	IntRectangle pageLabelRect;

	// GUI elements such as 9-Patches
	NinePatchLayer* guiBackground;
	NinePatchLayer* guiSlotCategory;
	NinePatchLayer* guiSlotCategorySelected;
};

#endif /*NET_MINECRAFT_CLIENT_GUI_SCREENS_CRAFT_PaneCraftingScreen_H__*/
