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
};

#endif /*NET_MINECRAFT_CLIENT_RENDERER_ENTITY__WitherRenderer_H__*/
