#ifndef NET_MINECRAFT_CLIENT_MODEL__DragonModel_H__
#define NET_MINECRAFT_CLIENT_MODEL__DragonModel_H__

// v22: ender dragon model. The entity is ~4 blocks long, so the
// model is built at natural scale (16 units = 1 block, no renderer
// scale override): a 32-unit body, a 12-unit head in front, two
// flat wings (26x2x12) and a two-segment tail. Texture
// mob/dragon.png (64x32, drawn): dark body with a lighter spine
// stripe and purple eyes on the head's front face (head offset
// (36,0) puts its front face at x 48..60, y 12..24).
//
// Wings flap in setupAnim (zRot roll); the tail sways gently
// (xRot). Like GhastModel, parts are default-constructed and only
// the tex offset differs per part — set below, never assigned (a
// ModelPart owns a GL buffer, copying one would double-free it).

#include "Model.h"

#include "geom/ModelPart.h"
#include "../../world/entity/Entity.h"
#include "../../util/Mth.h"

class DragonModel: public Model
{
	typedef Model super;
public:
	ModelPart body;
	ModelPart head;
	ModelPart wingL;
	ModelPart wingR;
	ModelPart tail1;
	ModelPart tail2;

	DragonModel(float g = 0)
	:	body(0, 0),
		head(36, 0),
		wingL(0, 0),
		wingR(0, 0),
		tail1(0, 0),
		tail2(0, 0)
	{
		body.setModel(this);
		body.addBox(-16, 0, -8, 32, 12, 16, g);
		body.setPos(0, 10, 0);

		head.setModel(this);
		head.addBox(-6, 0, -6, 12, 12, 12, g);
		head.setPos(0, 10, -14);

		wingL.setModel(this);
		wingL.addBox(0, 0, -6, 26, 2, 12, g);
		wingL.setPos(14, 12, 0);

		wingR.setModel(this);
		wingR.addBox(-26, 0, -6, 26, 2, 12, g);
		wingR.setPos(-14, 12, 0);

		tail1.setModel(this);
		tail1.addBox(-4, 0, -4, 8, 8, 16, g);
		tail1.setPos(0, 12, 16);

		tail2.setModel(this);
		tail2.addBox(-3, 0, -3, 6, 6, 12, g);
		tail2.setPos(0, 13, 28);
	}

	/*@Override*/
	void render(Entity* entity, float time, float r, float bob, float yRot, float xRot, float scale) {
		setupAnim(time, r, bob, yRot, xRot, scale);
		body.render(scale);
		head.render(scale);
		wingL.render(scale);
		wingR.render(scale);
		tail1.render(scale);
		tail2.render(scale);
	}

	void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale) {
		float flap = Mth::sin(time * 0.15f) * 0.45f;
		wingL.zRot = flap;
		wingR.zRot = -flap;
		tail1.xRot = Mth::sin(time * 0.08f) * 0.08f;
		tail2.xRot = Mth::sin(time * 0.08f + 0.6f) * 0.12f;
	}
};

#endif /*NET_MINECRAFT_CLIENT_MODEL__DragonModel_H__*/
