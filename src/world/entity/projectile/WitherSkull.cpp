#include "WitherSkull.h"

#include "../EntityTypes.h"
#include "../Mob.h"
#include "../../level/Level.h"
#include "../../phys/HitResult.h"
#include "../../phys/AABB.h"

WitherSkull::WitherSkull(Level* level)
:	super(level)
{
	entityRendererId = ER_WITHER_SKULL_RENDERER;
	setSize(0.5f, 0.5f);
}

WitherSkull::WitherSkull(Level* level, Mob* mob)
:	super(level, mob)
{
	entityRendererId = ER_WITHER_SKULL_RENDERER;
	setSize(0.5f, 0.5f);
}

void WitherSkull::onHit(const HitResult& res)
{
	(void) res;
	if (!level->isClientSide) {
		// Wither I (10 s) on every mob caught in the blast — the
		// v32 timer, same cadence as a wither skeleton's melee hit.
		// The wither itself is immune to its own skulls (modern
		// MC: the wither is immune to the wither effect).
		AABB box(x - 2.0f, y - 2.0f, z - 2.0f, x + 2.0f, y + 2.0f, z + 2.0f);
		EntityList& entities = level->getEntities(this, box);
		for (unsigned int i = 0; i < entities.size(); i++) {
			Entity* e = entities[i];
			if (!e->isMob()) continue;
			if (e->getEntityTypeId() == MobTypes::Wither) continue;
			((Mob*)e)->witherEffectTicks = 10 * 20;
		}
		// Small explosion (radius 1, no fire — modern MC wither
		// skull). Level::explode handles the blast, particles and
		// sound.
		level->explode(NULL, x, y, z, 1.0f, false);
	}
	remove();
}
