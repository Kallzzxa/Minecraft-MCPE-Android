#include "Ghast.h"

#include "../../item/Item.h"
#include "../../level/Level.h"
#include "../../level/tile/Tile.h"
#include "../../../util/Mth.h"
#include "../../../SharedConstants.h"
#include "../projectile/Fireball.h"

Ghast::Ghast(Level* level)
:	super(level)
{
	entityRendererId = ER_GHAST_RENDERER;
	textureName = "mob/ghast.png";
	setSize(4.0f, 4.0f);
	runSpeed = 0.5f;
	fireImmune = true;
	attackDamage = 0;
}

void Ghast::tick()
{
	super::tick();
	// Gentle hover bob so it never looks frozen in the air.
	if (!onGround) {
		yd += Mth::sin(tickCount * 0.06f) * 0.002f;
	}
}

// FlyingMob::travel, copied here because Ghast extends Monster (for
// the PathfinderMob chase + checkHurtTarget) rather than FlyingMob.
// The point is the same: no yd -= 0.08 gravity in air, so the ghast
// hovers at its height instead of falling to the floor.
void Ghast::travel(float xa, float ya)
{
	if (isInWater()) {
		moveRelative(xa, ya, 0.02f);
		move(xd, yd, zd);
		xd *= 0.80f;
		yd *= 0.80f;
		zd *= 0.80f;
	} else if (isInLava()) {
		moveRelative(xa, ya, 0.02f);
		move(xd, yd, zd);
		xd *= 0.50f;
		yd *= 0.50f;
		zd *= 0.50f;
	} else {
		float friction = 0.91f;
		if (onGround) {
			friction = 0.6f * 0.91f;
			int t = level->getTile(Mth::floor(x), Mth::floor(bb.y0 - 0.5f), Mth::floor(z));
			if (t > 0) {
				friction = Tile::tiles[t]->friction * 0.91f;
			}
		}
		float friction2 = (0.6f * 0.6f * 0.91f * 0.91f * 0.6f * 0.91f) / (friction * friction * friction);
		moveRelative(xa, ya, (onGround ? 0.1f * friction2 : 0.02f));
		friction = 0.91f;
		if (onGround) {
			friction = 0.6f * 0.91f;
			int t = level->getTile(Mth::floor(x), Mth::floor(bb.y0 - 0.5f), Mth::floor(z));
			if (t > 0) {
				friction = Tile::tiles[t]->friction * 0.91f;
			}
		}
		move(xd, yd, zd);
		xd *= friction;
		yd *= friction;
		zd *= friction;
	}
	walkAnimSpeedO = walkAnimSpeed;
	float xxd = x - xo;
	float zzd = z - zo;
	float wst = Mth::sqrt(xxd * xxd + zzd * zzd) * 4;
	if (wst > 1) wst = 1;
	walkAnimSpeed += (wst - walkAnimSpeed) * 0.4f;
	walkAnimPos += walkAnimSpeed;
}

void Ghast::checkHurtTarget(Entity* target, float d)
{
	// Ranged attack, skeleton-style: one aimed fireball every 3 s
	// while the target is within 28 blocks. No melee: a ghast never
	// gets close enough for Monster's 2-block attack to matter, and
	// attackDamage is 0 anyway.
	if (d < 28) {
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
}

void Ghast::dropDeathLoot()
{
	if (random.nextInt(2) == 0) {
		spawnAtLocation(Item::ghastTear->id, 1);
	}
	int powder = random.nextInt(3);
	for (int i = 0; i < powder; i++) {
		spawnAtLocation(Item::sulphur->id, 1);
	}
}
