#ifndef NET_MINECRAFT_WORLD_ITEM__Enchantment_H__
#define NET_MINECRAFT_WORLD_ITEM__Enchantment_H__

// v10: Enchanting. One enchantment per item (this 0.6.1 tree has no
// enchantment NBT list); the enchant id + level live on the ItemInstance
// itself (see ItemInstance::enchantmentId / enchantmentLevel) and are
// saved in the item tag ("Ench" / "EnchLvl").
//
// Enchantments that actually do something in this build:
//   Sharpness   — +1 attack damage per level (ItemInstance::getAttackDamage)
//   Efficiency  — +50% dig speed per level (DiggerItem::getDestroySpeed)
//   Protection  — -1 damage taken per total level, at most half the hit
//                 (Player::getDamageAfterArmorAbsorb)
//   Unbreaking  — level/(level+1) chance to take no durability damage
//                 (ItemInstance::hurt)
// No lapis cost, no anvil, no enchanted books in this batch — the only
// cost is whole XP levels (Player::spendExperienceLevels).

class ItemInstance;

namespace Enchantment
{
    const int NONE       = 0;
    const int SHARPNESS  = 1;
    const int EFFICIENCY = 2;
    const int PROTECTION = 3;
    const int UNBREAKING = 4;

    int getMaxLevel(int enchantmentId);
    const char* getName(int enchantmentId);
    const char* getRomanNumeral(int level);

    // Can this enchantment go on this item at all?
    //   Sharpness: swords only. Efficiency: pickaxe / shovel / axe
    //   (DiggerItem tools; hoes are plain Items in this tree, so no).
    //   Protection: armor. Unbreaking: any damageable item.
    bool canApplyTo(int enchantmentId, const ItemInstance* item);
    bool isEnchantable(const ItemInstance* item);
}

#endif /*NET_MINECRAFT_WORLD_ITEM__Enchantment_H__*/
