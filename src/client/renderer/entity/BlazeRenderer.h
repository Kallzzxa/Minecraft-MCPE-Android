#ifndef NET_MINECRAFT_CLIENT_RENDERER_ENTITY__BlazeRenderer_H__
#define NET_MINECRAFT_CLIENT_RENDERER_ENTITY__BlazeRenderer_H__

// v21: blaze renderer. The model is 20 units tall; 1.5x brings it to
// the 1.8-block entity height.

#include "MobRenderer.h"

#include "../gles.h"
#include "../../model/BlazeModel.h"
#include "../../../world/entity/monster/Blaze.h"

class BlazeRenderer: public MobRenderer
{
	typedef MobRenderer super;
public:
    BlazeRenderer()
	:	super(new BlazeModel(), 0.5f)
	{
    }

protected:
    /*@Override*/
    void scale(Mob* mob, float a) {
		(void) mob; (void) a;
        glScalef(1.5f, 1.5f, 1.5f);
    }
};

#endif /*NET_MINECRAFT_CLIENT_RENDERER_ENTITY__BlazeRenderer_H__*/
