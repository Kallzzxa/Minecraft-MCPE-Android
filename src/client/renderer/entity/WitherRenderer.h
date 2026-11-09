#ifndef NET_MINECRAFT_CLIENT_RENDERER_ENTITY__WitherRenderer_H__
#define NET_MINECRAFT_CLIENT_RENDERER_ENTITY__WitherRenderer_H__

// v34: wither renderer. The model is built at natural scale
// (16 units = 1 block), so there is no scale override, unlike the
// ghast's 4x — the plain MobRenderer scale is exactly right (same
// as the dragon).

#include "MobRenderer.h"

#include "../gles.h"
#include "../../model/WitherModel.h"
#include "../../../world/entity/monster/Wither.h"

class WitherRenderer: public MobRenderer
{
	typedef MobRenderer super;
public:
	WitherRenderer()
	:	super(new WitherModel(), 0.5f)
	{
	}

protected:
	// v36: phase tell in the world — the wither grows from
	// half-size to full while charging after the summon, and
	// bulks up ~8% once armoured (below half HP). Render scale
	// only; the hitbox is unchanged.
	void scale(Mob* mob, float a) {
		(void) a;
		Wither* wither = (Wither*)mob;
		if (wither->isCharging()) {
			float s = 0.5f + 0.5f * wither->getSpawnChargeFraction();
			glScalef(s, s, s);
		} else if (wither->isArmoured()) {
			glScalef(1.08f, 1.08f, 1.08f);
		}
	}
};

#endif /*NET_MINECRAFT_CLIENT_RENDERER_ENTITY__WitherRenderer_H__*/
