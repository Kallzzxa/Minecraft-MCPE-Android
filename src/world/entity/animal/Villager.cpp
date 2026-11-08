#include "Villager.h"
#include "../player/Player.h"

Villager::Villager(Level* level)
:	super(level)
{
	entityRendererId = ER_VILLAGER_RENDERER;

	textureName = "mob/villager.png";
	setSize(0.6f, 1.8f);
}

int Villager::getEntityTypeId() const
{
	return MobTypes::Villager;
}

int Villager::getMaxHealth()
{
	return 10;
}

bool Villager::interact(Player* player)
{
	if (player == NULL) return false;
	player->openTrading(this);
	return true;
}

const char* Villager::getAmbientSound()
{
	return "mob.villager";
}

std::string Villager::getHurtSound()
{
	return "mob.villager";
}

std::string Villager::getDeathSound()
{
	return "mob.villager";
}

int Villager::getExperienceReward(Player* killedBy)
{
	(void) killedBy;
	return 0;
}
