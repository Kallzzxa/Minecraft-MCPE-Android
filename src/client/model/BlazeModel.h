#ifndef NET_MINECRAFT_CLIENT_MODEL__BlazeModel_H__
#define NET_MINECRAFT_CLIENT_MODEL__BlazeModel_H__

// v21: blaze model. Head 8x8x8 (UV (0,0): front face (8,8)-(16,16),
// eyes there in mob/blaze.png) + 3 floating rods 2x12x2 arranged in
// a triangle around the body (UV (0,16): front face (2,18)-(4,30)).
// Model spans y 4..24 (20 units); the renderer scales 1.5x to the
// 1.8-block entity height. Rods bob slightly in setupAnim.

#include "Model.h"

#include "geom/ModelPart.h"
#include "../../world/entity/Entity.h"
#include "../../util/Mth.h"

class BlazeModel: public Model
{
    typedef Model super;
public:
    ModelPart head;
    ModelPart rods[3];

    BlazeModel(float g = 0)
    :   head(0, 0)
    {
        head.setModel(this);
        head.addBox(-4, 0, -4, 8, 8, 8, g); // Head
        head.setPos(0, 4, 0);

        static const float rodXZ[3][2] = {
            { 0.0f, 5.0f }, { 4.3f, -2.5f }, { -4.3f, -2.5f }
        };
        for (int i = 0; i < 3; i++) {
            // No assignment: ModelPart owns a GL buffer (see
            // GhastModel). Default tex offset is (0, 0); rods need
            // (0, 16), set via texOffs before adding the box.
            rods[i].texOffs(0, 16);
            rods[i].setModel(this);
            rods[i].addBox(-1, 0, -1, 2, 12, 2, g);
            rods[i].setPos(rodXZ[i][0], 12, rodXZ[i][1]);
        }
    }

    /*@Override*/
    void render(Entity* entity, float time, float r, float bob, float yRot, float xRot, float scale) {
        setupAnim(time, r, bob, yRot, xRot, scale);
        head.render(scale);
        for (int i = 0; i < 3; i++) {
            rods[i].render(scale);
        }
    }

    void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale) {
        for (int i = 0; i < 3; i++) {
            rods[i].y = 12 + Mth::sin(time * 0.1f + (float)i * 2.1f) * 0.6f;
        }
    }
};

#endif /*NET_MINECRAFT_CLIENT_MODEL__BlazeModel_H__*/
