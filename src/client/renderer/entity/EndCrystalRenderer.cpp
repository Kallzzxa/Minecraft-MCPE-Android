#include "EndCrystalRenderer.h"
#include "EntityRenderDispatcher.h"
#include "../Tesselator.h"
#include "../../../world/entity/Entity.h"
#include "../../../world/entity/Mob.h"
#include "../../../world/entity/item/EndCrystal.h"
#include "../../../world/level/Level.h"
#include "../../../util/Mth.h"

// v29: the dragon this crystal is healing, if any. This is the SAME
// condition as EndCrystal::tryHealDragon (server-side heal): the
// nearest live dragon within 32 blocks. It is re-derived here on
// the render side instead of syncing a "healing" flag across the
// network: the client crystal never runs tryHealDragon (its tick()
// returns early when level->isClientSide), and in single-player the
// renderer reads the same Level, so the beam appears exactly when
// the heal link is live — and vanishes when the dragon dies, flies
// out of range, or the crystal is destroyed (this renderer simply
// stops being called for a removed crystal).
static Mob* findHealedDragon(EndCrystal* crystal)
{
	if (crystal->level == NULL) return NULL;
	Mob* nearest = NULL;
	float nearestD2 = 32.0f * 32.0f;
	for (unsigned int i = 0; i < crystal->level->entities.size(); i++) {
		Entity* e = crystal->level->entities[i];
		if (e == NULL || e->removed
			|| e->getEntityTypeId() != MobTypes::EnderDragon) continue;
		Mob* dragon = (Mob*) e;
		if (dragon->health <= 0) continue;
		float dx = e->x - crystal->x;
		float dy = e->y - crystal->y;
		float dz = e->z - crystal->z;
		float d2 = dx * dx + dy * dy + dz * dz;
		if (d2 < nearestD2) {
			nearestD2 = d2;
			nearest = dragon;
		}
	}
	return nearest;
}

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

	// v29: heal beam. A thin ribbon from the crystal centre to the
	// dragon's body centre, drawn ONLY while findHealedDragon()
	// finds a live dragon in range (see above). No new render
	// pipeline: two crossed quads along the beam axis (the crystal's
	// own crossed-quad precedent, stretched into a line), plain
	// Tesselator colours, translucent via GL_BLEND (the TntRenderer
	// precedent — enabled here, disabled right after, so no other
	// renderer inherits the state). The two ribbons are
	// perpendicular, so from any viewing angle at least one face is
	// front-facing even with face culling on; depth testing stays
	// on, so pillars and the island occlude the beam like any other
	// geometry. The heal itself fires every 10 ticks, but the beam
	// is continuous while the in-range condition holds — a beam
	// flashing for a single frame per heal tick would be invisible.
	// Pulse: gentle alpha breathing, so the beam reads as an active
	// heal rather than a static decoration.
	Mob* dragon = findHealedDragon(crystal);
	if (dragon != NULL) {
		// Dragon body centre, interpolated like the dispatcher
		// does for the crystal itself (x/y/z arrive already
		// camera-relative: the dispatcher subtracted xOff/yOff/
		// zOff, so the same subtraction applies here). The dragon
		// is 2 blocks tall (setSize(4, 2)), so +1 is its centre.
		float ex = dragon->xo + (dragon->x - dragon->xo) * a - EntityRenderDispatcher::xOff;
		float ey = dragon->yo + (dragon->y - dragon->yo) * a + 1.0f - EntityRenderDispatcher::yOff;
		float ez = dragon->zo + (dragon->z - dragon->zo) * a - EntityRenderDispatcher::zOff;
		float sx = x;
		float sy = y + 1.0f + bob;
		float sz = z;

		float ax = ex - sx;
		float ay = ey - sy;
		float az = ez - sz;
		float len = Mth::sqrt(ax * ax + ay * ay + az * az);
		if (len > 0.5f) {
			ax /= len;
			ay /= len;
			az /= len;
			// side = axis x up (0,1,0) = (-az, 0, ax). Degenerate
			// only when the beam is near-vertical (dragon directly
			// overhead); then fall back to the X axis.
			float px = -az;
			float py = 0.0f;
			float pz = ax;
			float plen = Mth::sqrt(px * px + pz * pz);
			if (plen < 0.001f) {
				px = 1.0f;
				py = 0.0f;
				pz = 0.0f;
				plen = 1.0f;
			}
			px /= plen;
			py /= plen;
			pz /= plen;
			// up2 = side x axis: the second ribbon direction,
			// perpendicular to both the beam and the first ribbon.
			float ux = py * az - pz * ay;
			float uy = pz * ax - px * az;
			float uz = px * ay - py * ax;

			float pulse = 0.75f + 0.25f * Mth::sin((crystal->tickCount + a) * 0.3f);
			float outerW = 0.16f;
			float coreW = 0.06f;

			glDisable2(GL_TEXTURE_2D);
			glEnable2(GL_BLEND);
			glBlendFunc2(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			// Outer ribbon: pink, same hue as the crystal.
			t.begin();
			t.color(1.0f, 0.45f, 0.85f, 0.55f * pulse);
			t.vertex(sx + px * outerW, sy + py * outerW, sz + pz * outerW);
			t.vertex(sx - px * outerW, sy - py * outerW, sz - pz * outerW);
			t.vertex(ex - px * outerW, ey - py * outerW, ez - pz * outerW);
			t.vertex(ex + px * outerW, ey + py * outerW, ez + pz * outerW);
			t.draw();

			// Core ribbon: near-white, crossed against the outer
			// one so the beam reads from every angle.
			t.begin();
			t.color(1.0f, 0.9f, 1.0f, 0.85f * pulse);
			t.vertex(sx + ux * coreW, sy + uy * coreW, sz + uz * coreW);
			t.vertex(sx - ux * coreW, sy - uy * coreW, sz - uz * coreW);
			t.vertex(ex - ux * coreW, ey - uy * coreW, ez - uz * coreW);
			t.vertex(ex + ux * coreW, ey + uy * coreW, ez + uz * coreW);
			t.draw();

			glDisable2(GL_BLEND);
			glEnable2(GL_TEXTURE_2D);
		}
	}
}
