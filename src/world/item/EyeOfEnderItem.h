#ifndef NET_MINECRAFT_WORLD_ITEM__EyeOfEnderItem_H__
#define NET_MINECRAFT_WORLD_ITEM__EyeOfEnderItem_H__

// v22: eye of ender (modern MC id 381). Crafted shapeless from an
// ender pearl + blaze powder (Recipes.cpp — the recipe was commented
// out in this tree since the original; the item itself never
// existed). Its only use in this batch is filling an end portal
// frame: tap a frame tile with an eye to set its eye bit, and when
// all 12 frames of a 5x5 ring (corners empty) have eyes, the 3x3
// interior fills with the end portal tile.
//
// No stronghold exists in this tree (no structure generation beyond
// the Nether reactor and dungeons), so the frame is craftable
// instead: 1 obsidian + 1 eye of ender -> 4 frames (Recipes.cpp).
// The player builds the ring anywhere, fills it, and walks in.
// Throwing the eye to locate a stronghold is NOT implemented (no
// throwable use for the item yet, no stronghold to find) — see
// ROADMAP.

#include "Item.h"
#include "../level/Level.h"
#include "../level/tile/Tile.h"
#include "../entity/player/Player.h"

class EyeOfEnderItem: public Item
{
	typedef Item super;
public:
	EyeOfEnderItem(int id)
	:	super(id)
	{
	}

	bool useOn(ItemInstance* instance, Player* player, Level* level, int x, int y, int z, int face, float clickX, float clickY, float clickZ) {
		(void) player; (void) face; (void) clickX; (void) clickY; (void) clickZ;
		if (level->isClientSide) return false;
		if (level->getTile(x, y, z) != Tile::endPortalFrame->id) return false;
		int data = level->getData(x, y, z);
		if (data & 1) return false; // this frame already has an eye
		level->setData(x, y, z, data | 1);
		instance->count--;
		level->playSound(x + 0.5, y + 0.5, z + 0.5, "random.pop", 1, level->random.nextFloat() * 0.4f + 0.8f);
		tryCompletePortal(level, x, y, z);
		return true;
	}

private:
	// The tapped frame (x, y, z) may belong to a complete 5x5 ring
	// (corners empty). Try every 5x5 square that contains it as an
	// edge cell: all 12 edge cells must be frames with eyes, the 3x3
	// interior must be air. On success, fill the interior with the
	// end portal tile.
	static void tryCompletePortal(Level* level, int x, int y, int z) {
		for (int ox = -4; ox <= 0; ox++) {
			for (int oz = -4; oz <= 0; oz++) {
				int x0 = x + ox, z0 = z + oz;
				// The tapped frame must be on the ring edge.
				int ex = x - x0, ez = z - z0;
				bool onEdge = (ex == 0 || ex == 4 || ez == 0 || ez == 4)
					&& !((ex == 0 || ex == 4) && (ez == 0 || ez == 4));
				if (!onEdge) continue;
				bool ok = true;
				for (int i = 0; i < 5 && ok; i++) {
					for (int j = 0; j < 5 && ok; j++) {
						bool corner = (i == 0 || i == 4) && (j == 0 || j == 4);
						int t = level->getTile(x0 + i, y, z0 + j);
						if (corner) {
							if (t != 0) ok = false;
						} else if (i == 0 || i == 4 || j == 0 || j == 4) {
							// Edge cell: frame with an eye.
							if (t != Tile::endPortalFrame->id
									|| !(level->getData(x0 + i, y, z0 + j) & 1)) {
								ok = false;
							}
						} else {
							// Interior: must be air.
							if (t != 0) ok = false;
						}
					}
				}
				if (!ok) continue;
				for (int i = 1; i <= 3; i++) {
					for (int j = 1; j <= 3; j++) {
						level->setTile(x0 + i, y, z0 + j, Tile::endPortal->id);
					}
				}
				level->playSound(x + 0.5, y + 0.5, z + 0.5, "random.explode", 0.5f, 1.0f);
				return;
			}
		}
	}
};

#endif /*NET_MINECRAFT_WORLD_ITEM__EyeOfEnderItem_H__*/
