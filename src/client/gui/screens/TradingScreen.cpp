#include "TradingScreen.h"
#include "../components/Button.h"
#include "../../Minecraft.h"
#include "../../player/LocalPlayer.h"
#include "../../../world/entity/player/Player.h"
#include "../../../world/entity/player/Inventory.h"
#include "../../../world/item/Item.h"
#include "../../../world/item/ItemInstance.h"
#include <stdio.h>

TradingScreen::TradingScreen(Player* player, Villager* villager)
:   player(player),
    villager(villager),
    bClose(NULL),
    status("")
{
    for (int i = 0; i < 6; ++i) bTrades[i] = NULL;

    // Fixed trade list (see header). Labels use the items' real names.
    struct Def { int costId, costCount, resultId, resultCount; };
    const Def defs[6] = {
        { Item::wheat->id,       20, Item::emeraldGem->id,   1 },
        { Item::coal->id,        16, Item::emeraldGem->id,   1 },
        { Item::emeraldGem->id,   1, Item::bread->id,        3 },
        { Item::emeraldGem->id,   1, Item::beef_cooked->id,  2 },
        { Item::emeraldGem->id,   2, Item::pickAxe_iron->id, 1 },
        { Item::emeraldGem->id,   3, Item::sword_emerald->id, 1 },
    };
    for (int i = 0; i < 6; ++i) {
        Trade t;
        t.costId = defs[i].costId;
        t.costCount = defs[i].costCount;
        t.resultId = defs[i].resultId;
        t.resultCount = defs[i].resultCount;
        ItemInstance cost(defs[i].costId, 1, 0);
        ItemInstance result(defs[i].resultId, 1, 0);
        char buf[128];
        snprintf(buf, sizeof(buf), "%d %s  ->  %d %s",
            t.costCount, cost.getName().c_str(),
            t.resultCount, result.getName().c_str());
        t.label = buf;
        trades.push_back(t);
    }
}

TradingScreen::~TradingScreen()
{
    for (int i = 0; i < 6; ++i) delete bTrades[i];
    delete bClose;
}

int TradingScreen::countItem(int itemId)
{
    if (player == NULL || player->inventory == NULL) return 0;
    int total = 0;
    int size = player->inventory->getContainerSize();
    for (int i = 0; i < size; ++i) {
        ItemInstance* item = player->inventory->getItem(i);
        if (item != NULL && !item->isNull() && item->id == itemId)
            total += item->count;
    }
    return total;
}

bool TradingScreen::removeItems(int itemId, int count)
{
    if (player == NULL || player->inventory == NULL) return false;
    int remaining = count;
    int size = player->inventory->getContainerSize();
    for (int i = 0; i < size && remaining > 0; ++i) {
        ItemInstance* item = player->inventory->getItem(i);
        if (item == NULL || item->isNull() || item->id != itemId) continue;
        int take = remaining < item->count ? remaining : item->count;
        // NOTE: Inventory::removeItem(const ItemInstance*) hides the
        // base-class overload, so qualify the FillingContainer one.
        player->inventory->FillingContainer::removeItem(i, take);
        remaining -= take;
    }
    return remaining == 0;
}

bool TradingScreen::giveItem(int itemId, int count)
{
    if (player == NULL || player->inventory == NULL) return false;
    ItemInstance result(itemId, count, 0);
    return player->inventory->add(&result);
}

void TradingScreen::init()
{
    for (int i = 0; i < 6; ++i) {
        if (minecraft->useTouchscreen())
            bTrades[i] = new Touch::TButton(1 + i, trades[i].label);
        else
            bTrades[i] = new Button(1 + i, trades[i].label);
        buttons.push_back(bTrades[i]);
        tabButtons.push_back(bTrades[i]);
    }

    if (minecraft->useTouchscreen())
        bClose = new Touch::TButton(7, "Close");
    else
        bClose = new Button(7, "Close");
    buttons.push_back(bClose);
    tabButtons.push_back(bClose);
}

void TradingScreen::setupPositions()
{
    int w = width / 2;
    int x = width / 2 - w / 2;
    int y = height / 6;
    for (int i = 0; i < 6; ++i) {
        bTrades[i]->width = w;
        bTrades[i]->x = x;
        bTrades[i]->y = y + i * 22;
    }
    bClose->width = w / 2;
    bClose->x = width / 2 - bClose->width / 2;
    bClose->y = y + 6 * 22 + 8;
}

void TradingScreen::render(int xm, int ym, float a)
{
    fillGradient(0, 0, width, height, 0xc0101010, 0xd0202020);

    glPushMatrix2();
    glScalef2(2, 2, 2);
    drawCenteredString(font, "Trading", width / 2 / 2, 8, 0xffffff);
    glPopMatrix2();

    int emeralds = countItem(Item::emeraldGem->id);
    char buf[64];
    snprintf(buf, sizeof(buf), "Your emeralds: %d", emeralds);
    drawCenteredString(font, buf, width / 2, height / 6 - 14, 0x7dff7d);

    if (!status.empty())
        drawCenteredString(font, status.c_str(), width / 2, height - 24, 0xffffaa);

    Screen::render(xm, ym, a);
}

bool TradingScreen::renderGameBehind()
{
    return false;
}

void TradingScreen::buttonClicked(Button* button)
{
    if (button == bClose) {
        minecraft->setScreen(NULL);
        return;
    }

    int idx = button->id - 1;
    if (idx < 0 || idx >= (int) trades.size()) {
        status = "No trade there.";
        return;
    }

    const Trade& trade = trades[idx];
    if (countItem(trade.costId) < trade.costCount) {
        ItemInstance cost(trade.costId, 1, 0);
        char buf[96];
        snprintf(buf, sizeof(buf), "Not enough %s! Need %d.",
            cost.getName().c_str(), trade.costCount);
        status = buf;
        return;
    }

    if (!removeItems(trade.costId, trade.costCount)) {
        status = "Could not take the payment.";
        return;
    }
    if (!giveItem(trade.resultId, trade.resultCount)) {
        // Inventory full: refund the cost, do not eat the payment.
        giveItem(trade.costId, trade.costCount);
        status = "No room in your inventory!";
        return;
    }

    ItemInstance result(trade.resultId, 1, 0);
    char buf[128];
    snprintf(buf, sizeof(buf), "Traded for %d %s!",
        trade.resultCount, result.getName().c_str());
    status = buf;
}
