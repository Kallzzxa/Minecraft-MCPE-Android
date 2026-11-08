#ifndef NET_MINECRAFT_WORLD_ITEM_CRAFTING__BrewingRecipes_H__
#define NET_MINECRAFT_WORLD_ITEM_CRAFTING__BrewingRecipes_H__

// v20: brewing recipes — (input potion aux, ingredient item id) ->
// output potion aux. Follows modern MC where the ingredients exist in
// this tree: water bottle + nether wart -> awkward, awkward + secondary
// ingredient -> effect potion. No redstone (extend) / glowstone
// (level II) recipes: there is no potion-level system to upgrade yet.
// v25: gunpowder + any potion -> its splash variant (aux + 16, see
// PotionItem::SPLASH_FLAG); gunpowder is the only ingredient that
// accepts a splash input (it refuses it — already splash).
// The brewing stand (BrewingStandTileEntity) calls getResult() per
// bottle slot; the screen (BrewingScreen) calls isIngredient() to
// decide what the player may put in the ingredient slot.

class BrewingRecipes
{
public:
	// Output potion aux (see PotionItem), or -1 if the ingredient
	// does nothing to this potion.
	static int getResult(int inputAux, int ingredientId);

	// True if the item can be a brewing ingredient at all.
	static bool isIngredient(int itemId);

	// True if the item id is the blaze-powder fuel.
	static bool isFuel(int itemId);
};

#endif /*NET_MINECRAFT_WORLD_ITEM_CRAFTING__BrewingRecipes_H__*/
