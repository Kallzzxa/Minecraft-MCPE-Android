#include "Enchantment.h"
#include "Item.h"
#include "ItemInstance.h"

namespace Enchantment
{

int getMaxLevel(int enchantmentId)
{
    switch (enchantmentId)
    {
    case SHARPNESS:
    case EFFICIENCY:
    case PROTECTION:
    case UNBREAKING:
        return 3;
    default:
        return 0;
    }
}

const char* getName(int enchantmentId)
{
    switch (enchantmentId)
    {
    case SHARPNESS:  return "Sharpness";
    case EFFICIENCY: return "Efficiency";
    case PROTECTION: return "Protection";
    case UNBREAKING: return "Unbreaking";
    default:         return "Unknown";
    }
}

const char* getRomanNumeral(int level)
{
    switch (level)
    {
    case 1:  return "I";
    case 2:  return "II";
    case 3:  return "III";
    case 4:  return "IV";
    case 5:  return "V";
    default: return "?";
    }
}

static bool isSword(const Item* item)
{
    return item == Item::sword_wood
        || item == Item::sword_stone
        || item == Item::sword_iron
        || item == Item::sword_emerald
        || item == Item::sword_gold;
}

static bool isDiggerTool(const Item* item)
{
    return item == Item::pickAxe_wood
        || item == Item::pickAxe_stone
        || item == Item::pickAxe_iron
        || item == Item::pickAxe_emerald
        || item == Item::pickAxe_gold
        || item == Item::shovel_wood
        || item == Item::shovel_stone
        || item == Item::shovel_iron
        || item == Item::shovel_emerald
        || item == Item::shovel_gold
        || item == Item::hatchet_wood
        || item == Item::hatchet_stone
        || item == Item::hatchet_iron
        || item == Item::hatchet_emerald
        || item == Item::hatchet_gold;
}

bool canApplyTo(int enchantmentId, const ItemInstance* item)
{
    if (item == NULL || item->isNull())
        return false;

    const Item* base = item->getItem();
    if (base == NULL)
        return false;

    switch (enchantmentId)
    {
    case SHARPNESS:
        return isSword(base);
    case EFFICIENCY:
        return isDiggerTool(base);
    case PROTECTION:
        return ItemInstance::isArmorItem(item);
    case UNBREAKING:
        return item->isDamageableItem();
    default:
        return false;
    }
}

bool isEnchantable(const ItemInstance* item)
{
    return canApplyTo(SHARPNESS, item)
        || canApplyTo(EFFICIENCY, item)
        || canApplyTo(PROTECTION, item)
        || canApplyTo(UNBREAKING, item);
}

} // namespace Enchantment
