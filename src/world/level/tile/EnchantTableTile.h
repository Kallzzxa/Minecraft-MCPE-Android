#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__EnchantTableTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__EnchantTableTile_H__

// v10: enchanting table (tile id 116, matching modern MC). No tile
// entity — the screen enchants the item held in the player's hand, so
// no container storage is needed. Top face reuses the bookshelf
// texture (books on a table), sides/bottom are obsidian.

#include "Tile.h"
#include "../material/Material.h"

class EnchantTableTile: public Tile
{
	typedef Tile super;
public:
    EnchantTableTile(int id);

    int getTexture(int face);
    bool use(Level* level, int x, int y, int z, Player* player);
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_TILE__EnchantTableTile_H__*/
