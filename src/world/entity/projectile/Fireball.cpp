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
		level->explode(NULL, x, y, z, 1.0f, true);
	}
	remove();
}
