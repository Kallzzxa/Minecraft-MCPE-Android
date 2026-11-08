#ifndef NET_MINECRAFT_WORLD_FOOD__SimpleFoodData_H__
#define NET_MINECRAFT_WORLD_FOOD__SimpleFoodData_H__

//package net.minecraft.world.food;

class FoodItem;
class Player;

class SimpleFoodData
{
public:
    SimpleFoodData();

    void eat(int food);
    void eat(FoodItem* item);

    void tick(Player* player);
    // Adds exhaustion from activity (v6); every EXHAUSTION_PER_FOOD
    // drains 1 hunger point (modern Minecraft rule).
    void addExhaustion(float amount);

    int getFoodLevel() const { return foodLevel; }
private:
	int foodLevel;
	float exhaustion; // accumulated activity cost, 0..EXHAUSTION_PER_FOOD
	// Tick counters for the hunger system (20 ticks = 1 second).
	int healTimer;   // counts up to HEAL_TICKS while regenerating
	int starveTimer; // counts up to STARVE_TICKS while starving
};

#endif /*NET_MINECRAFT_WORLD_FOOD__SimpleFoodData_H__*/
