#ifndef NET_MINECRAFT_CLIENT_RENDERER_ENTITY__GhastRenderer_H__
#define NET_MINECRAFT_CLIENT_RENDERER_ENTITY__GhastRenderer_H__

// v21: ghast renderer. The model is 16 units tall (body + tentacles)
// and the entity is 4 blocks, so scale 4x (same trick as the slime's
// squash scale, but constant).

#include "MobRenderer.h"

#include "../gles.h"
#include "../../model/GhastModel.h"
#include "../../../world/entity/monster/Ghast.h"

class GhastRenderer: public MobRenderer
{
	typedef MobRenderer super;
public:
    GhastRenderer()
	:	super(new GhastModel(), 1.0f)
	{
    }

protected:
    /*@Override*/
    void scale(Mob* mob, float a) {
		(void) mob; (void) a;
        glScalef(4.0f, 4.0f, 4.0f);
    }
};

#endif /*NET_MINECRAFT_CLIENT_RENDERER_ENTITY__GhastRenderer_H__*/
