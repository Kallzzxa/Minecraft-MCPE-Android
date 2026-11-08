#ifndef NET_MINECRAFT_WORLD_ENTITY_MONSTER__Ghast_H__
#define NET_MINECRAFT_WORLD_ENTITY_MONSTER__Ghast_H__

// v21: ghast (MobTypes::Ghast = 39). A 4x4x4 flying monster that
// hovers (FlyingMob-style travel: no gravity) and shoots fireballs at
// the player from up to 28 blocks away, old-AI style like the
// skeleton (PathfinderMob chase + checkHurtTarget). Spawns only in
// the Nether (NetherLevelSource::getMobsAt). Drops 0-1 ghast tear +
// 0-2 gunpowder. Sounds are placeholders (no ghast audio files exist
// in this tree; v19 left the enderman silent rather than faking it —
// the ghast keeps silent ambient, pop for hurt, explode for death).

#include "Monster.h"

class Ghast: public Monster
{
	typedef Monster super;
public:
	Ghast(Level* level);

	int getMaxHealth() { return 10; }
	int getEntityTypeId() const { return MobTypes::Ghast; }
	int getMaxSpawnClusterSize() { return 1; }

	void tick();
	void travel(float xa, float ya);

	const char* getAmbientSound() { return NULL; }
	std::string getHurtSound() { return "random.pop"; }
	std::string getDeathSound() { return "random.explode"; }

protected:
	void checkHurtTarget(Entity* target, float d);
	void dropDeathLoot();
};

#endif /*NET_MINECRAFT_WORLD_ENTITY_MONSTER__Ghast_H__*/
