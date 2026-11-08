#ifndef NET_MINECRAFT_WORLD_ENTITY_MONSTER__Blaze_H__
#define NET_MINECRAFT_WORLD_ENTITY_MONSTER__Blaze_H__

// v21: blaze (MobTypes::Blaze = 40). A Nether monster (see
// NetherLevelSource::getMobsAt) that shoots fireballs from up to 16
// blocks away, skeleton-style, and falls back to Monster's melee
// attack up close. Fire-immune, 20 HP. Drops 0-1 blaze rod (the
// brewing-stand fuel / powder source, previously reactor-only).
// Sounds are placeholders (no blaze audio files exist in this tree).

#include "Monster.h"

class Blaze: public Monster
{
	typedef Monster super;
public:
	Blaze(Level* level);

	int getMaxHealth() { return 20; }
	int getEntityTypeId() const { return MobTypes::Blaze; }

	const char* getAmbientSound() { return NULL; }
	std::string getHurtSound() { return "random.fizz"; }
	std::string getDeathSound() { return "random.explode"; }

protected:
	void checkHurtTarget(Entity* target, float d);
	void dropDeathLoot();
};

#endif /*NET_MINECRAFT_WORLD_ENTITY_MONSTER__Blaze_H__*/
