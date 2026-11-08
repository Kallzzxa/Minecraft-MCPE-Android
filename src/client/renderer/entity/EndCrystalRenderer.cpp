#include "EndCrystalRenderer.h"
#include "EntityRenderDispatcher.h"
#include "../Tesselator.h"
#include "../../../world/entity/Entity.h"
#include "../../../world/entity/item/EndCrystal.h"
#include "../../../util/Mth.h"

EndCrystalRenderer::EndCrystalRenderer()
{
}

void EndCrystalRenderer::render(Entity* e, float x, float y, float z, float rot, float a)
{
	(void) rot;
	EndCrystal* crystal = (EndCrystal*) e;

	// The entity y is its feet (2 blocks tall); the crystal floats
	// centred 1 block up, bobbing gently and spinning around Y.
	float bob = Mth::sin((crystal->tickCount + a) * 0.08f) * 0.12f;
	float spin = (crystal->tickCount + a) * 3.0f;

	glPushMatrix2();

	glTranslatef2((float) x, (float) y + 1.0f + bob, (float) z);
	glRotatef2(spin, 0, 1, 0);

	glDisable2(GL_TEXTURE_2D);
	Tesselator& t = Tesselator::instance;

	// Outer crystal: two crossed vertical quads, pink glass.
	t.begin();
	t.color(1.0f, 0.45f, 0.85f);
	t.vertex(-0.45f, -0.9f, 0);
	t.vertex( 0.45f, -0.9f, 0);
	t.vertex( 0.45f,  0.9f, 0);
	t.vertex(-0.45f,  0.9f, 0);
	t.vertex(0, -0.9f, -0.45f);
	t.vertex(0, -0.9f,  0.45f);
	t.vertex(0,  0.9f,  0.45f);
	t.vertex(0,  0.9f, -0.45f);
	t.draw();

	// Bright core: same cross at half scale, near-white.
	t.begin();
	t.color(1.0f, 0.9f, 1.0f);
	t.vertex(-0.22f, -0.45f, 0);
	t.vertex( 0.22f, -0.45f, 0);
	t.vertex( 0.22f,  0.45f, 0);
	t.vertex(-0.22f,  0.45f, 0);
	t.vertex(0, -0.45f, -0.22f);
	t.vertex(0, -0.45f,  0.22f);
	t.vertex(0,  0.45f,  0.22f);
	t.vertex(0,  0.45f, -0.22f);
	t.draw();

	glEnable2(GL_TEXTURE_2D);

	glPopMatrix2();
}
