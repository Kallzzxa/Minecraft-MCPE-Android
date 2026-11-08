#include "EnchantScreen.h"
#include "../components/Button.h"
#include "../../Minecraft.h"
#include "../../player/LocalPlayer.h"
#include "../../../world/entity/player/Player.h"
#include "../../../world/item/ItemInstance.h"
#include "../../../world/item/Enchantment.h"
#include "../../../util/Mth.h"
#include <stdio.h>

EnchantScreen::EnchantScreen(Player* player)
:   player(player),
    bClose(NULL),
    status("")
{
    bOptions[0] = bOptions[1] = bOptions[2] = NULL;
}

EnchantScreen::~EnchantScreen()
{
    delete bOptions[0];
    delete bOptions[1];
    delete bOptions[2];
    delete bClose;
}

ItemInstance* EnchantScreen::getHeldItem()
{
    if (player == NULL) return NULL;
    return player->getSelectedItem();
}

void EnchantScreen::generateOffers()
{
    offers.clear();

    ItemInstance* item = getHeldItem();
    if (item == NULL || item->isNull()) return;
    if (!Enchantment::isEnchantable(item)) return;

    // Shuffle the 4 enchantments, then take the ones that fit this item.
    int candidates[4] = {
        Enchantment::SHARPNESS,
        Enchantment::EFFICIENCY,
        Enchantment::PROTECTION,
        Enchantment::UNBREAKING
    };
    for (int i = 3; i > 0; --i) {
        int j = (int) (Mth::random() * (i + 1));
        int tmp = candidates[i];
        candidates[i] = candidates[j];
        candidates[j] = tmp;
    }

    for (int i = 0; i < 4 && (int) offers.size() < 3; ++i) {
        int ench = candidates[i];
        if (!Enchantment::canApplyTo(ench, item)) continue;
        int maxLevel = Enchantment::getMaxLevel(ench);
        // Random level 1..3, capped by the enchantment's max.
        int level = 1 + (int) (Mth::random() * 3);
        if (level > maxLevel) level = maxLevel;
        Offer offer;
        offer.enchantmentId = ench;
        offer.level = level;
        offer.costLevels = level; // cost in whole XP levels: 1, 2 or 3.
        char buf[64];
        snprintf(buf, sizeof(buf), "%s %s  -  %d level%s",
            Enchantment::getName(ench),
            Enchantment::getRomanNumeral(level),
            level, (level == 1) ? "" : "s");
        offer.label = buf;
        offers.push_back(offer);
    }
}

void EnchantScreen::init()
{
    generateOffers();

    for (int i = 0; i < 3; ++i) {
        std::string label = (i < (int) offers.size()) ? offers[i].label : "---";
        if (minecraft->useTouchscreen())
            bOptions[i] = new Touch::TButton(1 + i, label);
        else
            bOptions[i] = new Button(1 + i, label);
        buttons.push_back(bOptions[i]);
        tabButtons.push_back(bOptions[i]);
    }

    if (minecraft->useTouchscreen())
        bClose = new Touch::TButton(4, "Close");
    else
        bClose = new Button(4, "Close");
    buttons.push_back(bClose);
    tabButtons.push_back(bClose);
}

void EnchantScreen::setupPositions()
{
    int w = width / 2;
    int x = width / 2 - w / 2;
    int y = height / 3 + 10;
    for (int i = 0; i < 3; ++i) {
        bOptions[i]->width = w;
        bOptions[i]->x = x;
        bOptions[i]->y = y + i * 22;
    }
    bClose->width = w / 2;
    bClose->x = width / 2 - bClose->width / 2;
    bClose->y = y + 3 * 22 + 8;
}

void EnchantScreen::render(int xm, int ym, float a)
{
    fillGradient(0, 0, width, height, 0xc0101010, 0xd0202020);

    glPushMatrix2();
    glScalef2(2, 2, 2);
    drawCenteredString(font, "Enchanting", width / 2 / 2, 8, 0xffffff);
    glPopMatrix2();

    int y = height / 4 - 6;
    ItemInstance* item = getHeldItem();
    if (item == NULL || item->isNull()) {
        drawCenteredString(font, "Hold a sword, tool or armor piece", width / 2, y, 0xffffaa);
        drawCenteredString(font, "in your hand, then use the table.", width / 2, y + 10, 0xffffaa);
    } else if (!Enchantment::isEnchantable(item)) {
        drawCenteredString(font, "This item cannot be enchanted.", width / 2, y, 0xffffaa);
        drawCenteredString(font, item->getName().c_str(), width / 2, y + 10, 0xffffff);
    } else if (item->isEnchanted()) {
        drawCenteredString(font, "Already enchanted:", width / 2, y, 0xaaaaff);
        drawCenteredString(font, item->getName().c_str(), width / 2, y + 10, 0xffffff);
    } else {
        drawCenteredString(font, item->getName().c_str(), width / 2, y, 0xffffff);
    }

    char levelBuf[32];
    snprintf(levelBuf, sizeof(levelBuf), "Your level: %d",
        player ? player->getExperienceLevel() : 0);
    drawCenteredString(font, levelBuf, width / 2, height / 3 - 12, 0x7dff7d);

    if (!status.empty())
        drawCenteredString(font, status.c_str(), width / 2, height - 24, 0xffffaa);

    Screen::render(xm, ym, a);
}

bool EnchantScreen::renderGameBehind()
{
    return false;
}

void EnchantScreen::buttonClicked(Button* button)
{
    if (button == bClose) {
        minecraft->setScreen(NULL);
        return;
    }

    int idx = button->id - 1;
    if (idx < 0 || idx >= (int) offers.size()) {
        status = "No enchantment there.";
        return;
    }

    ItemInstance* item = getHeldItem();
    if (item == NULL || item->isNull()) {
        status = "Hold an item in your hand first.";
        return;
    }
    if (item->isEnchanted()) {
        status = "Already enchanted.";
        return;
    }

    const Offer& offer = offers[idx];
    if (player->getExperienceLevel() < offer.costLevels) {
        char buf[64];
        snprintf(buf, sizeof(buf), "Not enough levels! Need %d.", offer.costLevels);
        status = buf;
        return;
    }

    if (!player->spendExperienceLevels(offer.costLevels)) {
        status = "Not enough levels!";
        return;
    }
    item->setEnchantment(offer.enchantmentId, offer.level);

    char buf[96];
    snprintf(buf, sizeof(buf), "Enchanted: %s %s!",
        Enchantment::getName(offer.enchantmentId),
        Enchantment::getRomanNumeral(offer.level));
    status = buf;
}
