#ifndef NET_MINECRAFT_CLIENT_MODEL__WitherModel_H__
#define NET_MINECRAFT_CLIENT_MODEL__WitherModel_H__

// v34: wither model. Natural scale (16 units = 1 block, like the
// dragon — no renderer scale override): the model spans y -32..24
// (56 units = 3.5 blocks, the entity height; feet at y 24, like the
// humanoid models). Three 6x6x6 heads in a row on top (all three
// share one texture region — wither heads are identical), an 8x28
// ribcage body below, and a 4x22 tapering tail to the feet.
//
// Texture mob/wither.png is 64x64 (ChestModel precedent:
// ModelPart::setModel applies the model's tex size, so texWidth /
// texHeight are set here before any setModel call). Regions:
// heads (0,0) 24x18, body (24,0) 26x38, tail (24,38) 16x22.
// Front faces (Cube.cpp unwrap): head (6,6)-(12,12) with white
// eyes, body (29,5)-(37,33) with a bone ribcage.
//
// Like GhastModel, parts are default-constructed in the init list
// (only the tex offset differs) and never assigned — a ModelPart
// owns a GL buffer, so copying one would double-free it.

#include "Model.h"

#include "geom/ModelPart.h"
#include "../../world/entity/Entity.h"
#include "../../util/Mth.h"

class WitherModel: public Model
{
	typedef Model super;
public:
	ModelPart headCenter;
	ModelPart headLeft;
	ModelPart headRight;
	ModelPart body;
	ModelPart tail;

	WitherModel(float g = 0)
	:	headCenter(0, 0),
		headLeft(0, 0),
		headRight(0, 0),
		body(24, 0),
		tail(24, 38)
	{
		texWidth = 64;
		texHeight = 64;

		headCenter.setModel(this);
		headCenter.addBox(-3, 0, -3, 6, 6, 6, g);
		headCenter.setPos(0, -32, 0);

		headLeft.setModel(this);
		headLeft.addBox(-3, 0, -3, 6, 6, 6, g);
		headLeft.setPos(-6, -32, 0);

		headRight.setModel(this);
		headRight.addBox(-3, 0, -3, 6, 6, 6, g);
		headRight.setPos(6, -32, 0);

		body.setModel(this);
		body.addBox(-4, 0, -2, 8, 28, 5, g);
		body.setPos(0, -26, 0);

		tail.setModel(this);
		tail.addBox(-2, 0, -2, 4, 22, 4, g);
		tail.setPos(0, 2, 0);
	}

	/*@Override*/
	void render(Entity* entity, float time, float r, float bob, float yRot, float xRot, float scale) {
		setupAnim(time, r, bob, yRot, xRot, scale);
		headCenter.render(scale);
		headLeft.render(scale);
		headRight.render(scale);
		body.render(scale);
		tail.render(scale);
	}

	void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale) {
		// The side heads drift gently, so the boss never looks
		// frozen while it hovers.
		headLeft.yRot = Mth::sin(time * 0.05f) * 0.15f;
		headRight.yRot = -Mth::sin(time * 0.05f) * 0.15f;
	}
};

#endif /*NET_MINECRAFT_CLIENT_MODEL__WitherModel_H__*/
