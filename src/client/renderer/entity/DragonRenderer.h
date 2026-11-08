#ifndef NET_MINECRAFT_CLIENT_RENDERER_ENTITY__DragonRenderer_H__
#define NET_MINECRAFT_CLIENT_RENDERER_ENTITY__DragonRenderer_H__

// v22: ender dragon renderer. The model is built at natural scale
// (16 units = 1 block), so there is no scale override, unlike the
// ghast's 4x — the plain MobRenderer scale is exactly right.

#include "MobRenderer.h"

#include "../gles.h"
#include "../../model/DragonModel.h"
#include "../../../world/entity/monster/EnderDragon.h"

class DragonRenderer: public MobRenderer
{
	typedef MobRenderer super;
public:
	DragonRenderer()
	:	super(new DragonModel(), 1.0f)
	{
	}
};

#endif /*NET_MINECRAFT_CLIENT_RENDERER_ENTITY__DragonRenderer_H__*/
