#ifndef NET_MINECRAFT_WORLD_LEVEL__ChunkPos_H__
#define NET_MINECRAFT_WORLD_LEVEL__ChunkPos_H__

//package net.minecraft.world.level;

#include <stdint.h>
#include "../entity/Entity.h"

class ChunkPos {
public:
    const int x, z;

	ChunkPos()
	:	x(0),
		z(0)
	{}

    ChunkPos(int x_, int z_)
	:	x(x_),
		z(z_)
	{}

    // v16: 64-bit chunk key — the old key packed 15 bits per axis plus a
    // sign bit, so chunks beyond +-32767 (+-524,288 blocks) COLLIDED onto
    // other chunks' keys. The streaming chunk map (ChunkCache +
    // RandomLevelSource::chunkMap) is keyed by this, so a collision would
    // silently return / delete the WRONG chunk. The full 32 bits of each
    // axis go into the key now: ((int64)x << 32) | (uint32)z. This is also
    // what ChunkPos::operator< orders by, so Level::_chunksToPoll (a
    // std::set<ChunkPos>) no longer merges distinct chunks either.
    static int64_t hashCode(int x, int z) {
        return ( (static_cast<int64_t>(x) << 32) | (uint32_t)z );
    }

    int64_t hashCode() const {
        return hashCode(x, z);
    }

	bool operator==(const ChunkPos& rhs) const {
		return x == rhs.x && z == rhs.z;
	}

	bool operator<(const ChunkPos& rhs) const {
		return hashCode() < rhs.hashCode();
	}

    float distanceToSqr(const Entity* e) const {
        float xPos = (float) (x * 16 + 8);
        float zPos = (float) (z * 16 + 8);

        float xd = xPos - e->x;
        float zd = zPos - e->z;

        return xd * xd + zd * zd;
    }
};

#endif /*NET_MINECRAFT_WORLD_LEVEL__ChunkPos_H__*/
