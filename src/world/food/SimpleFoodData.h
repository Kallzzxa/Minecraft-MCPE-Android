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

    int getFoodLevel() const { return foodLevel; }
private:
	int foodLevel;
	// Tick counters for the hunger system (20 ticks = 1 second).
	int foodTimer;   // counts up to FOOD_DEPLETE_TICKS, then -1 hunger
	int healTimer;   // counts up to HEAL_TICKS while regenerating
	int starveTimer; // counts up to STARVE_TICKS while starving
};

#endif /*NET_MINECRAFT_WORLD_FOOD__SimpleFoodData_H__*/
