#include "SimpleFoodData.h"
#include "FoodConstants.h"
#include "../item/FoodItem.h"
#include "../entity/player/Player.h"
#include "../Difficulty.h"

SimpleFoodData::SimpleFoodData()
:	foodLevel(FoodConstants::MAX_FOOD),
	foodTimer(0),
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

void SimpleFoodData::tick( Player* player ) {
	// Peaceful: hunger stays full, no drain (like modern MCPE).
	if (player->level && player->level->difficulty == Difficulty::PEACEFUL) {
		foodLevel = FoodConstants::MAX_FOOD;
		foodTimer = healTimer = starveTimer = 0;
		return;
	}

	if (foodLevel > 0) {
		// Hunger slowly drains over time: 1 point per FOOD_DEPLETE_TICKS.
		// A full bar (20) lasts about 20 minutes of play.
		if (++foodTimer >= FoodConstants::FOOD_DEPLETE_TICKS) {
			foodTimer = 0;
			foodLevel--;
		}
	} else {
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
