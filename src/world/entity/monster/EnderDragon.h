#ifndef NET_MINECRAFT_WORLD_ENTITY_MONSTER__EnderDragon_H__
#define NET_MINECRAFT_WORLD_ENTITY_MONSTER__EnderDragon_H__

// v22: ender dragon (MobTypes::EnderDragon = 41). The End boss.
//
// A Monster (old-AI, like the ghast) that flies a steady orbit
// around the island centre (radius 22, y=82 — above every pillar
// top inside that radius) and shoots fireballs at the player from
// up to 40 blocks away, skeleton-style, with a melee fallback up
// close. 200 HP, 6 melee damage, fire-immune.
//
// v28: PERCH. The dragon no longer only circles: every ~20 s of
// orbiting it descends onto the exit portal, sits there ~8 s
// (melee-vulnerable — the modern fight's sword phase, and the only
// time the dragon is reliably in sword range), then takes off and
// resumes its orbit. The perch is a small state machine on top of
// the existing steer-to-point orbit code (see tick()): every
// non-orbit state has a timeout that forces takeoff, the steer
// targets are fixed safe points (orbit r22/y82, portal (0.5,66,
// 0.5)), and a y<50 watchdog climbs back to orbit, so the dragon
// can never wedge in a perch state, get steered into the void, or
// get stuck on a pillar (the descend/ascend diagonal clears every
// pillar top by 3+ blocks — see the v28 ROADMAP note). While
// landing or perched it does not shoot fireballs (modern MC does
// not either); its melee bite still works up close. Perch state is
// transient (not saved): a reloaded dragon starts in orbit.
//
// Spawning: NOT in any spawn list. Minecraft::ensureEndDragon
// spawns exactly one when the player first enters the End, and
// never again once the exit portal exists (the dragon's death
// fills it — see dropDeathLoot). A dragon killed by a player drops
// 120 XP (modern MC gives 12,000, but Mob::dropExperience splits
// into 1-3 XP orbs, so 12,000 would spawn ~6,000 orb entities;
// 120 is the biggest sane reward here) and activates the exit
// portal + dragon egg. Sounds are placeholders (no dragon audio
// files exist in this tree; v19 left the enderman silent rather
// than faking it — the dragon keeps silent ambient, pop for hurt,
// explode for death).

#include "Monster.h"

class EnderDragon: public Monster
{
	typedef Monster super;
public:
	EnderDragon(Level* level);

	int getMaxHealth() { return 200; }
	int getEntityTypeId() const { return MobTypes::EnderDragon; }
	int getMaxSpawnClusterSize() { return 1; }

	// v27: Mob::heal caps at 20 HP (Mob.cpp) — a 200 HP dragon could
	// never be healed by its crystals. Cap at the dragon's own max.
	void heal(int amount);

	void tick();
	void travel(float xa, float ya);

	// v29: suppress the perch hop (see the .cpp note). Dragon-only
	// override of the virtual Mob::updateAi — no other mob is
	// touched.
	void updateAi();

	// v45: immune to its OWN fireballs only (see EnderDragon::hurt)
	// — a reflected fireball or another dragon's still hurts it.
	bool hurt(Entity* source, int dmg);

	const char* getAmbientSound() { return NULL; }
	std::string getHurtSound() { return "random.pop"; }
	std::string getDeathSound() { return "random.explode"; }

	virtual int getExperienceReward(Player* killedBy);

protected:
	void checkHurtTarget(Entity* target, float d);
	void dropDeathLoot();

	// v30: dragon-only wide target acquisition (see the .cpp note).
	// Monster::findAttackTarget (16 blocks) is left untouched for
	// every other mob.
	Entity* findAttackTarget();

private:
	// Orbit: radius 22 around (0, 0), altitude 82. The dragon
	// steers toward the moving orbit point every tick.
	static const int ORBIT_RADIUS = 22;
	static const int ORBIT_HEIGHT = 82;

	// v30: target acquisition range. The orbit (r22/y82) puts the
	// dragon typically ~27 blocks (3D) from a player standing on
	// the island at y~66, so the 16-block Monster range almost
	// never acquires while orbiting. 48 covers any player on the
	// island (radius ~48) as the orbit cycles, while the fireball
	// itself still only fires within 40 blocks (checkHurtTarget),
	// so acquisition alone never makes the dragon shoot from
	// across the void.
	static const int DRAGON_TARGET_RANGE = 48;

	// v28: perch state machine. ORBIT -> DESCEND -> PERCHED ->
	// ASCEND -> ORBIT. All steering is steerTo() (the old orbit
	// ease, capped 0.35/tick); only the target point changes.
	enum PerchState { PERCH_ORBIT = 0, PERCH_DESCEND, PERCH_PERCHED, PERCH_ASCEND };
	int perchState;
	int perchTimer; // ticks spent in the current perch state
	static const int PERCH_ORBIT_TICKS = 400;     // ~20 s orbiting between perches
	static const int PERCH_DESCEND_TIMEOUT = 300; // 15 s watchdog: give up, take off
	static const int PERCH_TIME_TICKS = 160;      // ~8 s sitting on the portal
	static const int PERCH_ASCEND_TIMEOUT = 300;  // 15 s watchdog: back to orbit

	// Steer the velocity toward a fixed point (the old orbit
	// steering, factored out so the perch reuses it exactly).
	void steerTo(float tx, float ty, float tz);
};

#endif /*NET_MINECRAFT_WORLD_ENTITY_MONSTER__EnderDragon_H__*/
