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

bool Blaze::hurt(Entity* source, int dmg)
{
	// v40: same owner-only rule as Ghast::hurt — immune to its
	// OWN fireball blasts (a blaze shoots at up to 16 blocks and
	// its blast can land beside it when the target closes in or
	// the shot hits the fortress wall), but a fireball from
	// ANOTHER blaze (different ownerId) still hurts, as do TNT /
	// creeper / crystal blasts, and a reflected fireball will
	// hurt once reflection exists and reassigns ownerId (see
	// Throwable::getOwnerId; no reflection mechanic exists in
	// this tree yet). Blocked hits return before Mob::hurt, so
	// no invulnerableTime, knockback or hurt sound is spent.
	if (source != NULL && source->getEntityTypeId() == EntityTypes::IdFireball
		&& ((Fireball*)source)->getOwnerId() == entityId) return false;
	return super::hurt(source, dmg);
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
