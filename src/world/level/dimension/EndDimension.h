#ifndef NET_MINECRAFT_WORLD_LEVEL_DIMENSION__EndDimension_H__
#define NET_MINECRAFT_WORLD_LEVEL_DIMENSION__EndDimension_H__

// v22: the End dimension (id 1, modern MC), reusing the v21 Nether
// pattern: a Dimension subclass + its own ChunkSource, a third Level
// object in Minecraft, storage <world>/DIM1.
//
// Properties, all via the hooks the base class already offers:
//  - foggy: no sky plane / sun / moon / clouds (LevelRenderer skips
//    them when dimension->foggy); fog starts at the camera, so the
//    void reads as dark instead of bright blue.
//  - hasCeiling is FALSE (there is no roof in the End): sky light
//    still reaches the island, so the pale end stone stays visible.
//    Darkness comes from getTimeOfDay fixed at 0.75 (permanent dusk,
//    like the Nether): skyDarken 5, and endermen can spawn.
//  - getFogColor: near-black purple haze instead of overworld blue.
//  - no day/night cycle, no weather (there is no rain system in
//    this tree anyway).

#include "Dimension.h"
#include "../levelgen/EndLevelSource.h"
#include "../Level.h"

class EndDimension: public Dimension {
public:
	EndDimension() {
		id = THE_END;
	}

protected:
	void init() {
		Dimension::init();
		foggy = true;
		ultraWarm = false;
		hasCeiling = false;
	}

public:
	float getTimeOfDay(long time, float a) {
		(void) time; (void) a;
		return 0.75f;
	}

	Vec3 getFogColor(float td, float a) {
		(void) td; (void) a;
		return Vec3(0.05f, 0.0f, 0.09f);
	}

	ChunkSource* createRandomLevelSource() {
		return new EndLevelSource(level, level->getSeed());
	}
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_DIMENSION__EndDimension_H__*/
