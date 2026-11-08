#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS__EnchantScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS__EnchantScreen_H__

// v10: enchanting-table screen. Deliberately simpler than the furnace /
// chest screens: there is no container slot — the screen enchants the
// item held in the player's hand. It shows up to 3 random offers (one
// per enchantment the held item can take), each costing 1-3 whole XP
// levels; tapping one enchants the held item and spends the levels.
// No lapis, no anvil, no enchanted books in this batch.

#include "../Screen.h"
#include <string>
#include <vector>

class Button;
class Player;
class ItemInstance;

class EnchantScreen: public Screen
{
public:
	EnchantScreen(Player* player);
	virtual ~EnchantScreen();

	void init();
	void setupPositions();

	void render(int xm, int ym, float a);
	bool renderGameBehind();
	void buttonClicked(Button* button);

private:
	struct Offer {
		int enchantmentId;
		int level;
		int costLevels;
		std::string label;
	};

	void generateOffers();
	ItemInstance* getHeldItem();

	Player* player;
	Button* bOptions[3];
	Button* bClose;
	std::vector<Offer> offers;
	std::string status;
};

#endif /*NET_MINECRAFT_CLIENT_GUI_SCREENS__EnchantScreen_H__*/
