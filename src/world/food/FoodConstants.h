#ifndef NET_MINECRAFT_WORLD_FOOD__FoodConstants_H__
#define NET_MINECRAFT_WORLD_FOOD__FoodConstants_H__

//package net.minecraft.world.food;

class FoodConstants
{
public:
    static const int MAX_FOOD = 20;
	// number of game ticks to change health because of food
    static const int HEALTH_TICK_COUNT = 80;
	static const int HEALTH_TICK_COUNT_SIMPLE = 5;
    static const int HEAL_LEVEL = 18;

	// Hunger system (v5): 20 ticks = 1 second.
	// 1 hunger point lost every FOOD_DEPLETE_TICKS (60 s -> full bar lasts ~20 min).
	static const int FOOD_DEPLETE_TICKS = 1200;
	// Starvation: 1 damage every STARVE_TICKS (4 s) while hunger is 0.
	static const int STARVE_TICKS = 80;
	// Regeneration: 1 health every HEAL_TICKS (4 s) while hunger >= HEAL_MIN_FOOD.
	static const int HEAL_TICKS = 80;
	static const int HEAL_MIN_FOOD = 18;
};

#endif /*NET_MINECRAFT_WORLD_FOOD__FoodConstants_H__*/
