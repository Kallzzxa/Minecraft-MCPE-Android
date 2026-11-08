#ifndef NET_MINECRAFT_CLIENT_RENDERER_ENTITY__ExperienceOrbRenderer_H__
#define NET_MINECRAFT_CLIENT_RENDERER_ENTITY__ExperienceOrbRenderer_H__

//package net.minecraft.client.renderer.entity;
//
// v8: renders an experience orb as a small billboarded glowing green
// quad (no texture — gui/icons.png has no XP-orb sprite in this 0.6.1
// tree, rows y=64..74 are fully transparent). Follows the
// ItemSpriteRenderer billboarding pattern.

#include "EntityRenderer.h"

class ExperienceOrbRenderer: public EntityRenderer
{
public:
	ExperienceOrbRenderer();

	void render(Entity* entity, float x, float y, float z, float rot, float a);
};

#endif /*NET_MINECRAFT_CLIENT_RENDERER_ENTITY__ExperienceOrbRenderer_H__*/
