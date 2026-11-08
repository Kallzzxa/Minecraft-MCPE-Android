#ifndef NET_MINECRAFT_WORLD_ENTITY_ITEM__EndCrystal_H__
#define NET_MINECRAFT_WORLD_ENTITY_ITEM__EndCrystal_H__

// v27: end crystal. A static floating entity (NOT a mob — same
// Entity-subclass pattern as ExperienceOrb, id 87), placed by the
// EndCrystalItem on top of bedrock / obsidian in the End. It spins
// in place (EndCrystalRenderer), never falls, and explodes when hurt
// (punched or shot — hurt() is the same path Player::attack and
// Arrow use, and isPickable() is what lets the crosshair target it).
//
// Two jobs, both server-side, both in tick():
//  - HEAL: every 10 ticks it heals the nearest live ender dragon
//    within 32 blocks by 1 HP (modern MC: the dragon heals from its
//    nearest crystal). This needs EnderDragon::heal — Mob::heal caps
//    at 20 HP (Mob.cpp), which would make a 200 HP dragon
//    unhealable, so the dragon overrides it.
//  - RESPAWN: every 20 ticks, if no dragon is alive AND the exit
//    portal is present (a dragon was defeated before), it counts the
//    crystals standing on respawn spots — the 10 pillar bedrock caps
//    and the 8 exit-portal bedrock ring cells (see
//    EndLevelSource::isPillarTop / isExitRingCell). At 4+, it
//    consumes those crystals, clears the exit portal back to air and
//    spawns a fresh dragon at full HP (same spawn as
//    Minecraft::ensureEndDragon). Modern MC wants the 4 crystals on
//    the exit portal rim; this build ALSO accepts pillar tops, since
//    the v27 brief specifies pillars and the rim alone would strand
//    the crystals the first fight leaves on the pillars. The exit
//    portal is the "already defeated" flag, so placing 4 crystals
//    during the FIRST fight (no portal yet) does nothing — no
//    premature second dragon.
// A respawned dragon's death runs the same dropDeathLoot: the exit
// portal reactivates and the egg is placed on the rim again (same
// spot, so still at most one egg unless the player took the first).
//
// Save/load: the base Entity save writes Pos/Motion/Rotation + the
// type id, and EntityFactory::CreateEntity rebuilds it — no extra
// state, so no save-data override is needed.

#include "../../entity/Entity.h"
#include "../../entity/EntityTypes.h"

class Level;

class EndCrystal: public Entity
{
	typedef Entity super;
public:
	EndCrystal(Level* level);
	EndCrystal(Level* level, float x, float y, float z);

	void tick();
	bool hurt(Entity* source, int damage);
	bool isPickable();
	int getEntityTypeId() const { return EntityTypes::IdEndCrystal; }

	// Support-block helpers. A crystal floats 1 block above its
	// support, so the support is (floor(x), floor(y)-1, floor(z)).
	static bool isOnRespawnSpot(Level* level, float x, float y, float z);

protected:
	// No extra state beyond the base Entity save (Pos/Motion/
	// Rotation) — both are empty by design.
	void addAdditonalSaveData(CompoundTag* tag);
	void readAdditionalSaveData(CompoundTag* tag);

private:
	void tryHealDragon();
	void tryRespawnDragon();

	bool exploding;
};

#endif /*NET_MINECRAFT_WORLD_ENTITY_ITEM__EndCrystal_H__*/
