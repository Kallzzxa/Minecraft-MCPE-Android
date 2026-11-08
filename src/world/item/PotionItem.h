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
//
// v25: SPLASH potions. A splash potion is the same item with the
// SPLASH_FLAG bit (16) set on the aux value: aux = base type + 16,
// so a splash potion of healing is aux 18, a splash water bottle is
// aux 16. The flag survives save/load untouched (ItemInstance saves
// aux as a short "Damage" tag) and the brewing stand turns any potion
// into its splash variant with gunpowder (see BrewingRecipes). A
// splash potion is NOT drunk: use() throws it as a ThrownPotion
// (EggItem pattern — tap to throw, the (Level, Mob) ctor already
// shoots along the player's aim), and the splash flag is stripped
// before the effect is applied (ThrownPotion normalises the aux).
// Splash icons live on gui/items.png row 14 (cols 0..10), tinted
// copies of the row-12 drinkable icons so the two are told apart in
// the hotbar; names are item.splashPotion.<desc>.name in en_US.lang.

#include "Item.h"
#include "ItemInstance.h"
#include "../entity/player/Player.h"
#include "../entity/projectile/ThrownPotion.h"
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

	// v25: splash flag — OR'd onto the aux value (see the header
	// comment). baseAux() strips it, isSplash() tests it.
	static const int SPLASH_FLAG = 16;
	// Splash icons: gui/items.png row 14, cols 0..10 (one per base
	// type, same column as the drinkable icon on row 12).
	static const int SPLASH_ICON_BASE = 14 * 16;

	static bool isSplash(int auxValue) { return (auxValue & SPLASH_FLAG) != 0; }
	static int baseAux(int auxValue) { return auxValue & (SPLASH_FLAG - 1); }

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
		// v25: splash potions use the tinted row-14 copies instead,
		// so a splash bottle is told apart from a drinkable one in
		// the hotbar. The flag bit is masked off either way.
		int base = baseAux(auxValue);
		if (isSplash(auxValue))
			return SPLASH_ICON_BASE + Mth::clamp(base, 0, NUM_TYPES - 1);
		return icon + Mth::clamp(base, 0, NUM_TYPES - 1);
	}

	std::string getDescriptionId(const ItemInstance* itemInstance) const {
		// v25: splash potions get their own lang keys
		// (item.splashPotion.<desc>.name) instead of the drinkable
		// ones, so the name itself says "Splash Potion of ...".
		int aux = itemInstance->getAuxValue();
		if (isSplash(aux))
			return std::string("item.splashPotion.") + getTypeDesc(baseAux(aux));
		return super::getDescriptionId() + "." + getTypeDesc(baseAux(aux));
	}

	int getUseDuration(ItemInstance* itemInstance) {
		return DRINK_DURATION;
	}

	UseAnim::UseAnimation getUseAnimation() {
		return UseAnim::drink;
	}

	ItemInstance* use(ItemInstance* instance, Level* level, Player* player) {
		// v25: a splash potion is THROWN, not drunk — EggItem
		// pattern (tap to throw; the Throwable (Level, Mob) ctor
		// shoots along the player's aim). The bottle is consumed
		// (unless creative) and the thrower CAN be splashed by its
		// own potion (ThrownPotion splashOwner=true, modern MC).
		if (isSplash(instance->getAuxValue())) {
			if (!player->abilities.instabuild)
				instance->count--;
			level->playSound(player, "random.bow", 0.5f, 0.4f / (random.nextFloat() * 0.4f + 0.8f));
			if (!level->isClientSide)
				level->addEntity(new ThrownPotion(level, player, instance->getAuxValue(), true));
			return instance;
		}
		// Drinkable any time (no hunger gate like FoodItem).
		if (!player->abilities.invulnerable)
			player->startUsingItem(*instance, getUseDuration(instance));
		return instance;
	}

	ItemInstance useTimeDepleted(ItemInstance* instance, Level* level, Player* player) {
		// Only reachable for drinkable potions (use() throws splash
		// ones instead of starting the drink), but mask the flag
		// anyway so a splash aux can never drink a wrong effect.
		int aux = baseAux(instance->getAuxValue());
		applyEffect(aux, player);
		level->playSound(player, "random.drink", 0.5f, level->random.nextFloat() * 0.1f + 0.9f);
		// The drunk bottle becomes an empty glass bottle.
		*instance = ItemInstance(Item::glassBottle);
		return *instance;
	}

	static const char* getTypeDesc(int auxValue) {
		// v25: mask the splash flag off first, so a splash aux
		// names its base potion type instead of clamping to 10.
		switch (Mth::clamp(baseAux(auxValue), 0, NUM_TYPES - 1)) {
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

	static void applyEffect(int aux, Mob* mob) {
		// v25: mask the splash flag off — a splash potion applies
		// its BASE effect (a splash healing potion heals, etc.).
		aux = baseAux(aux);
		switch (aux) {
			case HEALING: // instant health I: +4 HP
				mob->heal(4);
				break;
			case HARMING: // instant damage I: 6 HP
				mob->hurt(NULL, 6);
				break;
			case SWIFTNESS:
				mob->speedEffectTicks = DURATION_LONG;
				break;
			case STRENGTH:
				mob->strengthEffectTicks = DURATION_LONG;
				break;
			case FIRE_RESISTANCE:
				mob->fireResistanceEffectTicks = DURATION_LONG;
				break;
			case REGENERATION:
				mob->regenerationEffectTicks = DURATION_SHORT;
				break;
			case POISON:
				mob->poisonEffectTicks = DURATION_SHORT;
				break;
			case WEAKNESS:
				mob->weaknessEffectTicks = DURATION_MEDIUM;
				break;
			case SLOWNESS:
				mob->slownessEffectTicks = DURATION_MEDIUM;
				break;
			default:
				break; // water / awkward: no effect
		}
	}
};

#endif /*NET_MINECRAFT_WORLD_ITEM__PotionItem_H__*/
