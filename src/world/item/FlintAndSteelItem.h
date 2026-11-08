#ifndef NET_MINECRAFT_WORLD_ITEM__FlintAndSteelItem_H__
#define NET_MINECRAFT_WORLD_ITEM__FlintAndSteelItem_H__

//package net.minecraft.world.item;

#include "Item.h"
#include "../level/Level.h"
#include "../level/tile/Tile.h"
#include "../level/tile/FireTile.h"
#include "../entity/player/Player.h"

class FlintAndSteelItem: public Item
{
    typedef Item super;
public:
    FlintAndSteelItem(int id)
    :   super(id)
    {
		setMaxStackSize(1);
		setMaxDamage(64);
    }

    // v21: the original useOn (place fire) was commented out in this
    // tree, so flint & steel did nothing at all. It is restored here
    // and extended: using it on an obsidian frame lights a Nether
    // portal (see tryLightPortal); otherwise it places fire, as
    // before. World changes only run server-side; in multiplayer the
    // server applies them from its own useOn call.
    bool useOn(ItemInstance* instance, Player* player, Level* level, int x, int y, int z, int face, float clickX, float clickY, float clickZ) {
        (void) player; (void) clickX; (void) clickY; (void) clickZ;
        if (face == 0) y--;
        if (face == 1) y++;
        if (face == 2) z--;
        if (face == 3) z++;
        if (face == 4) x--;
        if (face == 5) x++;

        if (!level->isClientSide) {
            if (tryLightPortal(level, x, y, z)) {
                level->playSound(x + 0.5, y + 0.5, z + 0.5, "fire.ignite", 1, level->random.nextFloat() * 0.4f + 0.8f);
                instance->hurt(1);
                return true;
            }

            int targetType = level->getTile(x, y, z);
            if (targetType == 0) {
                level->playSound(x + 0.5, y + 0.5, z + 0.5, "fire.ignite", 1, level->random.nextFloat() * 0.4f + 0.8f);
                level->setTile(x, y, z, Tile::fire->id);
            }
        }

        instance->hurt(1);
        return true;
    }

private:
    // Try to light a Nether portal whose interior contains (x, y, z).
    // For each plane (X or Z), each frame size (w 4..23, h 5..23,
    // modern MC limits) and each frame origin that puts the clicked
    // cell inside the interior: the border must be all obsidian and
    // the interior all air/fire. Fills the interior with portal
    // blocks on the first match. The scan is skipped outright unless
    // an obsidian block is within 2 blocks of the clicked cell, so an
    // ordinary fire placement on grass costs one small neighbourhood
    // check instead of a full frame search.
    bool tryLightPortal(Level* level, int x, int y, int z) {
        bool nearObsidian = false;
        for (int dx = -2; dx <= 2 && !nearObsidian; dx++)
        for (int dy = -2; dy <= 2 && !nearObsidian; dy++)
        for (int dz = -2; dz <= 2 && !nearObsidian; dz++) {
            if (level->getTile(x + dx, y + dy, z + dz) == Tile::obsidian->id)
                nearObsidian = true;
        }
        if (!nearObsidian) return false;

        for (int axis = 0; axis < 2; axis++) {
            for (int w = 4; w <= 23; w++) {
                for (int h = 5; h <= 23; h++) {
                    // Frame origin ranges that put (x,y,z) inside the
                    // interior: interior spans [b+1, b+w-2] on the
                    // frame axis and [by+1, by+h-2] vertically.
                    int c0 = (axis == 0) ? x : z;
                    int fixed = (axis == 0) ? z : x;
                    for (int b = c0 - w + 2; b <= c0 - 1; b++) {
                        for (int by = y - h + 2; by <= y - 1; by++) {
                            if (isPortalFrame(level, axis, b, by, fixed, w, h)) {
                                fillPortal(level, axis, b, by, fixed, w, h);
                                return true;
                            }
                        }
                    }
                }
            }
        }
        return false;
    }

    // Frame at fixed z (axis 0) or fixed x (axis 1): b/by are the
    // frame's min corner on its plane axis / vertically, c is the
    // fixed coordinate.
    bool isPortalFrame(Level* level, int axis, int b, int by, int c, int w, int h) {
        for (int i = 0; i < w; i++) {
            if (!isObsidianAt(level, axis, b + i, by, c)) return false;
            if (!isObsidianAt(level, axis, b + i, by + h - 1, c)) return false;
        }
        for (int j = 1; j < h - 1; j++) {
            if (!isObsidianAt(level, axis, b, by + j, c)) return false;
            if (!isObsidianAt(level, axis, b + w - 1, by + j, c)) return false;
        }
        for (int i = 1; i < w - 1; i++) {
            for (int j = 1; j < h - 1; j++) {
                int t = tileAt(level, axis, b + i, by + j, c);
                if (t != 0 && t != Tile::fire->id) return false;
            }
        }
        return true;
    }

    void fillPortal(Level* level, int axis, int b, int by, int c, int w, int h) {
        for (int i = 1; i < w - 1; i++) {
            for (int j = 1; j < h - 1; j++) {
                setTileAt(level, axis, b + i, by + j, c, Tile::portalTile->id);
            }
        }
    }

    bool isObsidianAt(Level* level, int axis, int a, int y, int c) {
        return tileAt(level, axis, a, y, c) == Tile::obsidian->id;
    }

    int tileAt(Level* level, int axis, int a, int y, int c) {
        if (axis == 0) return level->getTile(a, y, c);
        return level->getTile(c, y, a);
    }

    void setTileAt(Level* level, int axis, int a, int y, int c, int tile) {
        if (axis == 0) level->setTile(a, y, c, tile);
        else level->setTile(c, y, a, tile);
    }
};

#endif /*NET_MINECRAFT_WORLD_ITEM__FlintAndSteelItem_H__*/
