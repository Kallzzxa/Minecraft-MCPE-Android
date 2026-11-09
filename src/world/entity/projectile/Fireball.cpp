#include "Fireball.h"

#include "../EntityTypes.h"
#include "../player/Player.h"
#include "../../level/Level.h"
#include "../../phys/HitResult.h"

Fireball::Fireball(Level* level)
:	super(level)
{
	entityRendererId = ER_FIREBALL_RENDERER;
	setSize(0.5f, 0.5f);
}

Fireball::Fireball(Level* level, Mob* mob)
:	super(level, mob)
{
	entityRendererId = ER_FIREBALL_RENDERER;
	setSize(0.5f, 0.5f);
}

bool Fireball::isPickable()
{
	return !removed;
}

float Fireball::getPickRadius()
{
	// v41: 0.3 matches the 0.3 grow Throwable/Arrow use for their own
	// hit tests, so the crosshair target is ~1.1 blocks — close to
	// modern MC's 1-block fireball, and hittable at ~1 block/tick.
	return 0.3f;
}

bool Fireball::hurt(Entity* source, int damage)
{
	(void) damage;
	if (removed) return false;
	// Only a player punch reflects. Arrows reach here with the Arrow
	// as source (Arrow.cpp calls hurt(this)) and keep bouncing off
	// as before (super::hurt returns false); explosion blasts reach
	// here with a fireball/TNT/creeper as source and likewise do not
	// reflect. No weapon durability is spent: Player::attack only
	// calls hurtEnemy on mobs.
	if (source == NULL || !source->isPlayer()) return super::hurt(source, damage);

	// v41: fireball reflection (modern MC's ghast fight). The ball
	// flies back along the player's look direction, faster than it
	// came (1.5 blocks/tick vs the 1.0 a ghast/blaze shoots, so it
	// can catch a flying ghast), with no spread (uncertainty 0, so
	// it goes exactly where the player aims). ownerId is reassigned
	// to the deflector — the v40 contract: Ghast::hurt / Blaze::hurt
	// only reject fireballs whose ownerId is their own, so the
	// reflected ball hurts the ghast that shot it (10 HP — one
	// clean reflected hit kills it) and explodes normally on any
	// block/mob it reaches. The flightTime < 5 owner skip in
	// Throwable::tick now protects the DEFLECTOR for its first
	// ticks, not the shooter.
	//
	// Scope: ghast/blaze/dragon fireballs reflect. The owner is
	// resolved through Level::getEntity (session-only ids, same
	// semantics as v40); a dragon-owned ball reflects too since
	// v46 — v45 gave the dragon the v40 owner immunity, so a
	// reflected dragon ball (ownerId reassigned to the deflector
	// below) is no longer its own and hurts it, exactly like a
	// reflected ghast ball hurts its ghast. ownerId 0 (rebuilt
	// from a save) or an owner that no longer exists reflects
	// anyway: the ball was almost certainly ghast/blaze/dragon-
	// shot, and reflecting it breaks nobody's rules.
	Entity* owner = (ownerId != 0) ? level->getEntity(ownerId) : NULL;
	if (owner != NULL) {
		int type = owner->getEntityTypeId();
		if (type != MobTypes::Ghast && type != MobTypes::Blaze
			&& type != MobTypes::EnderDragon) return super::hurt(source, damage);
	}

	markHurt();
	if (!level->isClientSide) {
		Player* player = (Player*) source;
		Vec3 dir = player->getViewVector(1.0f);
		ownerId = player->entityId;
		shoot(dir.x, dir.y, dir.z, 1.5f, 0);
	}
	return true;
}

void Fireball::onHit(const HitResult& res)
{
	(void) res;
	if (!level->isClientSide) {
		// Radius 1 + fire, matching the modern ghast/blaze fireball.
		// Level::explode handles the blast, particles and sound.
		// v40: the blast's source is the fireball itself (not NULL)
		// — Explosion excludes its source from the blast and, more
		// importantly, the source identifies the blast as a fireball
		// AND carries the shooter in its ownerId, so Ghast::hurt /
		// Blaze::hurt can reject exactly their OWN fireballs (modern
		// MC: a ghast/blaze takes no damage from a fireball it shot
		// itself). NULL would be indistinguishable from TNT/creeper/
		// crystal blasts, which must still hurt them — and a blanket
		// "immune to all fireballs" rule would kill the reflected-
		// fireball mechanic, so the owner check lives in hurt(),
		// not here. Player + other mobs are unaffected by the source
		// change: same damage formula, knockback now away from the
		// blast centre (the fireball sits at it) instead of random.
		level->explode(this, x, y, z, 1.0f, true);
	}
	remove();
}
