#ifndef NET_MINECRAFT_CLIENT_RENDERER_ENTITY__EndCrystalRenderer_H__
#define NET_MINECRAFT_CLIENT_RENDERER_ENTITY__EndCrystalRenderer_H__

// v27: renders an end crystal as a spinning, gently bobbing pair of
// crossed pink quads with a near-white core (no texture — the same
// plain-colour Tesselator precedent as ExperienceOrbRenderer). The
// spin comes from the entity tickCount, the bob from a sine on it,
// so the crystal reads as floating above its bedrock even though
// the entity itself never moves.

#include "EntityRenderer.h"

class EndCrystalRenderer: public EntityRenderer
{
public:
	EndCrystalRenderer();

	void render(Entity* entity, float x, float y, float z, float rot, float a);
};

#endif /*NET_MINECRAFT_CLIENT_RENDERER_ENTITY__EndCrystalRenderer_H__*/
