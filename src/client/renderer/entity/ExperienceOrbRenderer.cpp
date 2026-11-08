#include "ExperienceOrbRenderer.h"
#include "EntityRenderDispatcher.h"
#include "../Tesselator.h"
#include "../../../world/entity/Entity.h"
#include "../../../world/entity/item/ExperienceOrb.h"
#include "../../../util/Mth.h"

ExperienceOrbRenderer::ExperienceOrbRenderer()
{
}

void ExperienceOrbRenderer::render( Entity* e, float x, float y, float z, float rot, float a )
{
	ExperienceOrb* orb = (ExperienceOrb*) e;

	// Gentle pulse so orbs are easy to spot on the ground.
	float pulse = 0.8f + 0.2f * Mth::sin((orb->tickCount + a) * 0.25f);
	float s = 0.22f * pulse;      // outer glow half-size
	float c = 0.12f * pulse;      // bright core half-size

	glPushMatrix2();

	glTranslatef2((float) x, (float) y, (float) z);

	glRotatef2(180 - entityRenderDispatcher->playerRotY, 0, 1, 0);
	glRotatef2(-entityRenderDispatcher->playerRotX, 1, 0, 0);

	glDisable2(GL_TEXTURE_2D);
	Tesselator& t = Tesselator::instance;

	// Outer glow quad (XP green).
	t.begin();
	t.color(0.45f, 1.0f, 0.15f);
	t.vertex(-s, -s, 0);
	t.vertex( s, -s, 0);
	t.vertex( s,  s, 0);
	t.vertex(-s,  s, 0);
	t.draw();

	// Bright core quad.
	t.begin();
	t.color(0.85f, 1.0f, 0.45f);
	t.vertex(-c, -c, 0);
	t.vertex( c, -c, 0);
	t.vertex( c,  c, 0);
	t.vertex(-c,  c, 0);
	t.draw();

	glEnable2(GL_TEXTURE_2D);

	glPopMatrix2();
}
