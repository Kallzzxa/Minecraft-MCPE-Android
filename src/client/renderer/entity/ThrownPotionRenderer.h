#ifndef NET_MINECRAFT_CLIENT_RENDERER_ENTITY__ThrownPotionRenderer_H__
#define NET_MINECRAFT_CLIENT_RENDERER_ENTITY__ThrownPotionRenderer_H__

// v24: thrown-potion renderer. Same billboarded item sprite as
// ItemSpriteRenderer (snowball/egg precedent), but the icon comes
// from the potion's own aux value, so a poison bottle looks like
// poison and a harming bottle looks like harming (PotionItem icons
// are one cell per aux along gui/items.png row 12).

#include "EntityRenderer.h"
#include "EntityRenderDispatcher.h"
#include "../Tesselator.h"
#include "../../../world/entity/Entity.h"
#include "../../../world/entity/projectile/ThrownPotion.h"
#include "../../../world/item/Item.h"
#include "../../../world/item/PotionItem.h"

class ThrownPotionRenderer: public EntityRenderer
{
	typedef EntityRenderer super;
public:
	ThrownPotionRenderer()
	{
	}

	void render(Entity* e, float x, float y, float z, float rot, float a) {
		(void) rot; (void) a;
		int aux = ((ThrownPotion*)e)->getPotionAux();
		int icon = ((PotionItem*)Item::potion)->getIcon(aux);

		glPushMatrix2();
		glTranslatef2((float) x, (float) y, (float) z);
		glScalef2(1 / 2.0f, 1 / 2.0f, 1 / 2.0f);
		bindTexture("gui/items.png");
		Tesselator& t = Tesselator::instance;

		float u0 = ((icon % 16) * 16 + 0) / 256.0f;
		float u1 = ((icon % 16) * 16 + 16) / 256.0f;
		float v0 = ((icon / 16) * 16 + 0) / 256.0f;
		float v1 = ((icon / 16) * 16 + 16) / 256.0f;

		float r = 1.0f;
		float xo = 0.5f;
		float yo = 0.25f;

		glRotatef2(180 - entityRenderDispatcher->playerRotY, 0, 1, 0);
		glRotatef2(-entityRenderDispatcher->playerRotX, 1, 0, 0);
		t.begin();
		t.vertexUV(0 - xo, 0 - yo, 0, u0, v1);
		t.vertexUV(r - xo, 0 - yo, 0, u1, v1);
		t.vertexUV(r - xo, 1 - yo, 0, u1, v0);
		t.vertexUV(0 - xo, 1 - yo, 0, u0, v0);
		t.draw();

		glPopMatrix2();
	}
};

#endif /*NET_MINECRAFT_CLIENT_RENDERER_ENTITY__ThrownPotionRenderer_H__*/
