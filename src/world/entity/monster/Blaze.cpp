#include "Blaze.h"

#include "../../item/Item.h"
#include "../../level/Level.h"
#include "../../../util/Mth.h"
#include "../../../SharedConstants.h"
#include "../projectile/Fireball.h"

Blaze::Blaze(Level* level)
:	super(level)
{
	entityRendererId = ER_BLAZE_RENDERER;
	textureName = "mob/blaze.png";
	setSize(0.6f, 1.8f);
	runSpeed = 0.5f;
	fireImmune = true;
	attackDamage = 6;
}

void Blaze::checkHurtTarget(Entity* target, float d)
{
	// Ranged attack, skeleton-style: one aimed fireball every 3 s
	// while the target is within 16 blocks. Melee (super) still
	// applies up close when the cooldown has expired.
	if (d < 16) {
		float xd = target->x - x;
		float zd = target->z - z;
		if (attackTime == 0) {
			Fireball* fb = new Fireball(level, this);
			float yd = (target->y + target->getHeadHeight() - 0.7f) - fb->y;
			level->playSound(this, "random.bow", 1.0f, 0.6f);
			level->addEntity(fb);
			fb->shoot(xd, yd, zd, 1.0f, 8);
			attackTime = SharedConstants::TicksPerSecond * 3;
		}
		yRot = (float)(std::atan2(zd, xd) * Mth::RADDEG) - 90;
	}
	super::checkHurtTarget(target, d);
}

void Blaze::dropDeathLoot()
{
	if (random.nextInt(2) == 0) {
		spawnAtLocation(Item::blazeRod->id, 1);
	}
}
