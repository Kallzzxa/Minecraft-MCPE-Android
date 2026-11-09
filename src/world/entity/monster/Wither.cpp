#include "Wither.h"

#include "../../item/Item.h"
#include "../../level/Level.h"
#include "../../level/tile/Tile.h"
#include "../../../util/Mth.h"
#include "../../../SharedConstants.h"
#include "../player/Player.h"
#include "../projectile/WitherSkull.h"

Wither::Wither(Level* level)
:	super(level)
{
	entityRendererId = ER_WITHER_RENDERER;
	textureName = "mob/wither.png";
	setSize(1.0f, 3.5f);
	runSpeed = 0.5f;
	fireImmune = true;
	attackDamage = 6;
}

void Wither::heal(int amount)
{
	// Mob::heal caps at 20 HP; the wither's max is 300 (see header).
	if (health <= 0) return;
	health += amount;
	if (health > WITHER_MAX_HEALTH) health = WITHER_MAX_HEALTH;
	invulnerableTime = invulnerableDuration / 2;
}

void Wither::steerTo(float tx, float ty, float tz)
{
	// The dragon's orbit steering, factored out: ease toward a
	// fixed point (capped speed 0.35/tick), velocity smoothed 0.2.
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

void Wither::tick()
{
	if (!level->isClientSide) {
		// Regenerate 1 HP / 20 ticks (modern MC). Direct health
		// increment, NOT heal(): heal() sets invulnerableTime,
		// which would make the boss unhittable half the time.
		if (health > 0 && health < WITHER_MAX_HEALTH && tickCount % 20 == 0) {
			health++;
		}
		// Chase: hover above the nearest player. No target in
		// range: hover in place with a gentle bob (ghast pattern).
		Player* p = level->getNearestPlayer(this, (float)WITHER_TARGET_RANGE);
		if (p != NULL) {
			steerTo(p->x, p->y + (float)WITHER_HOVER_HEIGHT, p->z);
		} else if (!onGround) {
			yd += Mth::sin(tickCount * 0.06f) * 0.002f;
		}
		// Face the direction of travel.
		if (xd * xd + zd * zd > 0.001f) {
			yRot = (float)(std::atan2(zd, xd) * Mth::RADDEG) - 90;
		}
	}

	// NOTE: Mob::tick, not Monster::tick — Monster::tick removes the
	// mob on PEACEFUL, which would delete the boss (same reason as
	// the dragon, see EnderDragon::tick).
	Mob::tick();
}

// The wither flies by steering — it never needs the ground jump
// PathfinderMob::updateAi sets on horizontal collision, so the flag
// is cleared after the base AI runs (same fix shape as the dragon's
// perch-hop suppression, but unconditional: the wither is airborne
// everywhere, not just while perched).
void Wither::updateAi()
{
	super::updateAi();
	jumping = false;
}

// FlyingMob-style travel, copied from Ghast::travel: no yd -= 0.08
// gravity in air, so the wither holds its hover altitude instead
// of falling to the ground.
void Wither::travel(float xa, float ya)
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

// Dragon-only wide target acquisition, same reason and same shape:
// the wither hovers ~3 blocks above the player, so the 3D distance
// while chasing is routinely past Monster's 16-block range, and
// the skull code (checkHurtTarget below, d<40) would almost never
// run. 48 blocks covers the hover + chase while the 40-block fire
// range still gates actual shots. Wither-only: Monster's version
// is untouched, so no other mob's behaviour changes.
Entity* Wither::findAttackTarget()
{
	Player* player = level->getNearestPlayer(this, (float)WITHER_TARGET_RANGE);
	if (player != NULL && canSee(player)) return player;
	return NULL;
}

void Wither::checkHurtTarget(Entity* target, float d)
{
	// Up close, the normal melee attack.
	if (d < 4.0f) {
		super::checkHurtTarget(target, d);
		return;
	}
	// Ranged attack, ghast-style: one aimed wither skull every 2 s
	// while the target is within 40 blocks. (Modern MC fires from
	// all three heads; stage 1 fires one skull per volley — the
	// three-head volley is stage 2.)
	if (d < 40) {
		float xd = target->x - x;
		float zd = target->z - z;
		if (attackTime == 0) {
			WitherSkull* ws = new WitherSkull(level, this);
			float yd = (target->y + target->getHeadHeight() - 0.7f) - ws->y;
			level->playSound(this, "random.bow", 1.0f, 0.6f);
			level->addEntity(ws);
			ws->shoot(xd, yd, zd, 1.0f, 8);
			attackTime = SharedConstants::TicksPerSecond * 2;
		}
		yRot = (float)(std::atan2(zd, xd) * Mth::RADDEG) - 90;
	}
}

int Wither::getExperienceReward(Player* killedBy)
{
	(void) killedBy;
	// Modern MC gives 50 XP.
	return 50;
}

void Wither::dropDeathLoot()
{
	// The wither's only drop in modern MC: one nether star.
	spawnAtLocation(Item::netherStar->id, 1);
	level->playSound(this, "random.explode", 1.0f, 0.5f);
}

bool Wither::trySummonFromSkull(Level* level, int x, int y, int z)
{
	if (level->isClientSide) return false;
	// The placed skull must sit on soul sand.
	if (level->getTile(x, y - 1, z) != Tile::hellSand->id) return false;

	// The T can run along X or along Z, and the freshly placed
	// skull can be the left, middle or right head of the bar (m =
	// its offset from the bar's middle). The check only runs when
	// a skull is placed, so a pre-existing T without skulls — or
	// skulls placed on anything but this pattern — can never
	// false-positive: only the last skull completes the pattern.
	static const int AXES[2][2] = {{1, 0}, {0, 1}};
	for (int a = 0; a < 2; a++) {
		int ax = AXES[a][0], az = AXES[a][1];
		for (int m = -1; m <= 1; m++) {
			int cx = x - m * ax, cz = z - m * az;
			bool ok = true;
			for (int i = -1; i <= 1 && ok; i++) {
				int bx = cx + i * ax, bz = cz + i * az;
				if (level->getTile(bx, y - 1, bz) != Tile::hellSand->id) ok = false;
				if (level->getTile(bx, y, bz) != Tile::skull->id) ok = false;
			}
			if (!ok) continue;
			// The stem: one soul sand below the middle of the bar.
			if (level->getTile(cx, y - 2, cz) != Tile::hellSand->id) continue;

			// Consume the whole structure (4 soul sand + 3 skulls).
			for (int i = -1; i <= 1; i++) {
				int bx = cx + i * ax, bz = cz + i * az;
				level->setTile(bx, y, bz, 0);
				level->setTile(bx, y - 1, bz, 0);
			}
			level->setTile(cx, y - 2, cz, 0);

			// Summon with an explosion, modern MC style (radius 3,
			// no fire — stage 1 keeps the blast modest).
			float sx = cx + 0.5f, sy = (float)(y + 1), sz = cz + 0.5f;
			level->explode(NULL, sx, sy + 1.0f, sz, 3.0f, false);
			level->playSound(sx, sy, sz, "random.explode", 1.0f, 0.5f);
			Wither* wither = new Wither(level);
			wither->setPos(sx, sy, sz);
			level->addEntity(wither);
			return true;
		}
	}
	return false;
}
