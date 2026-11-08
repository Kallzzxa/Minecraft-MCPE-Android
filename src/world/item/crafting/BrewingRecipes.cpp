#include "BrewingRecipes.h"
#include "../PotionItem.h"
#include "../Item.h"

// v20: brewing recipe table. Ingredient ids are Item::...->id (full
// ids); potion sides are PotionItem aux values.

int BrewingRecipes::getResult(int inputAux, int ingredientId) {
	if (ingredientId == Item::netherWart->id) {
		// Base of every brew: water bottle -> awkward potion.
		if (inputAux == PotionItem::WATER) return PotionItem::AWKWARD;
		return -1;
	}
	if (inputAux != PotionItem::AWKWARD) {
		// Secondary ingredients only work on awkward potions, except
		// fermented spider eye, which corrupts finished potions.
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
	return itemId == Item::netherWart->id
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
