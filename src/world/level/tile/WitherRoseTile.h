#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__WitherRoseTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__WitherRoseTile_H__

// v35: wither rose (id 175, modern MC). A Bush (flower) with the
// cross-texture render shape and the small plant collision box,
// texture 253 (terrain.png — the cell was referenced ONLY by the
// never-generated placeholder tile info_updateGame2 (249), same
// spirit as v34 reusing cell 252 for the skull face).
//
// Differences from an ordinary flower (Bush), all deliberate:
//  - It grows anywhere, not just on grass/dirt/farmland:
//    mayPlaceOn accepts ANY solid block below (the wither kills
//    mobs in the Nether and the End too, where there is no grass),
//    and canSurvive drops Bush's light requirement (wither roses
//    do not need light) — it only needs solid support, so it pops
//    off like a torch when the block below it is broken.
//  - Touching it withers: entityInside (the cactus precedent —
//    Entity::move calls it for every tile in the mover's AABB)
//    gives any mob standing in it Wither I for 2 s, refreshed
//    while it stays in the flower (modern MC behaviour). The
//    wither boss itself is immune (same exemption as its own
//    skulls in WitherSkull::onHit); the rose does NOT mark the
//    victim as witheredByWither, so a mob killed by a rose never
//    drops another rose.
//  - It is never placed by world gen. It appears when the wither
//    boss kills a mob (Mob::die plants it where the mob died, or
//    drops it as an item when the spot cannot hold it), and the
//    player can pick it up, carry it and plant it by hand (the
//    auto-registered TileItem, id 431, is in the creative
//    inventory).

#include "Bush.h"
#include "../../entity/Mob.h"
#include "../../entity/EntityTypes.h"

class WitherRoseTile : public Bush
{
	typedef Bush super;
public:
	WitherRoseTile(int id, int tex)
	:	super(id, tex)
	{
	}

	// Any solid block can hold a wither rose (grass, dirt,
	// stone, netherrack, end stone, ...). Bush only accepts
	// grass/dirt/farmland.
	bool mayPlaceOn(int tile) {
		if (tile <= 0) return false;
		Tile* t = Tile::tiles[tile];
		return t != NULL && t->material != NULL && t->material->isSolid();
	}

	// No light requirement (unlike Bush, which needs light >= 8
	// or sky access) — only solid support underneath.
	bool canSurvive(Level* level, int x, int y, int z) {
		return level->isSolidBlockingTile(x, y - 1, z);
	}

	// Touching the rose withers (modern MC: Wither I, 2 s on
	// contact, refreshed while standing in it). Server-side
	// only — same rule as WitherSkull::onHit, which is the other
	// place the wither effect is applied.
	void entityInside(Level* level, int x, int y, int z, Entity* entity) {
		(void) x; (void) y; (void) z;
		if (level->isClientSide) return;
		if (entity == NULL || !entity->isMob()) return;
		if (entity->getEntityTypeId() == MobTypes::Wither) return;
		((Mob*)entity)->witherEffectTicks = 2 * 20;
	}
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_TILE__WitherRoseTile_H__*/
