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
	// Starvation: 1 damage every STARVE_TICKS (4 s) while hunger is 0.
	static const int STARVE_TICKS = 80;
	// Regeneration: 1 health every HEAL_TICKS (4 s) while hunger >= HEAL_MIN_FOOD.
	static const int HEAL_TICKS = 80;
	static const int HEAL_MIN_FOOD = 18;

	// Exhaustion (v6): activity drains hunger instead of a wall-clock timer.
	// 4.0 exhaustion = 1 hunger point (modern Minecraft rule).
	static const float EXHAUSTION_PER_FOOD = 4.0f;
	// Walking: 0.01/block -> 1 hunger per 400 blocks walked.
	static const float EXHAUSTION_WALK_PER_BLOCK = 0.01f;
	// Sprinting: 0.1/block -> 1 hunger per 40 blocks sprinted.
	static const float EXHAUSTION_SPRINT_PER_BLOCK = 0.1f;
	// Jumping: 0.05/jump -> 1 hunger per 80 jumps.
	static const float EXHAUSTION_JUMP = 0.05f;
};

#endif /*NET_MINECRAFT_WORLD_FOOD__FoodConstants_H__*/
