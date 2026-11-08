#include "EnchantTableTile.h"
#include "../Level.h"
#include "../../entity/player/Player.h"

// Obsidian texture index in terrain.png (see Tile::obsidian).
#define ENCHANT_TABLE_SIDE_TEX 37
// Bookshelf texture index — reads as books on the table top.
#define ENCHANT_TABLE_TOP_TEX 35

EnchantTableTile::EnchantTableTile(int id)
:   super(id, ENCHANT_TABLE_SIDE_TEX, Material::stone)
{
}

int EnchantTableTile::getTexture(int face)
{
    if (face == 1) return ENCHANT_TABLE_TOP_TEX;
    return tex;
}

bool EnchantTableTile::use(Level* level, int x, int y, int z, Player* player)
{
    if (level->isClientSide)
        return true;

    player->openEnchantTable();
    return true;
}
