#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS__TradingScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS__TradingScreen_H__

// v23: villager trading screen. Deliberately simpler than the furnace /
// chest screens, following the EnchantScreen (v10) pattern: there is no
// container — the screen trades directly with the player's inventory.
// Six fixed trades (same for every villager in this batch; no
// per-villager professions, no stock limits, no trade XP):
//   20 wheat        -> 1 emerald      (sell farm produce)
//   16 coal         -> 1 emerald      (sell mined coal)
//   1 emerald       -> 3 bread
//   1 emerald       -> 2 cooked beef
//   2 emeralds      -> 1 iron pickaxe
//   3 emeralds      -> 1 diamond sword
// The emerald here is the NEW Item::emeraldGem (v23), NOT the old
// Item::emerald, which is the diamond gem in this tree's naming.
// Tapping a trade checks the inventory, removes the cost, adds the
// result (refunding the cost if the inventory has no room).

#include "../Screen.h"
#include <string>
#include <vector>

class Button;
class Player;
class Villager;

class TradingScreen: public Screen
{
public:
	TradingScreen(Player* player, Villager* villager);
	virtual ~TradingScreen();

	void init();
	void setupPositions();

	void render(int xm, int ym, float a);
	bool renderGameBehind();
	void buttonClicked(Button* button);

private:
	struct Trade {
		int costId;
		int costCount;
		int resultId;
		int resultCount;
		std::string label;
	};

	int countItem(int itemId);
	bool removeItems(int itemId, int count);
	bool giveItem(int itemId, int count);

	Player* player;
	// Stored, never dereferenced after construction: trades are fixed
	// for every villager in this batch, so the screen does not need to
	// touch the entity (it may be killed/despawned while open).
	Villager* villager;
	Button* bTrades[6];
	Button* bClose;
	std::vector<Trade> trades;
	std::string status;
};

#endif /*NET_MINECRAFT_CLIENT_GUI_SCREENS__TradingScreen_H__*/
