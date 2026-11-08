#include "BrewingScreen.h"
#include "crafting/PaneCraftingScreen.h"
#include "../Screen.h"
#include "../components/NinePatch.h"
#include "../../Minecraft.h"
#include "../../player/LocalPlayer.h"
#include "../../renderer/Tesselator.h"
#include "../../renderer/entity/ItemRenderer.h"
#include "../../../world/item/Item.h"
#include "../../../world/item/PotionItem.h"
#include "../../../world/item/ItemCategory.h"
#include "../../../world/entity/player/Inventory.h"
#include "../../../world/entity/item/ItemEntity.h"
#include "../../../world/level/Level.h"
#include "../../../world/item/crafting/BrewingRecipes.h"
#include "../../../world/level/tile/entity/BrewingStandTileEntity.h"
#include "../../../locale/I18n.h"
#include "../../../util/StringUtils.h"
#include "../../../world/inventory/BrewingMenu.h"
#include "../../../network/packet/ContainerSetSlotPacket.h"
#include "../../../network/RakNetInstance.h"

// v20: brewing-stand screen, cloned from FurnaceScreen (same touch UX:
// tap a slot to select it, hold an inventory item to move it in, tap
// the selected slot again to take items back out). Layout: ingredient
// top-left, fuel below it, 3 bottle slots in a column on the right,
// brew-progress arrow between the columns, fuel flame next to the
// fuel slot. The preview under the bottles shows what the ingredient
// would brew the first brewable bottle into (ghost icon + name).

static int heldMs = -1;
static int percent = -1;
static const float MaxHoldMs = 500.0f;
static const int MinChargeMs = 200;

static void setIfNotSet(bool& ref, bool condition) {
	ref = (ref || condition);
}

const int rgbActive = 0xfff0f0f0;
const int rgbInactive = 0xc0635558;
const int rgbInactiveShadow = 0xc0aaaaaa;

#ifdef __APPLE__
	static const float BorderPixels = 4;
	#ifdef DEMO_MODE
		static const float BlockPixels = 22;
	#else
		static const float BlockPixels = 22;
	#endif
#else
	static const float BorderPixels = 4;
	static const float BlockPixels = 24;
#endif
static const int ItemSize = (int)(BlockPixels + 2*BorderPixels);

static const int Bx = 10; // Border Frame width
static const int By = 6; // Border Frame height


BrewingScreen::BrewingScreen(Player* player, BrewingStandTileEntity* stand)
:	super(new BrewingMenu(stand)), //@huge @attn
	inventoryPane(NULL),
	btnFuel(BrewingStandTileEntity::SLOT_FUEL),
	btnIngredient(BrewingStandTileEntity::SLOT_INGREDIENT),
	btnBottle0(BrewingStandTileEntity::SLOT_BOTTLE_0),
	btnBottle1(BrewingStandTileEntity::SLOT_BOTTLE_1),
	btnBottle2(BrewingStandTileEntity::SLOT_BOTTLE_2),
	btnClose(4, ""),
	bHeader (5, "Brewing Stand"),
	guiBackground(NULL),
	guiSlot(NULL),
	guiSlotMarked(NULL),
	guiSlotMarker(NULL),
	guiPaneFrame(NULL),
	player(player),
	stand(stand),
	selectedSlot(BrewingStandTileEntity::SLOT_BOTTLE_0),
	doRecreatePane(false),
	lastBrewTypeId(0),
	descWidth(90)
{
}

BrewingScreen::~BrewingScreen() {
	clearItems();

	delete inventoryPane;

	delete guiBackground;
	delete guiSlot;
	delete guiSlotMarked;
	delete guiSlotMarker;
	delete guiPaneFrame;

	delete menu;

	if (stand->clientSideOnly)
		delete stand;
}

void BrewingScreen::init() {
	super::init();

	ImageDef def;
	def.name = "gui/spritesheet.png";
	def.x = 0;
	def.y = 1;
	def.width = def.height = 18;
	def.setSrc(IntRectangle(60, 0, 18, 18));
	btnClose.setImageDef(def, true);
	btnClose.scaleWhenPressed = false;

	buttons.push_back(&bHeader);
	buttons.push_back(&btnIngredient);
	buttons.push_back(&btnFuel);
	buttons.push_back(&btnBottle0);
	buttons.push_back(&btnBottle1);
	buttons.push_back(&btnBottle2);
	buttons.push_back(&btnClose);

	// GUI - nine patches
	NinePatchFactory builder(minecraft->textures, "gui/spritesheet.png");

	guiBackground   = builder.createSymmetrical(IntRectangle(0, 0, 16, 16), 4, 4);
	guiSlot         = builder.createSymmetrical(IntRectangle(0, 32, 8, 8), 3, 3);
	guiSlotMarked   = builder.createSymmetrical(IntRectangle(0, 44, 8, 8), 3, 3);
	guiSlotMarker   = builder.createSymmetrical(IntRectangle(10, 42, 16, 16), 5, 5);
	guiPaneFrame    = builder.createSymmetrical(IntRectangle(28, 42, 4, 4), 1, 1)->setExcluded(1 << 4);

	recheckRecipes();
}

void BrewingScreen::setupPositions() {
	// Left  - Categories
	bHeader.x = bHeader.y = 0;
	bHeader.width = width;

	btnClose.width = btnClose.height = 19;
	btnClose.x = width - btnClose.width;
	btnClose.y = 0;

	// Inventory pane
	const int maxWidth = width/2 - Bx - Bx;
	const int InventoryColumns = maxWidth / ItemSize;
	const int realWidth = InventoryColumns * ItemSize;
	const int paneWidth = realWidth + Bx + Bx;
	const int realBx = (paneWidth - realWidth) / 2;

	inventoryPaneRect = IntRectangle(realBx,
#ifdef __APPLE__
		26 + By - ((width==240)?1:0), realWidth, ((width==240)?1:0) + height-By-By-28);
#else
		26 + By, realWidth, height-By-By-28);
#endif

	// Right - Slots, description etc. Two columns, like the furnace:
	// ingredient + fuel on the left, the 3 bottle slots on the right.
	{
	int cx = (inventoryPaneRect.x + inventoryPaneRect.w);
	int rightWidth = width - cx;

	btnIngredient.width = btnFuel.width = btnBottle0.width = btnBottle1.width = btnBottle2.width = (int)guiSlot->getWidth();
	btnIngredient.height = btnFuel.height = btnBottle0.height = btnBottle1.height = btnBottle2.height = (int)guiSlot->getHeight();
	int space = rightWidth - (2 * btnFuel.width + 40);
	int margin = space/2;
	int bx0 = cx + margin;
	int bx1 = width - margin - btnFuel.width;
	btnIngredient.x = btnFuel.x = bx0;
	descWidth = (float)Mth::Min(90, width - bx1 + 24 - 4);

	int by = 36;
	btnIngredient.y = by;
	btnFuel.y = by + 20 + btnFuel.height;

	btnBottle0.x = btnBottle1.x = btnBottle2.x = bx1;
	btnBottle0.y = by;
	btnBottle1.y = by + 20 + btnBottle0.height;
	btnBottle2.y = btnBottle1.y + 20 + btnBottle1.height;

	guiBackground->setSize((float)width, (float)height);
	guiSlotMarker->setSize((float)btnFuel.width + 4, (float)btnFuel.height + 4);

	recheckRecipes();
	setupInventoryPane();
	}
}

void BrewingScreen::tick() {
	if (inventoryPane)
		inventoryPane->tick();

	if (doRecreatePane) {
		recheckRecipes();
		setupInventoryPane();
		doRecreatePane = false;
	}
}

void BrewingScreen::handleRenderPane(Touch::InventoryPane* pane, Tesselator& t, int xm, int ym, float a) {
	if (pane) {
		int ms, id;
		pane->markerIndex = -1;
		if (pane->queryHoldTime(&id, &ms)) {
			heldMs = ms;

			const ItemInstance* item = inventoryItems[id];
			int count = (item && !item->isNull())? item->count : 0;
			float maxHoldMs = item? 700 + 10 * item->count: MaxHoldMs;

			if (count > 1 && canMoveToBrewing(id, item)) {
				float share = (heldMs-MinChargeMs) / maxHoldMs;
				pane->markerType = 1;
				pane->markerIndex = id;
				pane->markerShare = Mth::Max(share, 0.0f);

				percent = (int)Mth::clamp(100.0f * share, 0.0f, 100.0f);
				if (percent >= 100) {
					addItem(pane, id);
				}
			}
		}

		pane->render(xm, ym, a);
		guiPaneFrame->draw(t, (float)(pane->rect.x - 1), (float)(pane->rect.y - 1));
	}
}

void BrewingScreen::render(int xm, int ym, float a) {
	const int N = 5;
	static StopwatchNLast r(N);

	updateResult();

	Tesselator& t = Tesselator::instance;
	guiBackground->draw(t, 0, 0);
	glEnable2(GL_ALPHA_TEST);

	// Buttons (Left side + crafting)
	super::render(xm, ym, a);

	handleRenderPane(inventoryPane, t, xm, ym, a);

	t.colorABGR(0xffffffff);

	drawSlotItemAt(t, stand->getItem(btnIngredient.id), btnIngredient.x, btnIngredient.y, btnIngredient.id == selectedSlot);
	drawSlotItemAt(t, stand->getItem(btnFuel.id), btnFuel.x, btnFuel.y, btnFuel.id == selectedSlot);
	drawSlotItemAt(t, stand->getItem(btnBottle0.id), btnBottle0.x, btnBottle0.y, btnBottle0.id == selectedSlot);
	drawSlotItemAt(t, stand->getItem(btnBottle1.id), btnBottle1.x, btnBottle1.y, btnBottle1.id == selectedSlot);
	drawSlotItemAt(t, stand->getItem(btnBottle2.id), btnBottle2.x, btnBottle2.y, btnBottle2.id == selectedSlot);

	// Preview: ghost icon of what the first brewable bottle becomes,
	// drawn over that bottle slot, plus its name below the bottles.
	if (!brewResult.isNull()) {
		const ItemInstance* previewSlot = NULL;
		int previewX = btnBottle0.x, previewY = btnBottle0.y;
		for (int slot = BrewingStandTileEntity::SLOT_BOTTLE_0; slot <= BrewingStandTileEntity::SLOT_BOTTLE_2; ++slot) {
			const ItemInstance* bottle = stand->getItem(slot);
			if (bottle && !bottle->isNull() && bottle->id == Item::potion->id) {
				previewSlot = bottle;
				if (slot == BrewingStandTileEntity::SLOT_BOTTLE_1) { previewX = btnBottle1.x; previewY = btnBottle1.y; }
				if (slot == BrewingStandTileEntity::SLOT_BOTTLE_2) { previewX = btnBottle2.x; previewY = btnBottle2.y; }
				break;
			}
		}
		if (previewSlot != NULL) {
			glEnable2(GL_BLEND);
			t.beginOverride();
			t.colorABGR(0x33ffffff);
			t.noColor();
			ItemRenderer::renderGuiItem(minecraft->font, minecraft->textures, &brewResult, (float)(previewX + 7), (float)(previewY + 8), true);
			t.endOverrideAndDraw();
			glDisable2(GL_BLEND);
		}
		minecraft->font->drawWordWrap(currentItemDesc, (float)(btnBottle2.x - 24), (float)(btnBottle2.y + btnBottle2.height + 6), descWidth, rgbActive);
	}

	minecraft->textures->loadAndBindTexture("gui/spritesheet.png");
	// Fuel flame (same sprites as the furnace flame) next to the fuel
	// slot: height = remaining fuel charges out of 20.
	int yy = btnFuel.y + 8;
	int fpx = stand->getFuelProgress(14) + 2;
	int xx0 = btnIngredient.x + 8;
	blit(xx0, yy, 80, 40, 16, 16, 32, 32);
	blit(xx0, yy + 16 - fpx, 112, 40 + 32-fpx-fpx, 16, fpx, 32, fpx+fpx);

	// Brew-progress arrow between the columns, at the bottles' middle
	// height (same sprites as the furnace arrow).
	int bpx = stand->getBrewProgress(24);
	int xx1 = btnIngredient.x + 40;
	int ayy = btnBottle1.y + 8;
	blit(xx1, ayy, 144, 40, 24, 16, 48, 32);
	blit(xx1, ayy, 144, 72, bpx, 16, bpx+bpx, 32);
}

void BrewingScreen::buttonClicked(Button* button) {
	int slot = button->id;

	if (button == &btnClose) {
		minecraft->player->closeContainer();
	}

	if (slot >= BrewingStandTileEntity::SLOT_BOTTLE_0
	 && slot <= BrewingStandTileEntity::SLOT_FUEL) {
		// Every slot can be selected; tapping the selected slot again
		// takes its items back into the inventory.
		int oldSlot = selectedSlot;
		selectedSlot = slot;

		if (oldSlot == selectedSlot)
			takeAndClearSlot(slot);
	}
}

void BrewingScreen::recheckRecipes()
{
	clearItems();

	Stopwatch w;
	w.start();

	// Which inventory items belong in this screen's pane: potions
	// (bottle slots), brewing ingredients, blaze powder (fuel).
	if (minecraft->player && minecraft->player->inventory) {
		Inventory* inv = (minecraft->player)->inventory;

		for (int i = Inventory::MAX_SELECTION_SIZE; i < inv->getContainerSize(); ++i) {
			if (ItemInstance* item = inv->getItem(i)) {
				if (item->id == Item::potion->id) {
					CItem* ci = new CItem(*item, NULL, "");
					listBottles.push_back(ci);
				}
				if (BrewingRecipes::isFuel(item->id)) {
					CItem* ci = new CItem(*item, NULL, "");
					listFuel.push_back(ci);
				}
				if (BrewingRecipes::isIngredient(item->id)) {
					CItem* ci = new CItem(*item, NULL, "");
					listIngredient.push_back(ci);
				}
			}
		}
	}

	w.stop();
	w.printEvery(1, "> craft ");

	updateItems();
}

bool BrewingScreen::addItem(const Touch::InventoryPane* forPane, int itemIndex) {
	const ItemInstance* item = inventoryItems[itemIndex];
	if (!item || item->isNull()) return false;

	setIfNotSet(doRecreatePane, handleAddItem(selectedSlot, item));

	if (doRecreatePane) {
		int slot = inventorySlots[itemIndex];
		if (player->inventory->getItem(slot)) {
			// fix: if we deplete a slot we didn't click on (but had same type),
			// we need to NULLify the slot that actually was depleted
			for (unsigned int i = 0; i < inventorySlots.size(); ++i) {
				slot = inventorySlots[i];
				if (!player->inventory->getItem(slot)) {
					LOGI("Changed! removing slot %d (was: %d)\n", i, itemIndex);
					itemIndex = i;
					break;
				}
			}
		}
		inventoryItems[itemIndex] = NULL;
	}
	return true;
}

bool BrewingScreen::isAllowed( int slot )
{
	if (slot >= (int)inventoryItems.size()) return false;
    if (!inventoryItems[slot]) return false;
	const ItemInstance& item = *inventoryItems[slot];

	if (selectedSlot == btnFuel.id)
		return BrewingRecipes::isFuel(item.id);
	if (selectedSlot == btnIngredient.id)
		return BrewingRecipes::isIngredient(item.id);
	// Bottle slots take potions (any aux — water, awkward, brewed).
	return item.id == Item::potion->id;
}

bool BrewingScreen::renderGameBehind()
{
	return false;
}

std::vector<const ItemInstance*> BrewingScreen::getItems( const Touch::InventoryPane* forPane )
{
	return inventoryItems;
}

void BrewingScreen::clearItems()
{
	for (unsigned int i = 0; i < listFuel.size(); ++i) delete listFuel[i];
	for (unsigned int i = 0; i < listIngredient.size(); ++i) delete listIngredient[i];
	for (unsigned int i = 0; i < listBottles.size(); ++i) delete listBottles[i];
	listFuel.clear();
	listIngredient.clear();
	listBottles.clear();
}

void BrewingScreen::updateItems() {
	inventoryItems.clear();
	inventorySlots.clear();

	for (int i = Inventory::MAX_SELECTION_SIZE; i < minecraft->player->inventory->getContainerSize(); ++i) {
		ItemInstance* item = minecraft->player->inventory->getItem(i);
		if (!item) continue;
		bool added = false;
		for (unsigned int j = 0; j < listBottles.size(); ++j) {
			if (ItemInstance::matches(item, &listBottles[j]->item)) {
				inventorySlots.push_back(i);
				inventoryItems.push_back(item);
				added = true;
				break;
			}
		}
		if (added) continue;
		for (unsigned int j = 0; j < listFuel.size(); ++j) {
			if (ItemInstance::matches(item, &listFuel[j]->item)) {
				inventorySlots.push_back(i);
				inventoryItems.push_back(item);
				added = true;
				break;
			}
		}
		if (added) continue;
		for (unsigned int j = 0; j < listIngredient.size(); ++j) {
			if (ItemInstance::matches(item, &listIngredient[j]->item)) {
				inventorySlots.push_back(i);
				inventoryItems.push_back(item);
				added = true;
				break;
			}
		}
	}
}

bool BrewingScreen::canMoveToBrewing(int inventorySlot, const ItemInstance* item) {
	if (!isAllowed(inventorySlot)) return false;
	ItemInstance* jitem = stand->getItem(selectedSlot);
	if (!jitem || jitem->isNull()) return true;
	if (ItemInstance::isStackable(item, jitem) && jitem->count < jitem->getMaxStackSize())
		return true;

	return false;
}

void BrewingScreen::updateResult() {
	// Preview = what the first brewable bottle becomes with the
	// current ingredient (or nothing, if nothing can brew). Keyed on
	// the ingredient id + first bottle aux so it only recomputes on
	// change, like the furnace's updateResult.
	const ItemInstance* ingredient = stand->getItem(BrewingStandTileEntity::SLOT_INGREDIENT);
	int key = 0;
	int resultAux = -1;
	if (ingredient && !ingredient->isNull()) {
		for (int slot = BrewingStandTileEntity::SLOT_BOTTLE_0; slot <= BrewingStandTileEntity::SLOT_BOTTLE_2; ++slot) {
			const ItemInstance* bottle = stand->getItem(slot);
			if (bottle && !bottle->isNull() && bottle->id == Item::potion->id) {
				resultAux = BrewingRecipes::getResult(bottle->getAuxValue(), ingredient->id);
				if (resultAux != -1) {
					key = ingredient->id * 100 + bottle->getAuxValue();
					break;
				}
			}
		}
	}
	if (key == lastBrewTypeId) return;
	lastBrewTypeId = key;
	if (resultAux != -1) {
		brewResult = ItemInstance(Item::potion, 1, resultAux);
		currentItemDesc = brewResult.getName();
	} else {
		brewResult = ItemInstance();
		currentItemDesc = "";
	}
}

void BrewingScreen::setupInventoryPane()
{
	if (inventoryPane) delete inventoryPane;
	inventoryPane = new Touch::InventoryPane(this, minecraft, inventoryPaneRect, inventoryPaneRect.w, BorderPixels, inventoryItems.size(), ItemSize, (int)BorderPixels);
	inventoryPane->fillMarginX = 0;
	inventoryPane->fillMarginY = 0;
	guiPaneFrame->setSize((float)inventoryPaneRect.w + 2, (float)inventoryPaneRect.h + 2);
}

void BrewingScreen::drawSlotItemAt( Tesselator& t, const ItemInstance* item, int x, int y, bool selected)
{
	float xx = (float)x;
	float yy = (float)y;

	(selected? guiSlot : guiSlot)->draw(t, xx, yy);

	if (selected)
		guiSlotMarker->draw(t, xx - 2, yy - 2);

	if (item && !item->isNull()) {
		ItemRenderer::renderGuiItem(minecraft->font, minecraft->textures, item, xx + 7, yy + 8, true);
		minecraft->gui.renderSlotText(item, xx + 3, yy + 3, true, true);
	}
}

ItemInstance BrewingScreen::moveOver(const ItemInstance* item, int maxCount) {
	int wantedCount = item->count * percent / 100;
	if (!wantedCount || heldMs < MinChargeMs)
		wantedCount = 1;

	wantedCount = Mth::Min(wantedCount, maxCount);

	ItemInstance removed(item->id, wantedCount, item->getAuxValue());
	int oldSize = minecraft->player->inventory->getNumEmptySlots();
	if (minecraft->player->inventory->removeResource(removed)) {
		int newSize = minecraft->player->inventory->getNumEmptySlots();
		setIfNotSet(doRecreatePane, newSize != oldSize);
		return removed;
	}
	return ItemInstance();
}

void BrewingScreen::takeAndClearSlot( int slot )
{
	ItemInstance oldItem = *stand->getItem(slot);
	ItemInstance blank;

	stand->setItem(slot, &blank);
	if (minecraft->level->isClientSide) {
		ContainerSetSlotPacket p(menu->containerId, slot, blank);
		minecraft->raknetInstance->send(p);
	}

	int oldSize = minecraft->player->inventory->getNumEmptySlots();

	if (!minecraft->player->inventory->add(&oldItem))
		minecraft->player->drop(new ItemInstance(oldItem), false);

	int newSize = minecraft->player->inventory->getNumEmptySlots();
	setIfNotSet(doRecreatePane, newSize != oldSize);
}

bool BrewingScreen::handleAddItem( int slot, const ItemInstance* item )
{
	ItemInstance* standItem = stand->getItem(slot);
	int oldSize = minecraft->player->inventory->getNumEmptySlots();

	if (item->id == standItem->id && item->getAuxValue() == standItem->getAuxValue()) {
		// If stackable, stack them! Else deny the addition. (Potions
		// stack to 1, so a different potion in the slot is refused
		// here; the empty-slot branch below handles the rest.)
		const int maxMovedCount = standItem->getMaxStackSize() - standItem->count;
		if (maxMovedCount <= 0)
			return false;

		ItemInstance added = moveOver(item, maxMovedCount);
		standItem->count += added.count;
	} else {
		if (!stand->isSlotEmpty(slot))
			return false;

		ItemInstance moved = moveOver(item, item->getMaxStackSize());
		player->containerMenu->setSlot(slot, &moved);
	}

	if (minecraft->level->isClientSide) {
		ContainerSetSlotPacket p(menu->containerId, slot, *standItem);
		minecraft->raknetInstance->send(p);
	}

	int newSize = minecraft->player->inventory->getNumEmptySlots();
	return (newSize != oldSize);
}
