#include "EnderDragon.h"

#include "../../item/Item.h"
#include "../../level/Level.h"
#include "../../level/tile/Tile.h"
#include "../../level/levelgen/EndLevelSource.h"
#include "../../../util/Mth.h"
#include "../../../SharedConstants.h"
#include "../player/Player.h"
#include "../projectile/Fireball.h"

EnderDragon::EnderDragon(Level* level)
:	super(level),
	perchState(PERCH_ORBIT),
	perchTimer(0)
{
	entityRendererId = ER_ENDER_DRAGON_RENDERER;
	textureName = "mob/dragon.png";
	setSize(4.0f, 2.0f);
	runSpeed = 0.5f;
	fireImmune = true;
	attackDamage = 6;
}

bool EnderDragon::hurt(Entity* source, int dmg)
{
	// v45: immune to its OWN fireball blasts only — the v40
	// owner-only rule, copied exactly from Ghast::hurt /
	// Blaze::hurt (modern MC: the dragon takes no damage from a
	// fireball it shot itself). The path is the same as the
	// ghast's: checkHurtTarget shoots `new Fireball(level, this)`
	// (v30: one aimed ball every 3 s within 40 blocks while
	// orbiting), Throwable's (Level, Mob) ctor sets ownerId to
	// the shooter's entityId, and Fireball::onHit explodes with
	// the fireball itself as the blast source (v40), so
	// Explosion calls hurt(fireball, dmg) and the ownerId check
	// here is exact. When it can actually happen: the dragon
	// orbits at r22/y82 and fires outward/down, so its own blast
	// landing beside it is even less plausible than the ghast's
	// (v40 audit) — the real path is a shot hitting a pillar or
	// the portal ring while the dragon banks close past it, and
	// its 4-wide body catches a radius-1 blast from ~2 blocks
	// away. A fireball from ANOTHER dragon (different ownerId)
	// still hurts — only one dragon exists at a time
	// (Minecraft::ensureEndDragon), so that half is structural,
	// not testable — as do TNT / creeper / crystal blasts (NULL
	// or another entity), player melee, arrows and end-crystal
	// damage. A REFLECTED fireball will hurt it too once
	// reflection is enabled for dragon balls and reassigns
	// ownerId to the deflector (v41 deliberately excluded the
	// dragon because this immunity did not exist yet; enabling
	// it is a separate batch/decision, NOT this one — see
	// ROADMAP v45). Blocked hits return before Mob::hurt, so no
	// invulnerableTime, knockback or hurt sound is spent.
	if (source != NULL && source->getEntityTypeId() == EntityTypes::IdFireball
		&& ((Fireball*)source)->getOwnerId() == entityId) return false;
	return super::hurt(source, dmg);
}

void EnderDragon::heal(int amount)
{
	// Mob::heal caps at 20 HP; the dragon's max is 200 (see header).
	if (health <= 0) return;
	health += amount;
	if (health > 200) health = 200;
	invulnerableTime = invulnerableDuration / 2;
}

void EnderDragon::steerTo(float tx, float ty, float tz)
{
	// The old orbit steering, factored out: ease toward a fixed
	// point (capped speed 0.35/tick), velocity smoothed 0.2.
	float dx = tx - x;
	float dy = ty - y;
	float dz = tz - z;
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
}

void EnderDragon::tick()
{
	// v28: perch state machine, BEFORE the base tick: the base
	// tick's travel() (copied from Ghast) moves the dragon by
	// (xd, yd, zd) with friction, so setting the velocity here is
	// the steering input.
	//
	// The perch point is the exit-portal centre (0.5, 66, 0.5):
	// the bedrock ring top is y=66 (ring blocks at y=65), so feet
	// at 66.0 sit the 4-wide dragon on the ring, overhanging the
	// centre air cell. The descend/ascend diagonal from the orbit
	// (r22/y82) to the centre clears every pillar top by 3+
	// blocks (closest pillar tops near the path are y=77 at r=20,
	// while the path is at y~80.5 there), so the dragon cannot get
	// stuck on a pillar; the ring is the only thing below the
	// perch point, so it cannot fall to the void either.
	if (!level->isClientSide) {
		perchTimer++;
		// Watchdog: below the island top the dragon has been knocked
		// off its path — abandon the perch and climb back to orbit.
		// (In ORBIT/ASCEND the steer target is already y=82, so
		// this only ever fires out of DESCEND/PERCHED.)
		if (y < 50.0f && perchState != PERCH_ASCEND) {
			perchState = PERCH_ASCEND;
			perchTimer = 0;
		}
		// The moving orbit point creeps around the island at
		// ~0.7 deg/tick (same formula as the old orbit-only code).
		float angle = tickCount * 0.012f;
		float ox = Mth::cos(angle) * ORBIT_RADIUS;
		float oz = Mth::sin(angle) * ORBIT_RADIUS;
		switch (perchState) {
		case PERCH_ORBIT:
			steerTo(ox, (float)ORBIT_HEIGHT, oz);
			if (perchTimer >= PERCH_ORBIT_TICKS) {
				perchState = PERCH_DESCEND;
				perchTimer = 0;
			}
			break;
		case PERCH_DESCEND: {
			steerTo(0.5f, 66.0f, 0.5f);
			float dx = 0.5f - x;
			float dy = 66.0f - y;
			float dz = 0.5f - z;
			if (dx * dx + dy * dy + dz * dz < 1.0f) {
				// Landed on the portal: sit still (steering holds
				// the point while perched, so a sword knockback
				// shoves it off the ring and it slides back).
				perchState = PERCH_PERCHED;
				perchTimer = 0;
			} else if (perchTimer >= PERCH_DESCEND_TIMEOUT) {
				// Never arrived (knocked away, or the platform is
				// built over) — give up and take off instead of
				// circling the portal forever.
				perchState = PERCH_ASCEND;
				perchTimer = 0;
			}
			break;
		}
		case PERCH_PERCHED:
			steerTo(0.5f, 66.0f, 0.5f);
			if (perchTimer >= PERCH_TIME_TICKS) {
				perchState = PERCH_ASCEND;
				perchTimer = 0;
			}
			break;
		case PERCH_ASCEND:
		default: {
			steerTo(ox, (float)ORBIT_HEIGHT, oz);
			float dx = ox - x;
			float dy = (float)ORBIT_HEIGHT - y;
			float dz = oz - z;
			if (dx * dx + dy * dy + dz * dz < 4.0f
				|| perchTimer >= PERCH_ASCEND_TIMEOUT) {
				perchState = PERCH_ORBIT;
				perchTimer = 0;
			}
			break;
		}
		}
		// Face the direction of travel. (While perched the velocity
		// is ~0, so this leaves yRot alone and PathfinderMob's
		// lookAt() in the base tick faces the player instead.)
		if (xd * xd + zd * zd > 0.001f) {
			yRot = (float)(std::atan2(zd, xd) * Mth::RADDEG) - 90;
		}
	}

	// NOTE: Mob::tick, not Monster::tick — Monster::tick removes the
	// mob on PEACEFUL, which would delete the boss (and its exit
	// portal with it, since the portal only appears on its death).
	// The dragon is the End's boss: it stays in peaceful. (v30:
	// its target acquisition is the dragon-only 48-block
	// findAttackTarget override, so it now fights while orbiting
	// instead of only once perched — see findAttackTarget above.)
	Mob::tick();
}

// v29: perch-hop fix (the v28 ROADMAP cosmetic risk, PAID).
// PathfinderMob::updateAi sets jumping=true when the next path node
// is above the dragon's feet (yd > 0) or on horizontal collision,
// and Mob::tick then calls jumpFromGround() (yd 0.42) if the dragon
// is onGround on the ring — a small hop while perched. The dragon
// never needs to jump: it flies by steering (Ghast-style travel,
// no gravity), so while PERCHED its jump flag is simply cleared
// after the base AI runs. The clear happens inside updateAi(),
// which Mob::tick calls BEFORE its jumping check in the same tick,
// so the suppression takes effect immediately. Dragon-only (this
// override lives on EnderDragon, not on PathfinderMob/Mob), and
// only in PERCHED — orbit/descent/ascent are untouched (the dragon
// is airborne there anyway, so jumpFromGround could not fire).
void EnderDragon::updateAi()
{
	super::updateAi();
	if (perchState == PERCH_PERCHED) jumping = false;
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

// v30: dragon-only wide target acquisition.
//
// AUDIT (why this exists): Monster::findAttackTarget
// (Monster.cpp:57) acquires the nearest player within 16 blocks
// (3D, via Level::getNearestPlayer + canSee). The dragon orbits at
// r=22, y=82 while the player stands on the island at y~66, so the
// typical 3D distance is sqrt(22^2 + 16^2) ~= 27 blocks — outside
// 16 — and the dragon almost never had a target while orbiting.
// Its fireball code (checkHurtTarget below, d<40, one per 3 s,
// ghast pattern) was therefore dead in the air: it only fought
// once it perched (player within a few blocks) or the player
// climbed a pillar. PathfinderMob::updateAi only calls
// findAttackTarget when it has no target yet, and once acquired
// it keeps calling checkHurtTarget every tick while canSee holds
// (dropping the target only if unseen AND d>32), so widening
// acquisition alone is enough — no change to the fireball logic,
// cooldown (3 s), speed (1.0) or inaccuracy (8) was needed.
// DRAGON_TARGET_RANGE (48) covers any player on the island
// (radius ~48) as the ~30 s orbit cycles past them, while the
// 40-block fire range still gates actual shots. Dragon-only:
// Monster::findAttackTarget is untouched, so no other mob's
// behaviour changes.
Entity* EnderDragon::findAttackTarget()
{
	Player* player = level->getNearestPlayer(this, (float)DRAGON_TARGET_RANGE);
	if (player != NULL && canSee(player)) return player;
	return NULL;
}

void EnderDragon::checkHurtTarget(Entity* target, float d)
{
	// Up close, the normal melee attack.
	if (d < 4.0f) {
		super::checkHurtTarget(target, d);
		return;
	}
	// v28: no fireballs while landing or perched — the modern dragon
	// does not shoot while it sits on the portal, and a point-blank
	// fireball would hit the portal ring itself. The melee bite
	// above still works while perched.
	if (perchState == PERCH_DESCEND || perchState == PERCH_PERCHED) return;
	// Ranged attack, ghast-style: one aimed fireball every 3 s
	// while the target is within 40 blocks. (v30: this now also
	// fires while ORBITING — the dragon-only findAttackTarget
	// override acquires the player from 48 blocks, so the
	// d<40 gate here is what actually starts the shots.)
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
