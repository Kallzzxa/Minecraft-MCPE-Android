#ifndef NET_MINECRAFT_CLIENT_MODEL__EndermanModel_H__
#define NET_MINECRAFT_CLIENT_MODEL__EndermanModel_H__

//package net.minecraft.client.model;

#include "Model.h"

#include "geom/ModelPart.h"
#include "../../world/entity/Entity.h"
#include "../../util/Mth.h"

// Tall, thin humanoid (vanilla enderman proportions): 30px arms/legs,
// 12px body, 8px head. Model spans y -26..24 (feet at 24, like
// HumanoidModel); UV offsets follow the standard humanoid layout so the
// mob/enderman.png texture maps the same way as char.png.
class EndermanModel: public Model
{
    typedef Model super;
public:
    ModelPart head, body, arm0, arm1, leg0, leg1;

    EndermanModel(float g = 0)
    :   head(0, 0),
        body(16, 16),
        arm0(40, 16),
        arm1(40, 16),
        leg0(0, 16),
        leg1(0, 16)
    {
        head.setModel(this);
        body.setModel(this);
        arm0.setModel(this);
        arm1.setModel(this);
        leg0.setModel(this);
        leg1.setModel(this);

        head.addBox(-4, -8, -4, 8, 8, 8, g); // Head
        head.setPos(0, -18, 0);

        body.addBox(-4, 0, -2, 8, 12, 4, g); // Body
        body.setPos(0, -18, 0);

        arm0.addBox(-3, -2, -2, 4, 30, 4, g); // Arm0
        arm0.setPos(-5, -16, 0);

        arm1.mirror = true;
        arm1.addBox(-1, -2, -2, 4, 30, 4, g); // Arm1
        arm1.setPos(5, -16, 0);

        leg0.addBox(-2, 0, -2, 4, 30, 4, g); // Leg0
        leg0.setPos(-2, -6, 0);

        leg1.mirror = true;
        leg1.addBox(-2, 0, -2, 4, 30, 4, g); // Leg1
        leg1.setPos(2, -6, 0);
    }

    /*@Override*/
    void render(Entity* entity, float time, float r, float bob, float yRot, float xRot, float scale) {
        setupAnim(time, r, bob, yRot, xRot, scale);

        head.render(scale);
        body.render(scale);
        arm0.render(scale);
        arm1.render(scale);
        leg0.render(scale);
        leg1.render(scale);
    }

    void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale) {
        head.yRot = yRot / (float) (180 / Mth::PI);
        head.xRot = xRot / (float) (180 / Mth::PI);

        // Same walk swing as HumanoidModel; long limbs, same phase.
        const float tcos0 = Mth::cos(time * 0.6662f) * r;
        const float tcos1 = Mth::cos(time * 0.6662f + Mth::PI) * r;

        arm0.xRot = tcos1;
        arm1.xRot = tcos0;
        arm0.zRot = 0;
        arm1.zRot = 0;

        leg0.xRot = tcos0 * 1.4f;
        leg1.xRot = tcos1 * 1.4f;
        leg0.yRot = 0;
        leg1.yRot = 0;
    }
};

#endif /*NET_MINECRAFT_CLIENT_MODEL__EndermanModel_H__*/
