#ifndef NET_MINECRAFT_CLIENT_RENDERER_ENTITY__EndermanRenderer_H__
#define NET_MINECRAFT_CLIENT_RENDERER_ENTITY__EndermanRenderer_H__

//package net.minecraft.client.renderer.entity;

#include "MobRenderer.h"

#include "../../model/EndermanModel.h"
#include "../../../world/entity/monster/Enderman.h"

class EndermanRenderer: public MobRenderer
{
	typedef MobRenderer super;
public:
    EndermanRenderer()
	:	super(new EndermanModel(), 0.5f)
	{
    }
};

#endif /*NET_MINECRAFT_CLIENT_RENDERER_ENTITY__EndermanRenderer_H__*/
