#ifndef NET_MINECRAFT_WORLD_ENTITY_MONSTER__Wither_H__
#define NET_MINECRAFT_WORLD_ENTITY_MONSTER__Wither_H__

// v36: wither boss, stage 3 — armoured phase + spawn charge-up.
//
// Armoured phase (below half HP, 150): the wither is immune to
// projectiles (arrows and every Throwable — modern MC arrow
// immunity), melee only. Deliberately NO damage reduction on top:
// the coordinator's tuning rule for v36 is "do not make the fight
// harder than v35" (no v34/v35 device feel yet), so the phase is a
// different second half, not a wall — melee DPS is unchanged, the
// player just has to close in. Once armoured, always armoured
// (derived from health, no flag to clear).
//
// Spawn charge-up: a summoned wither does not fight at once — it
// grows for WITHER_SPAWN_CHARGE_TICKS (5 s), fully invulnerable
// (hurt() returns false), hovering in place without chasing or
// attacking, then a power-7 explosion goes off (modern MC summon
// blast, was radius 3 in v34/v35) and the fight starts. The 5 s
// window is the dodge time: walk ~20 blocks away in it. The blast
// is centred on the wither (source = the wither, so the explosion
// itself never hurts it, and hurt() still blocks while the counter
// is 1 on the blast tick).
//
// Visuals: the renderer grows the wither from half-size to full
// during the charge and bulks it ~8% while armoured; the boss bar
// shows "Wither - Charging" / "Wither - Armoured" with a changed
// fill colour; smoke puffs rise off the wither while charging or
// armoured. The charge counter survives save/load ("WitherCharge");
// the armoured state needs no save (derived from health).
//
// Damage routing (audit): every hit in this tree arrives as
// Mob::hurt(Entity* source, int dmg) — Player::attack passes the
// player (melee), Arrow::tick passes the Arrow entity (type 80),
// explosions pass their source entity or NULL. So the armour check
// is a source-type check in this one override; NULL-source damage
// (explosions, wither-effect ticks) is NOT blocked in the armoured
// phase — TNT stays a valid weapon and the wither can still be
// hurt by its own skulls' blasts, exactly as in v34/v35.
//
// No phase/invulnerable pattern existed to copy: the EnderDragon
// (v22/v28) has a perch STATE MACHINE (ORBIT/DESCEND/PERCHED/
// ASCEND) but no damage-phase and no invulnerability — the dragon
// is always hittable, even perched. The charge counter here is a
// plain countdown in tick(), not a dragon-style state machine.

#include "Monster.h"

class Wither: public Monster
{
	typedef Monster super;
public:
	Wither(Level* level);

	int getMaxHealth() { return WITHER_MAX_HEALTH; }
	int getEntityTypeId() const { return MobTypes::Wither; }
	int getMaxSpawnClusterSize() { return 1; }

	// Mob::heal caps at 20 HP; the wither's max is 300 (same
	// override shape as the dragon's 200).
	void heal(int amount);

	// v36: damage gate — charge-up invulnerability + armoured
	// projectile immunity (see header comment). Blocked hits
	// return false before Mob::hurt, so no invulnerableTime,
	// knockback or hurt sound is spent on them; an arrow that
	// fails here bounces off (Arrow::tick's hurt==false path).
	bool hurt(Entity* source, int dmg);

	void tick();
	void travel(float xa, float ya);

	// The wither flies by steering — it never needs the ground
	// jump PathfinderMob::updateAi sets on horizontal collision
	// (same fix shape as the dragon's perch-hop suppression,
	// but unconditional: the wither is airborne everywhere).
	void updateAi();

	// v36: armoured phase — live, below half HP (150). Derived
	// from health, so it survives save/load for free and can
	// never be "turned off" once entered.
	bool isArmoured() { return health > 0 && health < WITHER_ARMOUR_HEALTH; }
	// v36: still growing after the summon (charge counter > 0).
	bool isCharging() { return spawnChargeTicks > 0; }
	int getSpawnChargeTicks() { return spawnChargeTicks; }
	// v36: 0.0 just summoned, 1.0 fully grown (renderer scale).
	float getSpawnChargeFraction() {
		return 1.0f - (float)spawnChargeTicks / (float)WITHER_SPAWN_CHARGE_TICKS;
	}
	// v36: called by trySummonFromSkull — starts the 5 s
	// charge-up instead of exploding at once (v34/v35).
	void startSpawnCharge() { spawnChargeTicks = WITHER_SPAWN_CHARGE_TICKS; }

	void addAdditonalSaveData(CompoundTag* tag);
	void readAdditionalSaveData(CompoundTag* tag);

	const char* getAmbientSound() { return "mob.skeleton"; }
	std::string getHurtSound() { return "mob.skeletonhurt"; }
	std::string getDeathSound() { return "mob.skeletondeath"; }

	virtual int getExperienceReward(Player* killedBy);

	// Ritual detection. (x, y, z) is a freshly placed skull tile.
	// Returns true when the full T + 3 skulls matched and a wither
	// was summoned (the structure is consumed). Server-side only
	// (level->isClientSide bails out — in MP the client also runs
	// the place path, and the server is the one that summons).
	static bool trySummonFromSkull(Level* level, int x, int y, int z);

protected:
	void checkHurtTarget(Entity* target, float d);
	void dropDeathLoot();

	// Melee (d < 4, via Monster::checkHurtTarget): the boss's
	// swing also withers for 10 s and marks the victim as
	// withered-by-the-boss (modern MC melee wither; the rose
	// marker). Another wither is never withered (boss immunity,
	// same rule as its own skulls). Needs Monster::doHurtTarget
	// virtual (v35 audit fix — see Monster.h).
	bool doHurtTarget(Entity* target);

	// Dragon-only wide target acquisition, same reason: a flying
	// boss hovers well outside Monster's 16-block range, so the
	// 16-block findAttackTarget would almost never acquire while
	// the wither circles above the player.
	Entity* findAttackTarget();

private:
	static const int WITHER_MAX_HEALTH = 300;
	static const int WITHER_TARGET_RANGE = 48;
	static const int WITHER_HOVER_HEIGHT = 3; // blocks above the player
	// v36 tuning (PROVISIONAL — no v34/v35 device feel yet, see
	// ROADMAP v36): armour below half HP; 5 s charge-up; power-7
	// summon blast (modern MC value, was radius 3 in v34/v35).
	static const int WITHER_ARMOUR_HEALTH = 150;
	static const int WITHER_SPAWN_CHARGE_TICKS = 100;
	static const float WITHER_SUMMON_BLAST_POWER;

	// v36: charge-up countdown (ticks left). 0 = fighting.
	// Set only by startSpawnCharge() at summon; persisted in
	// the save tag so a quit mid-charge resumes the charge.
	int spawnChargeTicks;

	// v36: is this damage source a projectile? Arrow (80),
	// snowball (81), egg (82), fireball (85), splash potion
	// (86), wither skull (88) — every EntityTypes projectile
	// id. Melee (a Mob/Player source) and explosions (source =
	// the exploding entity, or NULL) are NOT projectiles.
	static bool isProjectileSource(Entity* source);

	// Steer the velocity toward a fixed point (the dragon's orbit
	// steering, reused exactly: ease toward the point, capped
	// 0.35/tick, velocity smoothed 0.2).
	void steerTo(float tx, float ty, float tz);
};

#endif /*NET_MINECRAFT_WORLD_ENTITY_MONSTER__Wither_H__*/
