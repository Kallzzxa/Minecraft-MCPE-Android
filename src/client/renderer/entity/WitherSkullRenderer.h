#ifndef NET_MINECRAFT_CLIENT_RENDERER_ENTITY__WitherSkullRenderer_H__
#define NET_MINECRAFT_CLIENT_RENDERER_ENTITY__WitherSkullRenderer_H__

// v34: wither-skull renderer. A billboarded dark ball (near-black
// outer glow + bright purple core), modelled on FireballRenderer —
// no texture needed, so no new asset to load.

#include "EntityRenderer.h"
#include "EntityRenderDispatcher.h"
#include "../Tesselator.h"
#include "../../../world/entity/Entity.h"
#include "../../../world/entity/projectile/WitherSkull.h"
#include "../../../util/Mth.h"

class WitherSkullRenderer: public EntityRenderer
{
	typedef EntityRenderer super;
public:
	WitherSkullRenderer()
	{
	}

	void render(Entity* e, float x, float y, float z, float rot, float a) {
		(void) e; (void) rot; (void) a;
		float pulse = 0.85f + 0.15f * Mth::sin((e->tickCount + a) * 0.3f);
		float s = 0.25f * pulse; // outer glow half-size
		float c = 0.13f * pulse; // bright core half-size

		glPushMatrix2();
		glTranslatef2((float) x, (float) y, (float) z);
		glRotatef2(180 - entityRenderDispatcher->playerRotY, 0, 1, 0);
		glRotatef2(-entityRenderDispatcher->playerRotX, 1, 0, 0);
		glDisable2(GL_TEXTURE_2D);
		Tesselator& t = Tesselator::instance;

		// Outer glow quad (near-black).
		t.begin();
		t.color(0.05f, 0.05f, 0.08f);
		t.vertex(-s, -s, 0);
		t.vertex( s, -s, 0);
		t.vertex( s,  s, 0);
		t.vertex(-s,  s, 0);
		t.draw();

		// Bright core quad (purple).
		t.begin();
		t.color(0.6f, 0.1f, 0.9f);
		t.vertex(-c, -c, 0);
		t.vertex( c, -c, 0);
		t.vertex( c,  c, 0);
		t.vertex(-c,  c, 0);
		t.draw();

		glEnable2(GL_TEXTURE_2D);
		glPopMatrix2();
	}
};

#endif /*NET_MINECRAFT_CLIENT_RENDERER_ENTITY__WitherSkullRenderer_H__*/
