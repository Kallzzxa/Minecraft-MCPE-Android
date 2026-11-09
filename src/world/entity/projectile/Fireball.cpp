#include "Fireball.h"

#include "../EntityTypes.h"
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
