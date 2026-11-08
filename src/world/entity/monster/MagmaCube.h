#ifndef NET_MINECRAFT_WORLD_ENTITY_MONSTER__MagmaCube_H__
#define NET_MINECRAFT_WORLD_ENTITY_MONSTER__MagmaCube_H__

// v31: magma cube (MobTypes::MagmaCube = 44). A Nether slime: a
// Slime subclass, so the hop movement, squash-and-stretch animation
// (SlimeRenderer casts to Slime*, which a subclass satisfies) and
// the zombie-style goals all come for free. Differences from the
// overworld slime: fire-immune (lava does not hurt it), orange
// magma texture (mob/magmacube.png, a tint of mob/slime.png), and
// it drops magma cream (Item::magmaCream, already in the tree since
// v20 brewing) instead of slimeballs. Single size only (8 HP), like
// the v12 slime — no splitting into smaller cubes on death.

#include "Slime.h"

class MagmaCube: public Slime
{
	typedef Slime super;
public:
	MagmaCube(Level* level);

	int getMaxHealth() { return 8; }
	int getEntityTypeId() const { return MobTypes::MagmaCube; }

protected:
	int getDeathLoot();
};

#endif /*NET_MINECRAFT_WORLD_ENTITY_MONSTER__MagmaCube_H__*/
