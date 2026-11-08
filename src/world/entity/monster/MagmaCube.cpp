#include "MagmaCube.h"

#include "../../item/Item.h"

MagmaCube::MagmaCube(Level* level)
:	super(level)
{
	// Slime's ctor already set ER_SLIME_RENDERER, 1x1 size, hop
	// movement and the melee/stroll goals; only the Nether parts
	// differ.
	textureName = "mob/magmacube.png";
	fireImmune = true;
}

int MagmaCube::getDeathLoot()
{
	// Item::magmaCream was already registered in this tree (v20
	// brewing ingredient). Mob::dropDeathLoot drops 0-2 of it, like
	// the slime's 0-2 slimeballs.
	return Item::magmaCream->id;
}
