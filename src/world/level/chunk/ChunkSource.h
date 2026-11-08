#ifndef NET_MINECRAFT_WORLD_LEVEL_CHUNK__ChunkSource_H__
#define NET_MINECRAFT_WORLD_LEVEL_CHUNK__ChunkSource_H__

//package net.minecraft.world.level.chunk;

#include <string>
#include <vector>
#include "../biome/Biome.h"

class ProgressListener;
class LevelChunk;

class ChunkSource
{
public:
	bool isChunkCache;

	ChunkSource()
	:	isChunkCache(false)
	{}

	virtual ~ChunkSource(){}

	virtual bool hasChunk(int x, int y) = 0;

    virtual LevelChunk* getChunk(int x, int z) = 0;

    virtual LevelChunk* create(int x, int z) = 0;

    virtual void postProcess(ChunkSource* parent, int x, int z) = 0;

    //virtual bool save(bool force, ProgressListener* progressListener) = 0;

    virtual bool tick() = 0;

    virtual bool shouldSave() = 0;
	virtual void saveAll(bool onlyUnsaved) {}

	// v16 streaming: drop a chunk from this source's own store and delete
	// it. Returns true if this source owned the chunk (and deleted it) —
	// the caller (ChunkCache) then only erases its own reference and must
	// NOT delete the object again. Returns false if the chunk is not in
	// this source's store (e.g. it was loaded from disk, not generated),
	// in which case the caller owns the delete. Default: not owned.
	virtual bool evictChunk(int x, int z) { return false; }

	// v16 streaming: append every chunk this source currently holds
	// resident (for ChunkCache: the whole map). Used by the storage tick
	// scan so it only looks at resident chunks instead of re-loading the
	// whole world through getChunk() every 50 ticks (which would undo
	// eviction). Default: empty (sources without a resident set).
	virtual void getResidentChunks(std::vector<LevelChunk*>& out) {}

	virtual Biome::MobList getMobsAt(const MobCategory& mobCategory, int x, int y, int z) = 0;

    /**
     * Returns some stats that are rendered when the user holds F3.
     */
    virtual std::string gatherStats() = 0;
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_CHUNK__ChunkSource_H__*/
