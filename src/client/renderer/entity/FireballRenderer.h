#ifndef NET_MINECRAFT_CLIENT_RENDERER_ENTITY__FireballRenderer_H__
#define NET_MINECRAFT_CLIENT_RENDERER_ENTITY__FireballRenderer_H__

// v21: fireball renderer. A billboarded orange ball (outer glow +
// bright yellow core), modelled on ExperienceOrbRenderer — no texture
// needed, so no new asset to load.

#include "EntityRenderer.h"
#include "EntityRenderDispatcher.h"
#include "../Tesselator.h"
#include "../../../world/entity/Entity.h"
#include "../../../world/entity/projectile/Fireball.h"
#include "../../../util/Mth.h"

class FireballRenderer: public EntityRenderer
{
	typedef EntityRenderer super;
public:
	FireballRenderer()
	{
	}

	void render(Entity* e, float x, float y, float z, float rot, float a) {
		(void) e; (void) rot; (void) a;
		float pulse = 0.85f + 0.15f * Mth::sin((e->tickCount + a) * 0.3f);
		float s = 0.30f * pulse; // outer glow half-size
		float c = 0.16f * pulse; // bright core half-size

		glPushMatrix2();
		glTranslatef2((float) x, (float) y, (float) z);
		glRotatef2(180 - entityRenderDispatcher->playerRotY, 0, 1, 0);
		glRotatef2(-entityRenderDispatcher->playerRotX, 1, 0, 0);
		glDisable2(GL_TEXTURE_2D);
		Tesselator& t = Tesselator::instance;

		// Outer glow quad (orange).
		t.begin();
		t.color(1.0f, 0.55f, 0.10f);
		t.vertex(-s, -s, 0);
		t.vertex( s, -s, 0);
		t.vertex( s,  s, 0);
		t.vertex(-s,  s, 0);
		t.draw();

		// Bright core quad (yellow-white).
		t.begin();
		t.color(1.0f, 0.85f, 0.30f);
		t.vertex(-c, -c, 0);
		t.vertex( c, -c, 0);
		t.vertex( c,  c, 0);
		t.vertex(-c,  c, 0);
		t.draw();

		glEnable2(GL_TEXTURE_2D);
		glPopMatrix2();
	}
};

#endif /*NET_MINECRAFT_CLIENT_RENDERER_ENTITY__FireballRenderer_H__*/
