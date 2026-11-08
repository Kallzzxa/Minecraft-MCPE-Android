#ifndef NET_MINECRAFT_CLIENT_MODEL__SlimeModel_H__
#define NET_MINECRAFT_CLIENT_MODEL__SlimeModel_H__

//package net.minecraft.client.model;

#include "Model.h"

#include "geom/ModelPart.h"
#include "../../world/entity/Entity.h"
#include "../../util/Mth.h"

// Single-cube slime: one 16x16x16 box (1 block). MobRenderer translates
// the model so that model y=24 is at the entity's feet, so the box spans
// y 8..24. Squash-and-stretch comes from SlimeRenderer::scale, not here.
class SlimeModel: public Model
{
    typedef Model super;
public:
    ModelPart body;

    SlimeModel(float g = 0)
    :   body(0, 0)
    {
        body.setModel(this);

        body.addBox(-8, 0, -8, 16, 16, 16, g); // Body
        body.setPos(0, 8, 0);
    }

    /*@Override*/
    void render(Entity* entity, float time, float r, float bob, float yRot, float xRot, float scale) {
        setupAnim(time, r, bob, yRot, xRot, scale);

        body.render(scale);
    }

    void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale) {
        // No limbs; the hop squash is applied by the renderer.
    }
};

#endif /*NET_MINECRAFT_CLIENT_MODEL__SlimeModel_H__*/
