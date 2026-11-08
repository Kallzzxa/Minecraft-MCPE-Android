#include "BrewingRecipes.h"
#include "../PotionItem.h"
#include "../Item.h"

// v20: brewing recipe table. Ingredient ids are Item::...->id (full
// ids); potion sides are PotionItem aux values.
//
// v26: redstone (extend) + glowstone (level II) upgrades. Both work
// on drinkable AND splash potions (the variant flag is OR'd onto the
// input aux, so the splash bit survives), and both are mutually
// exclusive — a potion carrying either flag refuses the other, like
// modern MC.

int BrewingRecipes::getResult(int inputAux, int ingredientId) {
	// v25: gunpowder turns ANY potion into its splash variant
	// (modern MC) — water and awkward included (splash water
	// bottle / splash awkward potion are real). A potion that is
	// already splash cannot be splashed again. v26: this also
	// splashes extended / level II potions — the gunpowder branch
	// runs first and only checks the splash flag.
	if (ingredientId == Item::sulphur->id) {
		if (PotionItem::isSplash(inputAux)) return -1;
		// v26: OR the flag on instead of rebuilding from the base
		// type, so an extended / level II potion keeps its variant
		// when splashed (baseAux() + SPLASH_FLAG would have
		// stripped it).
		return inputAux | PotionItem::SPLASH_FLAG;
	}
	// v26: redstone extends DURATION (~2x, modern MC table in
	// PotionItem.h). Only timed-effect potions: instant healing /
	// harming have no duration to extend, water / awkward have no
	// effect at all. An already-extended or level II potion
	// refuses redstone (mutual exclusivity).
	if (ingredientId == Item::redStone->id) {
		if (PotionItem::isExtended(inputAux) || PotionItem::isLevel2(inputAux)) return -1;
		switch (PotionItem::baseAux(inputAux)) {
			case PotionItem::SWIFTNESS:
			case PotionItem::STRENGTH:
			case PotionItem::FIRE_RESISTANCE:
			case PotionItem::REGENERATION:
			case PotionItem::POISON:
			case PotionItem::WEAKNESS:
			case PotionItem::SLOWNESS:
				return inputAux | PotionItem::EXTENDED_FLAG;
			default:
				return -1;
		}
	}
	// v26: glowstone upgrades to LEVEL II (stronger effect, shorter
	// duration, modern MC). Only effects that HAVE a level II in
	// modern MC: healing / harming (instant, +8 HP / 12 damage),
	// swiftness, strength, regeneration, poison. Fire resistance
	// is binary (no level II), weakness / slowness have no brewable
	// level II in modern MC, water / awkward have no effect.
	// An already-extended or level II potion refuses glowstone.
	if (ingredientId == Item::yellowDust->id) {
		if (PotionItem::isExtended(inputAux) || PotionItem::isLevel2(inputAux)) return -1;
		switch (PotionItem::baseAux(inputAux)) {
			case PotionItem::HEALING:
			case PotionItem::HARMING:
			case PotionItem::SWIFTNESS:
			case PotionItem::STRENGTH:
			case PotionItem::REGENERATION:
			case PotionItem::POISON:
				return inputAux | PotionItem::LEVEL2_FLAG;
			default:
				return -1;
		}
	}
	// Splash potions cannot be brewed further with any other
	// ingredient (v26: redstone / glowstone above are the only
	// exceptions — they run before this check).
	if (PotionItem::isSplash(inputAux)) return -1;
	if (ingredientId == Item::netherWart->id) {
		// Base of every brew: water bottle -> awkward potion.
		if (inputAux == PotionItem::WATER) return PotionItem::AWKWARD;
		return -1;
	}
	if (inputAux != PotionItem::AWKWARD) {
		// Secondary ingredients only work on awkward potions, except
		// fermented spider eye, which corrupts finished potions.
		// v26: corruption does NOT apply to upgraded potions (an
		// extended / level II potion keeps its variant — modern MC
		// would carry it over, but this tree has no corrupt table
		// for variants, so it refuses rather than silently
		// downgrading the potion).
		if (PotionItem::isExtended(inputAux) || PotionItem::isLevel2(inputAux)) return -1;
		if (ingredientId == Item::fermentedSpiderEye->id) {
			if (inputAux == PotionItem::HEALING) return PotionItem::HARMING;
			if (inputAux == PotionItem::POISON) return PotionItem::HARMING;
			if (inputAux == PotionItem::SWIFTNESS) return PotionItem::SLOWNESS;
		}
		return -1;
	}
	if (ingredientId == Item::glisteringMelon->id) return PotionItem::HEALING;
	if (ingredientId == Item::sugar->id) return PotionItem::SWIFTNESS;
	if (ingredientId == Item::blazePowder->id) return PotionItem::STRENGTH;
	if (ingredientId == Item::magmaCream->id) return PotionItem::FIRE_RESISTANCE;
	if (ingredientId == Item::ghastTear->id) return PotionItem::REGENERATION;
	if (ingredientId == Item::spiderEye->id) return PotionItem::POISON;
	if (ingredientId == Item::fermentedSpiderEye->id) return PotionItem::WEAKNESS;
	return -1;
}

bool BrewingRecipes::isIngredient(int itemId) {
	return itemId == Item::sulphur->id // v25: gunpowder -> splash
		|| itemId == Item::redStone->id // v26: redstone -> extended
		|| itemId == Item::yellowDust->id // v26: glowstone dust -> level II
		|| itemId == Item::netherWart->id
		|| itemId == Item::glisteringMelon->id
		|| itemId == Item::sugar->id
		|| itemId == Item::blazePowder->id
		|| itemId == Item::magmaCream->id
		|| itemId == Item::ghastTear->id
		|| itemId == Item::spiderEye->id
		|| itemId == Item::fermentedSpiderEye->id;
}

bool BrewingRecipes::isFuel(int itemId) {
	return itemId == Item::blazePowder->id;
}
