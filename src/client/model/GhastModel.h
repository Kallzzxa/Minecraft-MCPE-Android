#ifndef NET_MINECRAFT_CLIENT_MODEL__GhastModel_H__
#define NET_MINECRAFT_CLIENT_MODEL__GhastModel_H__

// v21: ghast model. The entity is 4 blocks tall (renderer scales 4x),
// so the model spans y 8..24 (16 units): body 12 units (y 12..24) +
// 9 tentacles 4 units hanging below it (y 8..12). Body UV (0,0):
// front face is (16,12)-(32,24) — the eyes/mouth live there in
// mob/ghast.png. Tentacles share UV (0,0) (the free (0,0)-(8,6)
// corner); they sway gently in setupAnim.

#include "Model.h"

#include "geom/ModelPart.h"
#include "../../world/entity/Entity.h"
#include "../../util/Mth.h"

class GhastModel: public Model
{
    typedef Model super;
public:
    ModelPart body;
    ModelPart tentacles[9];

    GhastModel(float g = 0)
    :   body(0, 0)
    {
        body.setModel(this);
        body.addBox(-8, 0, -8, 16, 12, 16, g); // Body
        body.setPos(0, 12, 0);

        int i = 0;
        for (int tx = -6; tx <= 6; tx += 6) {
            for (int tz = -6; tz <= 6; tz += 6) {
                // No assignment here: ModelPart owns a GL buffer, so
                // copying one would share (and double-free) the VBO.
                // Array elements are default-constructed (0, 0) and
                // only the tex offset differs per part — set below.
                tentacles[i].setModel(this);
                tentacles[i].addBox(-1, 0, -1, 2, 4, 2, g);
                tentacles[i].setPos((float)tx, 8, (float)tz);
                i++;
            }
        }
    }

    /*@Override*/
    void render(Entity* entity, float time, float r, float bob, float yRot, float xRot, float scale) {
        setupAnim(time, r, bob, yRot, xRot, scale);
        body.render(scale);
        for (int i = 0; i < 9; i++) {
            tentacles[i].render(scale);
        }
    }

    void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale) {
        for (int i = 0; i < 9; i++) {
            tentacles[i].xRot = Mth::sin(time * 0.08f + (float)i * 0.7f) * 0.08f;
        }
    }
};

#endif /*NET_MINECRAFT_CLIENT_MODEL__GhastModel_H__*/
