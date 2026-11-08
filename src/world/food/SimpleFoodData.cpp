#include "SimpleFoodData.h"
#include "FoodConstants.h"
#include "../item/FoodItem.h"
#include "../entity/player/Player.h"
#include "../Difficulty.h"

SimpleFoodData::SimpleFoodData()
:	foodLevel(FoodConstants::MAX_FOOD),
	exhaustion(0),
	healTimer(0),
	starveTimer(0)
{
}

void SimpleFoodData::eat( int food ) {
	foodLevel = Mth::Min(food + foodLevel, FoodConstants::MAX_FOOD);
}

void SimpleFoodData::eat( FoodItem* item ) {
	eat(item->getNutrition());
}

void SimpleFoodData::addExhaustion(float amount) {
	exhaustion += amount;
	// 4.0 exhaustion drains 1 hunger point (modern Minecraft rule).
	while (exhaustion >= FoodConstants::EXHAUSTION_PER_FOOD) {
		exhaustion -= FoodConstants::EXHAUSTION_PER_FOOD;
		if (foodLevel > 0) foodLevel--;
	}
}

void SimpleFoodData::tick( Player* player ) {
	// Peaceful: hunger stays full, no drain (like modern MCPE).
	if (player->level && player->level->difficulty == Difficulty::PEACEFUL) {
		foodLevel = FoodConstants::MAX_FOOD;
		exhaustion = 0;
		healTimer = starveTimer = 0;
		return;
	}

	// Hunger drain is activity-based only (v6 exhaustion): walking,
	// sprinting and jumping feed addExhaustion() above. Standing still
	// costs nothing, like modern Minecraft. (The old v5 wall-clock
	// drain of 1 point per 60 s is removed.)
	if (foodLevel <= 0) {
		// Starving: lose 1 health every STARVE_TICKS, but never below 1 HP.
		if (player->health > 1 && ++starveTimer >= FoodConstants::STARVE_TICKS) {
			starveTimer = 0;
			player->hurt(NULL, 1);
		}
	}

	// Regeneration: with a nearly full belly, slowly heal at the cost of hunger.
	if (foodLevel >= FoodConstants::HEAL_MIN_FOOD && player->isHurt()) {
		if (++healTimer >= FoodConstants::HEAL_TICKS) {
			healTimer = 0;
			player->heal(1);
			foodLevel--;
		}
	} else {
		healTimer = 0;
	}
}
