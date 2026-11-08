#include "EnderDragon.h"

#include "../../item/Item.h"
#include "../../level/Level.h"
#include "../../level/tile/Tile.h"
#include "../../level/levelgen/EndLevelSource.h"
#include "../../../util/Mth.h"
#include "../../../SharedConstants.h"
#include "../projectile/Fireball.h"

EnderDragon::EnderDragon(Level* level)
:	super(level)
{
	entityRendererId = ER_ENDER_DRAGON_RENDERER;
	textureName = "mob/dragon.png";
	setSize(4.0f, 2.0f);
	runSpeed = 0.5f;
	fireImmune = true;
	attackDamage = 6;
}

void EnderDragon::heal(int amount)
{
	// Mob::heal caps at 20 HP; the dragon's max is 200 (see header).
	if (health <= 0) return;
	health += amount;
	if (health > 200) health = 200;
	invulnerableTime = invulnerableDuration / 2;
}

void EnderDragon::tick()
{
	// Orbit steering, BEFORE the base tick: the base tick's travel()
	// (copied from Ghast) moves the dragon by (xd, yd, zd) with
	// friction, so setting the velocity here is the steering input.
	// The orbit point creeps around the island at ~0.7 deg/tick.
	if (!level->isClientSide) {
		float angle = tickCount * 0.012f;
		float tx = Mth::cos(angle) * ORBIT_RADIUS;
		float tz = Mth::sin(angle) * ORBIT_RADIUS;
		float ty = (float)ORBIT_HEIGHT;
		float dx = tx - x;
		float dy = ty - y;
		float dz = tz - z;
		// Ease toward the orbit point (capped speed 0.35/tick).
		float wantX = dx * 0.06f;
		float wantY = dy * 0.06f;
		float wantZ = dz * 0.06f;
		float sp = Mth::sqrt(wantX * wantX + wantY * wantY + wantZ * wantZ);
		if (sp > 0.35f) {
			wantX *= 0.35f / sp;
			wantY *= 0.35f / sp;
			wantZ *= 0.35f / sp;
		}
		xd += (wantX - xd) * 0.2f;
		yd += (wantY - yd) * 0.2f;
		zd += (wantZ - zd) * 0.2f;
		// Face the direction of travel.
		if (xd * xd + zd * zd > 0.001f) {
			yRot = (float)(std::atan2(zd, xd) * Mth::RADDEG) - 90;
		}
	}

	// NOTE: Mob::tick, not Monster::tick — Monster::tick removes the
	// mob on PEACEFUL, which would delete the boss (and its exit
	// portal with it, since the portal only appears on its death).
	// The dragon is the End's boss: it stays in peaceful, it just
	// never gets a target there (findAttackTarget needs a player in
	// range and the peaceful spawner never hands it one — the
	// orbit continues regardless).
	Mob::tick();
}

// FlyingMob-style travel, copied from Ghast::travel: no yd -= 0.08
// gravity in air, so the dragon holds its orbit altitude instead
// of falling to the island.
void EnderDragon::travel(float xa, float ya)
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

void EnderDragon::checkHurtTarget(Entity* target, float d)
{
	// Up close, the normal melee attack.
	if (d < 4.0f) {
		super::checkHurtTarget(target, d);
		return;
	}
	// Ranged attack, ghast-style: one aimed fireball every 3 s
	// while the target is within 40 blocks.
	if (d < 40) {
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

int EnderDragon::getExperienceReward(Player* killedBy)
{
	(void) killedBy;
	// Modern MC gives 12,000 XP; Mob::dropExperience splits into
	// 1-3 XP orbs, so 12,000 would spawn ~6,000 orb entities.
	// 120 XP (~60 orbs) is the biggest sane reward here.
	return 120;
}

void EnderDragon::dropDeathLoot()
{
	// The exit portal activates: the centre of the spawn platform
	// becomes the end portal tile, and the dragon egg appears on
	// the platform rim. Minecraft::ensureEndDragon treats the
	// portal tile at the centre as "dragon already defeated", so
	// the dragon never respawns on re-entry.
	level->setTile(0, EndLevelSource::EXIT_PORTAL_Y, 0, Tile::endPortal->id);
	level->setTile(2, EndLevelSource::EXIT_PORTAL_Y, 0, Tile::dragonEgg->id);
	level->playSound(this, "random.explode", 1.0f, 0.5f);
}
