#ifndef NET_MINECRAFT_CLIENT_RENDERER_ENTITY__SlimeRenderer_H__
#define NET_MINECRAFT_CLIENT_RENDERER_ENTITY__SlimeRenderer_H__

//package net.minecraft.client.renderer.entity;

#include "MobRenderer.h"

#include "../gles.h"
#include "../../model/SlimeModel.h"
#include "../../../world/entity/monster/Slime.h"

class SlimeRenderer: public MobRenderer
{
	typedef MobRenderer super;
public:
    SlimeRenderer()
	:	super(new SlimeModel(), 0.5f)
	{
    }

protected:
    /*@Override*/
    void scale(Mob* mob, float a) {
        // Squash-and-stretch from the entity's hop cycle: squashed
        // (wide + short) on the ground, stretched in the air.
        float s = ((Slime*) mob)->getSquish(a);
        glScalef(1.0f + s * 0.5f, 1.0f - s * 0.5f, 1.0f + s * 0.5f);
    }
};

#endif /*NET_MINECRAFT_CLIENT_RENDERER_ENTITY__SlimeRenderer_H__*/
