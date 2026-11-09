#include "PaneCraftingScreen.h"
#include <cstdio>
#include <cstring>
#include "../touch/TouchStartMenuScreen.h"
#include "../../Screen.h"
#include "../../components/NinePatch.h"
#include "../../../Minecraft.h"
#include "../../../player/LocalPlayer.h"
#include "../../../renderer/Tesselator.h"
#include "../../../renderer/entity/ItemRenderer.h"
#include "../../../../platform/input/Keyboard.h"
#include "../../../../platform/input/Mouse.h"
#include "../../../../world/item/Item.h"
#include "../../../../world/item/crafting/Recipes.h"
#include "../../../../world/item/ItemCategory.h"
#include "../../../../world/entity/player/Inventory.h"
#include "../../../../util/StringUtils.h"
#include "../../../../locale/I18n.h"
#include "../../../../world/entity/item/ItemEntity.h"
#include "../../../../world/level/Level.h"
#include "../../../../world/item/DyePowderItem.h"
#include "../../../../world/item/crafting/Recipe.h"

const float BorderPixels = 6.0f;

const int rgbActive = 0xfff0f0f0;
const int rgbInactive = 0xc0635558;
// v42 palette for the modern layout
const int rgbPanelDark = 0xFF2E2E2E;
const int rgbPanelBorder = 0xFF9A9A9A;
const int rgbSearchField = 0xFF1E1E1E;
const int rgbSearchHint = 0xFF808080;
const int rgbTitleDark = 0xFF3F3F3F;
const int rgbDimOverlay = 0x80000000;

// v42: case-insensitive substring match for the live search box.
static bool containsIgnoreCase(const std::string& haystack, const std::string& needle) {
	if (needle.empty())
		return true;
	for (size_t i = 0; i + needle.size() <= haystack.size(); ++i) {
		bool match = true;
		for (size_t j = 0; j < needle.size(); ++j) {
			char a = haystack[i + j];
			char b = needle[j];
			if (a >= 'A' && a <= 'Z') a += 'a' - 'A';
			if (b >= 'A' && b <= 'Z') b += 'a' - 'A';
			if (a != b) { match = false; break; }
		}
		if (match)
			return true;
	}
	return false;
}

// v42: draw one line of text at 2/3 scale (same trick the old pane used
// for the have/need counts), so "12/64" fits inside a 20px slot.
static void drawSmallText(Font* font, const std::string& str, float x, float y, int color) {
	Tesselator& t = Tesselator::instance;
	t.beginOverride();
	t.scale2d(0.6667f, 0.6667f);
	font->drawShadow(str, 1.5f * x, 1.5f * y, color);
	t.resetScale();
	t.endOverrideAndDraw();
}

class CategoryButton: public ImageButton {
	typedef ImageButton super;
public:
	CategoryButton(int id, const ImageButton* const* selectedPtr, NinePatchLayer* stateNormal, NinePatchLayer* statePressed)
	:	super(id, ""),
		selectedPtr(selectedPtr),
		stateNormal(stateNormal),
		statePressed(statePressed),
		hasIconItem(false)
	{}

	// v42: draw a real item icon (compass for the "All" tab) instead of
	// a spritesheet region.
	void setIconItem(const ItemInstance& item) {
		iconItem = item;
		hasIconItem = true;
	}

	void renderBg(Minecraft* minecraft, int xm, int ym) {
		bool hovered = active && (minecraft->useTouchscreen()?
			(_currentlyDown && xm >= x && ym >= y && xm < x + width && ym < y + height) : false);

		if (hovered || *selectedPtr == this)
			statePressed->draw(Tesselator::instance, (float)x, (float)y);
		else
			stateNormal->draw(Tesselator::instance, (float)x, (float)y);

		if (hasIconItem) {
			ItemRenderer::renderGuiItem(NULL, minecraft->textures, &iconItem,
				(float)(x + (width - 16) / 2), (float)(y + (height - 16) / 2), 16, 16, false);
		}
	}
	bool isSecondImage(bool hovered) { return false; }

private:
	const ImageButton* const* selectedPtr;
	NinePatchLayer* stateNormal;
	NinePatchLayer* statePressed;
	ItemInstance iconItem;
	bool hasIconItem;
};

PaneCraftingScreen::PaneCraftingScreen(int craftingSize)
:	craftingSize(craftingSize),
	currentCategory(0),
	currentPage(0),
	totalPages(1),
	currentItem(NULL),
	btnClose(2, ""),
	btnPagePrev(3, 0, 0, 18, 16, "<"),
	btnPageNext(4, 0, 0, 18, 16, ">"),
	searchFocused(false),
	gridFilled(false),
	selectedCategoryButton(NULL),
	guiBackground(NULL),
	guiSlotCategory(NULL),
	guiSlotCategorySelected(NULL),
	numCategories(5)
{
	// v42: five tabs like the modern recipe book (screenshot):
	// All (compass), Tools, Blocks (Structures + Decorations),
	// Food & Armor, Mechanisms (redstone). Bitmask 0 = show all.
	// NOTE: ItemCategory's static consts have no out-of-line
	// definition, so push_back(const int&) would odr-use them (link
	// error: undefined reference). Copy to locals first — the
	// lvalue-to-rvalue conversion there is not an odr-use.
	const int toolsCat = ItemCategory::Tools;
	const int blocksCat = ItemCategory::Structures | ItemCategory::Decorations;
	const int foodArmorCat = ItemCategory::FoodArmor;
	const int mechanismsCat = ItemCategory::Mechanisms;
	categoryBitmasks.push_back(0);
	categoryBitmasks.push_back(toolsCat);
	categoryBitmasks.push_back(blocksCat);
	categoryBitmasks.push_back(foodArmorCat);
	categoryBitmasks.push_back(mechanismsCat);
	// Spritesheet icons (see gui/spritesheet.png): -1 = compass item,
	// 1 = pickaxe, 0 = planks, 2 = armor+apple, 4 = rails.
	categorySpriteIcons.push_back(-1);
	categorySpriteIcons.push_back(1);
	categorySpriteIcons.push_back(0);
	categorySpriteIcons.push_back(2);
	categorySpriteIcons.push_back(4);

	for (int i = 0; i < 9; ++i) {
		gridHas[i] = 0;
	}
}

PaneCraftingScreen::~PaneCraftingScreen() {
	for (unsigned int i = 0; i < _items.size(); ++i)
		delete _items[i];

	for (unsigned int i = 0; i < _categoryButtons.size(); ++i)
		delete _categoryButtons[i];

	delete guiBackground;
	delete guiSlotCategory;
	delete guiSlotCategorySelected;
}

void PaneCraftingScreen::init() {
	ImageDef def;
	def.name = "gui/spritesheet.png";
	def.x = 0;
	def.y = 1;
	def.width = def.height = 18;
	def.setSrc(IntRectangle(60, 0, 18, 18));
	btnClose.setImageDef(def, true);
	btnClose.scaleWhenPressed = false;

	buttons.push_back(&btnClose);
	buttons.push_back(&btnPagePrev);
	buttons.push_back(&btnPageNext);

	// GUI patches
	NinePatchFactory builder(minecraft->textures, "gui/spritesheet.png");

	guiBackground   = builder.createSymmetrical(IntRectangle(0, 0, 16, 16), 4, 4);
	guiSlotCategory = builder.createSymmetrical(IntRectangle(8, 32, 8, 8), 2, 2);
	guiSlotCategorySelected = builder.createSymmetrical(IntRectangle(0, 32, 8, 8), 2, 2);
	guiSlotCategory->setSize((float)SlotSize, (float)SlotSize);
	guiSlotCategorySelected->setSize((float)SlotSize, (float)SlotSize);

	initCategories();
}

void PaneCraftingScreen::initCategories() {
	_categories.resize(numCategories);

	// Category buttons
	for (int i = 0; i < numCategories; ++i) {
		ImageButton* button = new CategoryButton(100 + i, &selectedCategoryButton, guiSlotCategory, guiSlotCategorySelected);
		_categoryButtons.push_back( button );
		buttons.push_back( button );
	}

	const RecipeList& all = Recipes::getInstance()->getRecipes();
	RecipeList filtered;
	filtered.reserve(all.size());

	// Apply size filter
	for (unsigned int i = 0; i < all.size(); ++i) {
		if (craftingSize >= all[i]->getCraftingSize())
			filtered.push_back(all[i]);
	}
	// Filter by subclass impl
	filterRecipes(filtered);

	// Add items from filtered recipes
	for (unsigned int i = 0; i < filtered.size(); ++i)
		addItem(filtered[i]);

	recheckRecipes();

	currentCategory = 0;
	currentPage = 0;
	applyFilter();
	if (!_categoryButtons.empty())
		selectedCategoryButton = (CategoryButton*)_categoryButtons[0];
	if (!_filteredItems.empty())
		onItemSelected(0, _filteredItems[0]);
}

void PaneCraftingScreen::setupPositions() {
	// Tabs column on the far left
	const int tabW = 26;
	const int tabH = 26;
	const int tabGap = 2;
	for (unsigned c = 0; c < _categoryButtons.size(); ++c) {
		ImageButton* button = _categoryButtons[c];
		button->x = (int)BorderPixels;
		button->y = (int)BorderPixels + c * (tabH + tabGap);
		button->width = tabW;
		button->height = tabH;

		int icon = categorySpriteIcons[c];
		if (icon < 0) {
			// "All" tab: real compass item icon.
			((CategoryButton*)button)->setIconItem(ItemInstance(Item::compass, 1));
		} else {
			ImageDef def;
			def.x = 0;
			def.width = def.height = (float)tabH;
			def.name = "gui/spritesheet.png";
			def.setSrc(IntRectangle(32 * (icon/2), 64 + (icon&1) * 32, 32, 32));
			button->setImageDef(def, false);
		}
	}

	// Panels: left (dark recipe book) ~46%, right (light crafting) rest.
	const int availX = (int)BorderPixels + tabW + 2;
	const int availW = width - availX - (int)BorderPixels;
	const int leftW = availW * 46 / 100;
	const int rightW = availW - leftW - 6;
	const int panelY = (int)BorderPixels;
	const int panelH = height - 2 * (int)BorderPixels;
	leftPanelRect = IntRectangle(availX, panelY, leftW, panelH);
	rightPanelRect = IntRectangle(availX + leftW + 6, panelY, rightW, panelH);

	// Left panel: search bar, 5x4 recipe grid, pagination.
	searchRect = IntRectangle(leftPanelRect.x + 6, panelY + 6, leftW - 12, 18);
	const int gridW = GridCols * SlotSize;
	const int gridH = GridRows * SlotSize;
	const int gridX = leftPanelRect.x + (leftW - gridW) / 2;
	const int gridY = searchRect.y + searchRect.h + 6;
	recipeGridRect = IntRectangle(gridX, gridY, gridW, gridH);

	const int pageY = gridY + gridH + 6;
	btnPagePrev.x = gridX;
	btnPagePrev.y = pageY;
	btnPagePrev.width = 18;
	btnPagePrev.height = 16;
	btnPageNext.x = gridX + gridW - 18;
	btnPageNext.y = pageY;
	btnPageNext.width = 18;
	btnPageNext.height = 16;
	pageLabelRect = IntRectangle(gridX + 20, pageY, gridW - 40, 16);

	// Right panel: "Crafting" 3x3 grid + arrow + result slot, then
	// "Inventory" 9x4 grid.
	const int craftBlockW = 3 * SlotSize + 6 + 20 + 6 + SlotSize;
	const int craftBlockX = rightPanelRect.x + (rightW - craftBlockW) / 2;
	const int craftY = panelY + 24;
	craftGridRect = IntRectangle(craftBlockX, craftY, 3 * SlotSize, 3 * SlotSize);
	resultRect = IntRectangle(craftBlockX + 3 * SlotSize + 6 + 20 + 6, craftY, SlotSize, SlotSize);

	const int invW = 9 * SlotSize;
	const int invH = 4 * SlotSize;
	invGridRect = IntRectangle(rightPanelRect.x + (rightW - invW) / 2, craftY + 3 * SlotSize + 26, invW, invH);

	btnClose.width = btnClose.height = 19;
	btnClose.x = width - btnClose.width;
	btnClose.y = 0;

	guiBackground->setSize((float)width, (float)height);
}

void PaneCraftingScreen::render(int xm, int ym, float a) {
	Tesselator& t = Tesselator::instance;
	guiBackground->draw(t, 0, 0);
	glEnable2(GL_ALPHA_TEST);

	// Panels: dark recipe book on the left, light crafting on the right.
	fill(leftPanelRect.x, leftPanelRect.y,
		leftPanelRect.x + leftPanelRect.w, leftPanelRect.y + leftPanelRect.h, rgbPanelDark);
	fill(leftPanelRect.x, leftPanelRect.y,
		leftPanelRect.x + leftPanelRect.w, leftPanelRect.y + 1, rgbPanelBorder);
	fill(leftPanelRect.x, leftPanelRect.y + leftPanelRect.h - 1,
		leftPanelRect.x + leftPanelRect.w, leftPanelRect.y + leftPanelRect.h, rgbPanelBorder);
	fill(leftPanelRect.x, leftPanelRect.y,
		leftPanelRect.x + 1, leftPanelRect.y + leftPanelRect.h, rgbPanelBorder);
	fill(leftPanelRect.x + leftPanelRect.w - 1, leftPanelRect.y,
		leftPanelRect.x + leftPanelRect.w, leftPanelRect.y + leftPanelRect.h, rgbPanelBorder);
	fill(rightPanelRect.x, rightPanelRect.y,
		rightPanelRect.x + rightPanelRect.w, rightPanelRect.y + 1, rgbPanelBorder);
	fill(rightPanelRect.x, rightPanelRect.y + rightPanelRect.h - 1,
		rightPanelRect.x + rightPanelRect.w, rightPanelRect.y + rightPanelRect.h, rgbPanelBorder);
	fill(rightPanelRect.x, rightPanelRect.y,
		rightPanelRect.x + 1, rightPanelRect.y + rightPanelRect.h, rgbPanelBorder);
	fill(rightPanelRect.x + rightPanelRect.w - 1, rightPanelRect.y,
		rightPanelRect.x + rightPanelRect.w, rightPanelRect.y + rightPanelRect.h, rgbPanelBorder);

	// Buttons (tabs + close + pagination)
	super::render(xm, ym, a);

	// Search bar
	fill(searchRect.x, searchRect.y,
		searchRect.x + searchRect.w, searchRect.y + searchRect.h, rgbSearchField);
	fill(searchRect.x, searchRect.y,
		searchRect.x + searchRect.w, searchRect.y + 1, rgbPanelBorder);
	fill(searchRect.x, searchRect.y + searchRect.h - 1,
		searchRect.x + searchRect.w, searchRect.y + searchRect.h, rgbPanelBorder);
	fill(searchRect.x, searchRect.y,
		searchRect.x + 1, searchRect.y + searchRect.h, rgbPanelBorder);
	fill(searchRect.x + searchRect.w - 1, searchRect.y,
		searchRect.x + searchRect.w, searchRect.y + searchRect.h, rgbPanelBorder);
	if (searchQuery.empty())
		font->drawShadow("Search...", (float)(searchRect.x + 4), (float)(searchRect.y + 5), rgbSearchHint);
	else {
		std::string shown = searchQuery;
		if (searchFocused)
			shown += "_";
		font->drawShadow(shown, (float)(searchRect.x + 4), (float)(searchRect.y + 5), rgbActive);
	}

	// Recipe grid (current page, 5x4 icons)
	const int pageBase = currentPage * GridPageSize;
	t.beginOverride();
	for (int i = 0; i < GridPageSize; ++i) {
		if (pageBase + i >= (int)_filteredItems.size())
			break;
		CItem* citem = _filteredItems[pageBase + i];
		const int cx = recipeGridRect.x + (i % GridCols) * SlotSize;
		const int cy = recipeGridRect.y + (i / GridCols) * SlotSize;
		(citem == currentItem ? guiSlotCategorySelected : guiSlotCategory)->draw(t, (float)cx, (float)cy);
	}
	t.endOverrideAndDraw();
	t.beginOverride();
	for (int i = 0; i < GridPageSize; ++i) {
		if (pageBase + i >= (int)_filteredItems.size())
			break;
		CItem* citem = _filteredItems[pageBase + i];
		const int cx = recipeGridRect.x + (i % GridCols) * SlotSize;
		const int cy = recipeGridRect.y + (i / GridCols) * SlotSize;
		ItemRenderer::renderGuiItem(NULL, minecraft->textures, &citem->item,
			(float)(cx + 2), (float)(cy + 2), 16, 16, false);
	}
	t.endOverrideAndDraw();
	for (int i = 0; i < GridPageSize; ++i) {
		if (pageBase + i >= (int)_filteredItems.size())
			break;
		CItem* citem = _filteredItems[pageBase + i];
		if (citem->canCraft())
			continue;
		const int cx = recipeGridRect.x + (i % GridCols) * SlotSize;
		const int cy = recipeGridRect.y + (i / GridCols) * SlotSize;
		fill(cx, cy, cx + SlotSize, cy + SlotSize, rgbDimOverlay);
	}

	// Pagination label "N/M"
	char pageBuf[16];
	snprintf(pageBuf, sizeof(pageBuf), "%d/%d", currentPage + 1, totalPages);
	drawCenteredString(font, pageBuf,
		pageLabelRect.x + pageLabelRect.w / 2, pageLabelRect.y + 4, rgbActive);

	// Selected recipe name + description (left panel, under pagination)
	if (currentItem) {
		font->drawShadow(currentItem->text,
			(float)(leftPanelRect.x + 6), (float)(pageLabelRect.y + 24), rgbActive);
		font->drawWordWrap(currentItemDesc,
			(float)(leftPanelRect.x + 6), (float)(pageLabelRect.y + 36),
			(float)(leftPanelRect.w - 12), rgbActive);
	}

	// Right panel titles
	font->drawShadow("Crafting", (float)craftGridRect.x, (float)(craftGridRect.y - 16), rgbTitleDark);
	font->drawShadow("Inventory", (float)invGridRect.x, (float)(invGridRect.y - 14), rgbTitleDark);

	// 3x3 crafting grid (auto-filled from the selected recipe)
	t.beginOverride();
	for (int i = 0; i < 9; ++i) {
		const int cx = craftGridRect.x + (i % 3) * SlotSize;
		const int cy = craftGridRect.y + (i / 3) * SlotSize;
		guiSlotCategory->draw(t, (float)cx, (float)cy);
	}
	guiSlotCategory->draw(t, (float)resultRect.x, (float)resultRect.y);
	t.endOverrideAndDraw();
	if (gridFilled) {
		t.beginOverride();
		for (int i = 0; i < 9; ++i) {
			if (gridCells[i].isNull())
				continue;
			const int cx = craftGridRect.x + (i % 3) * SlotSize;
			const int cy = craftGridRect.y + (i / 3) * SlotSize;
			ItemRenderer::renderGuiItem(NULL, minecraft->textures, &gridCells[i],
				(float)(cx + 2), (float)(cy + 2), 16, 16, false);
		}
		t.endOverrideAndDraw();
		// have/need under each filled cell
		for (int i = 0; i < 9; ++i) {
			if (gridCells[i].isNull())
				continue;
			const int cx = craftGridRect.x + (i % 3) * SlotSize;
			const int cy = craftGridRect.y + (i / 3) * SlotSize;
			char buf[16];
			int n = 0;
			n += Gui::itemCountItoa(&buf[n], gridHas[i]);
			strcpy(&buf[n], "/"); n += 1;
			n += Gui::itemCountItoa(&buf[n], gridCells[i].count);
			buf[n] = 0;
			drawSmallText(font, buf, (float)(cx + 2), (float)(cy + SlotSize - 6),
				gridHas[i] >= gridCells[i].count ? rgbActive : rgbInactive);
		}
	}
	// Arrow grid -> result
	{
		t.beginOverride();
		t.scale2d(2.0f, 2.0f);
		const float ax = 0.5f * (float)(craftGridRect.x + 3 * SlotSize + 8);
		const float ay = 0.5f * (float)(craftGridRect.y + (3 * SlotSize - 16) / 2);
		font->drawShadow(">", ax, ay, rgbTitleDark);
		t.resetScale();
		t.endOverrideAndDraw();
	}
	// Result slot: click to craft. Highlighted when craftable.
	if (currentItem) {
		t.beginOverride();
		if (currentItem->canCraft())
			guiSlotCategorySelected->draw(t, (float)resultRect.x, (float)resultRect.y);
		ItemRenderer::renderGuiItem(NULL, minecraft->textures, &currentItem->item,
			(float)(resultRect.x + 2), (float)(resultRect.y + 2), 16, 16, false);
		t.endOverrideAndDraw();
		if (!currentItem->canCraft())
			fill(resultRect.x, resultRect.y,
				resultRect.x + SlotSize, resultRect.y + SlotSize, rgbDimOverlay);
	}

	// Inventory grid (display-only in stage 1): rows 0-2 = main
	// inventory slots 9-35, row 3 = hotbar slots 0-8.
	Inventory* inv = (minecraft->player ? minecraft->player->inventory : NULL);
	t.beginOverride();
	for (int i = 0; i < 36; ++i) {
		const int cx = invGridRect.x + (i % 9) * SlotSize;
		const int cy = invGridRect.y + (i / 9) * SlotSize;
		guiSlotCategory->draw(t, (float)cx, (float)cy);
	}
	t.endOverrideAndDraw();
	if (inv) {
		t.beginOverride();
		for (int i = 0; i < 36; ++i) {
			const int slot = (i < 27) ? (9 + i) : (i - 27);
			ItemInstance* item = (slot < inv->getContainerSize()) ? inv->getItem(slot) : NULL;
			if (!item)
				continue;
			const int cx = invGridRect.x + (i % 9) * SlotSize;
			const int cy = invGridRect.y + (i / 9) * SlotSize;
			ItemRenderer::renderGuiItem(NULL, minecraft->textures, item,
				(float)(cx + 2), (float)(cy + 2), 16, 16, false);
		}
		t.endOverrideAndDraw();
		for (int i = 0; i < 36; ++i) {
			const int slot = (i < 27) ? (9 + i) : (i - 27);
			ItemInstance* item = (slot < inv->getContainerSize()) ? inv->getItem(slot) : NULL;
			if (!item || item->count <= 1)
				continue;
			const int cx = invGridRect.x + (i % 9) * SlotSize;
			const int cy = invGridRect.y + (i / 9) * SlotSize;
			char buf[16];
			int n = Gui::itemCountItoa(buf, item->count);
			buf[n] = 0;
			float tw = (float)font->width(buf) * 0.6667f;
			drawSmallText(font, buf, (float)(cx + SlotSize - 2) - tw, (float)(cy + SlotSize - 6), rgbActive);
		}
	}
}

void PaneCraftingScreen::buttonClicked(Button* button) {
	if (button == &btnClose) {
		minecraft->setScreen(NULL);
		return;
	}

	if (button == &btnPagePrev) {
		if (currentPage > 0)
			--currentPage;
		return;
	}
	if (button == &btnPageNext) {
		if (currentPage < totalPages - 1)
			++currentPage;
		return;
	}

	// Did we click a category?
	if (button->id >= 100 && button->id < 200) {
		int categoryId = button->id - 100;
		currentCategory = categoryId;
		currentPage = 0;
		selectedCategoryButton = (CategoryButton*)button;
		applyFilter();
		if (!_filteredItems.empty())
			onItemSelected(categoryId, _filteredItems[0]);
	}
}

void PaneCraftingScreen::mouseReleased(int x, int y, int buttonNum) {
	super::mouseReleased(x, y, buttonNum);
	if (buttonNum != MouseAction::ACTION_LEFT)
		return;

	// Search bar: tap to focus (opens the soft keyboard), tap
	// elsewhere to unfocus.
	if (x >= searchRect.x && y >= searchRect.y
		&& x < searchRect.x + searchRect.w && y < searchRect.y + searchRect.h) {
		if (!searchFocused) {
			searchFocused = true;
			minecraft->platform()->showKeyboard();
		}
		return;
	}
	if (searchFocused) {
		searchFocused = false;
		minecraft->platform()->hideKeyboard();
	}

	// Recipe grid: tap a cell to select that recipe.
	if (x >= recipeGridRect.x && y >= recipeGridRect.y
		&& x < recipeGridRect.x + recipeGridRect.w && y < recipeGridRect.y + recipeGridRect.h) {
		int col = (x - recipeGridRect.x) / SlotSize;
		int row = (y - recipeGridRect.y) / SlotSize;
		int idx = currentPage * GridPageSize + row * GridCols + col;
		if (idx >= 0 && idx < (int)_filteredItems.size())
			onItemSelected(currentCategory, _filteredItems[idx]);
		return;
	}

	// Result slot: tap to craft (same as the old Craft button).
	if (x >= resultRect.x && y >= resultRect.y
		&& x < resultRect.x + resultRect.w && y < resultRect.y + resultRect.h) {
		craftSelectedItem();
		return;
	}
}

void PaneCraftingScreen::keyPressed(int eventKey) {
	if (searchFocused) {
		if (eventKey == Keyboard::KEY_BACKSPACE) {
			if (!searchQuery.empty()) {
				searchQuery.erase(searchQuery.size() - 1);
				applyFilter();
			}
			return;
		}
		if (eventKey == Keyboard::KEY_RETURN || eventKey == Keyboard::KEY_ESCAPE) {
			searchFocused = false;
			minecraft->platform()->hideKeyboard();
			return;
		}
		// While typing, ignore other keys (no tab-walking).
		return;
	}
	super::keyPressed(eventKey);
}

void PaneCraftingScreen::keyboardNewChar(char inputChar) {
	if (!searchFocused)
		return;
	if (inputChar >= 32 && inputChar < 127 && searchQuery.size() < 24) {
		searchQuery += inputChar;
		applyFilter();
	}
}

bool PaneCraftingScreen::handleBackEvent(bool isDown) {
	if (isDown && searchFocused) {
		searchFocused = false;
		minecraft->platform()->hideKeyboard();
		return true;
	}
	return false;
}

void PaneCraftingScreen::removed() {
	if (searchFocused) {
		searchFocused = false;
		minecraft->platform()->hideKeyboard();
	}
}

static void randomlyFillItemPack(ItemPack* ip, int numItems) {
	int added = 0;
	ItemInstance item(0, 1, 0);
	while (added < numItems) {
		int t = Mth::random(512);
		if (!Item::items[t]) continue;

		item.id = t;
		int id = ItemPack::getIdForItemInstance(&item);
		int count = Mth::random(10);
		for (int i = 0; i < count; ++i)
			ip->add(id);
		++added;
	}
}

static bool sortCanCraftPredicate(const CItem* a, const CItem* b) {
	//if (a->maxBuildCount == 0 && b->maxBuildCount > 0) return false;
	//if (b->maxBuildCount == 0 && a->maxBuildCount > 0) return true;
	return a->sortText < b->sortText;
}

void PaneCraftingScreen::buildInventoryPack(ItemPack& ip) {
	if (minecraft->player && minecraft->player->inventory) {
		Inventory* inv = (minecraft->player)->inventory;

		// NOTE: same quirk as before — only the main inventory (slots
		// 9+) counts toward craftability here, not the hotbar. The
		// grid have/need counts use the same pack so they agree with
		// canCraft(); craftSelectedItem itself removes via
		// removeResource, which searches the whole inventory.
		for (int i = Inventory::MAX_SELECTION_SIZE; i < inv->getContainerSize(); ++i) {
			if (ItemInstance* item = inv->getItem(i))
				ip.add(ItemPack::getIdForItemInstance(item), item->count);
		}
	}
}

int PaneCraftingScreen::inventoryCountFor(const ItemInstance& item, ItemPack& ip) {
	if (!Recipe::isAnyAuxValue(&item) && (item.getAuxValue() == Recipe::ANY_AUX_VALUE)) {
		// If the aux value on the item matters, but the recipe says it doesn't,
		// use this override (by fetching all items with aux-ids 0-15)
		ItemInstance aux(item);
		int has = 0;
		for (int i = 0; i < 16; ++i) {
			aux.setAuxValue(i);
			has += ip.getCount(ItemPack::getIdForItemInstance(&aux));
		}
		return has;
	}
	return ip.getCount(ItemPack::getIdForItemInstance(&item));
}

void PaneCraftingScreen::recheckRecipes() {
	ItemPack ip;

	if (minecraft->player && minecraft->player->inventory) {
		buildInventoryPack(ip);
	} else {
		randomlyFillItemPack(&ip, 50);
	}

	ip.print();

	Stopwatch w;
	w.start();

	for (unsigned int i = 0; i < _items.size(); ++i) {
		CItem* item = _items[i];
		item->neededItems.clear();
		item->setCanCraft(true);

		Recipe* recipe = item->recipe;
		item->inventoryCount = ip.getCount(ItemPack::getIdForItemInstance(&item->item));
		//item->maxBuildCount = recipe->getMaxCraftCount(ip);
		// Override the canCraft thing, since I'm too lazy
		// to fix the above (commented out) function
		std::vector<ItemInstance> items = recipe->getItemPack().getItemInstances();
		for (unsigned int j = 0; j < items.size(); ++j) {
			ItemInstance& jtem = items[j];
			int has = inventoryCountFor(jtem, ip);
			CItem::ReqItem req(jtem, has);
			item->neededItems.push_back(req);
			item->setCanCraft(item->canCraft() && req.enough());
		}
	}
	w.stop();
	w.printEvery(1, "> craft ");

	for (unsigned int c = 0; c < _categories.size(); ++c)
		std::stable_sort(_categories[c].begin(), _categories[c].end(), sortCanCraftPredicate);
}

void PaneCraftingScreen::applyFilter() {
	_filteredItems.clear();
	const ItemList& src = _categories[currentCategory];
	for (unsigned int i = 0; i < src.size(); ++i) {
		if (searchQuery.empty() || containsIgnoreCase(src[i]->text, searchQuery))
			_filteredItems.push_back(src[i]);
	}

	int pages = (int)(_filteredItems.size() + GridPageSize - 1) / GridPageSize;
	totalPages = pages < 1 ? 1 : pages;
	if (currentPage >= totalPages)
		currentPage = totalPages - 1;
	if (currentPage < 0)
		currentPage = 0;

	// Keep the selection only if it is still in the filtered view.
	bool stillThere = false;
	for (unsigned int i = 0; i < _filteredItems.size(); ++i) {
		if (_filteredItems[i] == currentItem) { stillThere = true; break; }
	}
	if (!stillThere) {
		currentItem = NULL;
		currentItemDesc.clear();
		gridFilled = false;
	}
}

void PaneCraftingScreen::fillCraftGrid() {
	for (int i = 0; i < 9; ++i) {
		gridCells[i].setNull();
		gridHas[i] = 0;
	}
	gridFilled = false;
	if (!currentItem || !currentItem->recipe)
		return;

	Recipe* recipe = currentItem->recipe;
	ItemPack ip;
	buildInventoryPack(ip);

	int w = recipe->getGridWidth();
	int h = recipe->getGridHeight();
	for (int y = 0; y < h && y < 3; ++y) {
		for (int x = 0; x < w && x < 3; ++x) {
			ItemInstance cell = recipe->getGridItem(x, y);
			if (cell.isNull())
				continue;
			gridCells[y * 3 + x] = cell;
			gridHas[y * 3 + x] = inventoryCountFor(cell, ip);
		}
	}
	gridFilled = true;
}

void PaneCraftingScreen::addItem( Recipe* recipe )
{
	ItemInstance instance = recipe->getResultItem();
	Item* item = instance.getItem();
	CItem* ci = new CItem(instance, recipe, instance.getName());//item->getDescriptionId());
	if (item->id == Tile::cloth->id)
		ci->sortText = "Wool " + ci->text;
	if (item->id == Item::dye_powder->id)
		ci->sortText = "ZDye " + ci->text;
	_items.push_back(ci);

	if (item->category < 0)
		return;

	for (int i = 0; i < (int)categoryBitmasks.size(); ++i) {
		int bitmask = categoryBitmasks[i];
		if (bitmask == 0 || (bitmask & item->category) != 0)
			_categories[i].push_back( ci );
	}
}

void PaneCraftingScreen::onItemSelected(int buttonIndex, CItem* item) {
	currentItem = item;
	currentItemDesc = I18n::getDescriptionString(currentItem->item);
	currentCategory = buttonIndex;
	fillCraftGrid();
}

void PaneCraftingScreen::craftSelectedItem()
{
	if (!currentItem)
		return;
    if (!currentItem->canCraft())
        return;

	ItemInstance resultItem = currentItem->item;

	if (minecraft->player) {
		// Remove all items required for the recipe and ...
		for (unsigned int i = 0; i < currentItem->neededItems.size(); ++i) {
			CItem::ReqItem& req = currentItem->neededItems[i];

            // If the recipe allows any aux-value as ingredients, first deplete
            // aux == 0 from inventory. Since I'm not sure if this always is
            // correct, let's only do it for ingredient sandstone for now.
            ItemInstance toRemove = req.item;

            if (Tile::sandStone->id == req.item.id
             && Recipe::ANY_AUX_VALUE == req.item.getAuxValue()) {
                 toRemove.setAuxValue(0);
                 toRemove.count = minecraft->player->inventory->removeResource(toRemove, true);
                 toRemove.setAuxValue(Recipe::ANY_AUX_VALUE);
            }

            if (toRemove.count > 0) {
                minecraft->player->inventory->removeResource(toRemove);
            }
		}
		// ... add the new one! (in this order, to fill empty slots better)
		// if it doesn't fit, throw it on the ground!
		if (!minecraft->player->inventory->add(&resultItem)) {
			minecraft->player->drop(new ItemInstance(resultItem), false);
		}

		recheckRecipes();
		applyFilter();
		fillCraftGrid();
	}
}

bool PaneCraftingScreen::renderGameBehind()
{
	return false;
}

bool PaneCraftingScreen::closeOnPlayerHurt() {
    return true;
}

void PaneCraftingScreen::filterRecipes(RecipeList& recipes) {
	for (int i = recipes.size() - 1; i >= 0; --i) {
		if (!filterRecipe(*recipes[i]))
			recipes.erase(recipes.begin() + i);
	}
}

void PaneCraftingScreen::setSingleCategoryAndIcon(int categoryBitmask, int categoryIcon) {
	assert(!minecraft && "setSingleCategoryAndIcon needs to be called from subclass constructor!\n");

	numCategories = 1;

	categorySpriteIcons.clear();
	categorySpriteIcons.push_back(categoryIcon);

	categoryBitmasks.clear();
	categoryBitmasks.push_back(categoryBitmask);
}
