#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__RedStoneTorchTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__RedStoneTorchTile_H__

// v11: redstone torch (tile id 76, modern MC id). A normal torch that
// is also a constant signal source: it always powers neighbouring
// wire / lamps / TNT and the block it is attached to
// (getDirectSignal), so a torch on the side of a block powers wire
// lying next to that block.
//
// Always-ON by design for this batch: a torch that turns itself off
// when powered needs to tell "power feeding me" apart from "power I
// am feeding" (a wire it powers would otherwise switch it straight
// back off). No burnout / toggle-off logic yet.
//
// Texture 203 is a red-tipped torch drawn into a row-12 terrain.png
// cell that was an unused magenta placeholder (same trick as v9).
// Light level 7, like modern MC (normal torch is 15).

#include "TorchTile.h"
#include "../Level.h"

class RedStoneTorchTile : public TorchTile
{
	typedef TorchTile super;
public:
	RedStoneTorchTile(int id, int tex)
	:	super(id, tex)
	{
	}

	bool isSignalSource() {
		return true;
	}

	bool getSignal(LevelSource* level, int x, int y, int z) {
		return true;
	}

	bool getSignal(LevelSource* level, int x, int y, int z, int dir) {
		return true;
	}

	bool getDirectSignal(Level* level, int x, int y, int z, int dir) {
		return true;
	}

	void animateTick(Level* level, int x, int y, int z, Random* random) {
		// Redstone sparkles instead of the normal torch's smoke/flame.
		level->addParticle(PARTICLETYPE(reddust),
			x + 0.5f, y + 0.7f, z + 0.5f, 0, 0, 0);
	}
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_TILE__RedStoneTorchTile_H__*/
