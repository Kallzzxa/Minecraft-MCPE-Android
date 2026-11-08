#ifndef NET_MINECRAFT_WORLD_LEVEL_DIMENSION__NetherDimension_H__
#define NET_MINECRAFT_WORLD_LEVEL_DIMENSION__NetherDimension_H__

// v21: the Nether dimension (id -1, modern MC).
//
// The Dimension framework already existed (Dimension base +
// NormalDayCycleDimension); what was missing was a Nether subclass, a
// generator (NetherLevelSource), and any code that switches levels
// (Minecraft::toggleDimension was an empty stub, Player::dimension was
// saved to NBT but never used to pick a level).
//
// Properties, all via the hooks the base class already offers:
//  - hasCeiling: no sky light, no sun/moon/clouds (LevelRenderer skips
//    them when dimension->foggy; Level/LevelChunk skip sky light when
//    hasCeiling). The ceiling is real bedrock at y=127 (generator).
//  - foggy: fog starts at the camera (GameRenderer) + no sky plane.
//  - ultraWarm: lava flows slower / water evaporates (LiquidTileDynamic
//    already branches on it); there is no rain system in this tree, so
//    "no rain" needs no extra code.
//  - getTimeOfDay fixed at 0.75: permanent night. That makes isDay()
//    false (skyDarken 5, see Level::getSkyDarken), so the v19 music
//    scheduler plays night crickets here and zombies never burn from
//    the sky (they cannot see it anyway, hasCeiling). There is no
//    day/night cycle in the Nether, matching modern MC.
//  - getFogColor: dark red haze instead of the overworld blue.

#include "Dimension.h"
#include "../levelgen/NetherLevelSource.h"
#include "../Level.h"

class NetherDimension: public Dimension {
public:
	NetherDimension() {
		id = NETHER;
	}

protected:
	void init() {
		Dimension::init();
		foggy = true;
		ultraWarm = true;
		hasCeiling = true;
	}

public:
	float getTimeOfDay(long time, float a) {
		(void) time; (void) a;
		return 0.75f;
	}

	Vec3 getFogColor(float td, float a) {
		(void) td; (void) a;
		return Vec3(0.22f, 0.03f, 0.03f);
	}

	ChunkSource* createRandomLevelSource() {
		return new NetherLevelSource(level, level->getSeed());
	}
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_DIMENSION__NetherDimension_H__*/
