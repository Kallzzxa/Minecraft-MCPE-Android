#ifndef NET_MINECRAFT_WORLD_ITEM__PotionItem_H__
#define NET_MINECRAFT_WORLD_ITEM__PotionItem_H__

// v20: potions. One item id (ctor 117, full id 373 — modern MC), one
// bottle icon per aux value (gui/items.png row 12, cols 0..10 — see
// getIcon, same pattern as DyePowderItem) and one name per aux (see
// getDescriptionId; lang keys item.potion.<desc>.name in en_US.lang).
// Drinking is the FoodItem flow: hold to drink for 32 ticks
// (UseAnim::drink), then the effect applies and the bottle becomes an
// empty glass bottle (BowlFoodItem pattern). Timed effects live on Mob
// (speedEffectTicks etc.) and are ticked down in Mob::tick.

#include "Item.h"
#include "ItemInstance.h"
#include "../entity/player/Player.h"
#include "../level/Level.h"
#include "../../util/Mth.h"

class PotionItem: public Item
{
	typedef Item super;
public:
	// aux values (potion types)
	static const int WATER = 0;
	static const int AWKWARD = 1;
	static const int HEALING = 2;
	static const int SWIFTNESS = 3;
	static const int STRENGTH = 4;
	static const int FIRE_RESISTANCE = 5;
	static const int REGENERATION = 6;
	static const int POISON = 7;
	static const int WEAKNESS = 8;
	static const int HARMING = 9;
	static const int SLOWNESS = 10;
	static const int NUM_TYPES = 11;

	// Durations in ticks (20 tps). Modern MC: 3:00 for the 3-minute
	// potions, 0:45 for regeneration/poison, 1:30 for weakness/slowness.
	static const int DRINK_DURATION = 32; // 1.6 s, like food
	static const int DURATION_LONG = 3600;
	static const int DURATION_MEDIUM = 1800;
	static const int DURATION_SHORT = 900;

	PotionItem(int id)
	:	super(id)
	{
		setStackedByData(true);
		setMaxDamage(0);
		setMaxStackSize(1);
	}

	int getIcon(int auxValue) {
		// Base icon is the water bottle (row 12, col 0); each aux
		// steps one cell right along the row (DyePowderItem pattern).
		return icon + Mth::clamp(auxValue, 0, NUM_TYPES - 1);
	}

	std::string getDescriptionId(const ItemInstance* itemInstance) const {
		return super::getDescriptionId() + "." + getTypeDesc(itemInstance->getAuxValue());
	}

	int getUseDuration(ItemInstance* itemInstance) {
		return DRINK_DURATION;
	}

	UseAnim::UseAnimation getUseAnimation() {
		return UseAnim::drink;
	}

	ItemInstance* use(ItemInstance* instance, Level* level, Player* player) {
		// Drinkable any time (no hunger gate like FoodItem).
		if (!player->abilities.invulnerable)
			player->startUsingItem(*instance, getUseDuration(instance));
		return instance;
	}

	ItemInstance useTimeDepleted(ItemInstance* instance, Level* level, Player* player) {
		int aux = Mth::clamp(instance->getAuxValue(), 0, NUM_TYPES - 1);
		applyEffect(aux, player);
		level->playSound(player, "random.drink", 0.5f, level->random.nextFloat() * 0.1f + 0.9f);
		// The drunk bottle becomes an empty glass bottle.
		*instance = ItemInstance(Item::glassBottle);
		return *instance;
	}

	static const char* getTypeDesc(int auxValue) {
		switch (Mth::clamp(auxValue, 0, NUM_TYPES - 1)) {
			case WATER: return "emptyPotion";
			case AWKWARD: return "awkward";
			case HEALING: return "heal";
			case SWIFTNESS: return "moveSpeed";
			case STRENGTH: return "damageBoost";
			case FIRE_RESISTANCE: return "fireResistance";
			case REGENERATION: return "regeneration";
			case POISON: return "poison";
			case WEAKNESS: return "weakness";
			case HARMING: return "harm";
			case SLOWNESS: return "moveSlowdown";
			default: return "emptyPotion";
		}
	}

	static void applyEffect(int aux, Player* player) {
		switch (aux) {
			case HEALING: // instant health I: +4 HP
				player->heal(4);
				break;
			case HARMING: // instant damage I: 6 HP
				player->hurt(NULL, 6);
				break;
			case SWIFTNESS:
				player->speedEffectTicks = DURATION_LONG;
				break;
			case STRENGTH:
				player->strengthEffectTicks = DURATION_LONG;
				break;
			case FIRE_RESISTANCE:
				player->fireResistanceEffectTicks = DURATION_LONG;
				break;
			case REGENERATION:
				player->regenerationEffectTicks = DURATION_SHORT;
				break;
			case POISON:
				player->poisonEffectTicks = DURATION_SHORT;
				break;
			case WEAKNESS:
				player->weaknessEffectTicks = DURATION_MEDIUM;
				break;
			case SLOWNESS:
				player->slownessEffectTicks = DURATION_MEDIUM;
				break;
			default:
				break; // water / awkward: no effect
		}
	}
};

#endif /*NET_MINECRAFT_WORLD_ITEM__PotionItem_H__*/
