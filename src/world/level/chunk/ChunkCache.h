#ifndef NET_MINECRAFT_WORLD_LEVEL_CHUNK__ChunkCache_H__
#define NET_MINECRAFT_WORLD_LEVEL_CHUNK__ChunkCache_H__

//package net.minecraft.world.level.chunk;

// v16 — STREAMING CORE: the fixed 32x32 slot array is gone. Resident
// chunks live in a std::map keyed by the 64-bit ChunkPos hash, and a
// periodic sweep (tick()) evicts chunks farther than
// CHUNK_STREAM_RADIUS (16) chunks from the player chunk, saving them
// first. The ChunkSource interface is unchanged, so Level::getChunk
// callers do not change.
//
// The world is STILL 512x512 in v16 (fits() stays 0..31): the RegionFile
// offset table only holds 32x32 chunks, so chunks outside cannot be
// saved yet — that is v17. What changed is HOW the 512 world is cached
// and streamed: prepareLevel() no longer generates all 1024 chunks at
// load, and the resident window now moves with the player instead of
// being the whole world at once. Worst case memory is unchanged
// (<= 1024 chunks, ~85 MiB), because the world itself is still capped.

#include <map>
#include <vector>
#include <stdint.h>
#include <stdio.h>
#include "ChunkSource.h"
#include "storage/ChunkStorage.h"
#include "EmptyLevelChunk.h"
#include "../Level.h"
#include "../LevelConstants.h"
#include "../ChunkPos.h"

class ChunkCache: public ChunkSource {
    //static const int CHUNK_CACHE_WIDTH = CHUNK_CACHE_WIDTH; // WAS 32;
    static const int MAX_SAVES = 2;
public:
    ChunkCache(Level* level_, ChunkStorage* storage_, ChunkSource* source_)
	:	xLast(-999999999),
		zLast(-999999999),
		last(NULL),
		level(level_),
		storage(storage_),
		source(source_),
		lastEvictTime(0)
	{
		isChunkCache = true;
        //emptyChunk = new EmptyLevelChunk(level_, emptyChunkBlocks, 0, 0);
		emptyChunk = new EmptyLevelChunk(level_, NULL, 0, 0);
    }

	~ChunkCache() {
		// Delete resident chunks FIRST, then the source: the generator's
		// chunkMap holds the same pointers for generated chunks, and its
		// destructor must never run while we still owe it a delete.
		for (ChunkMap::iterator it = chunks.begin(); it != chunks.end(); ++it)
		{
			if (it->second)
			{
				it->second->deleteBlockData();
				delete it->second;
			}
		}
		chunks.clear();
		delete source;
		delete emptyChunk;
	}

    // v16: this is now the WORLD border (0..31 chunks = 512x512 blocks),
    // not the cache size. Outside it getChunk() still returns the shared
    // emptyChunk (air) exactly as before — the border is lifted in v17
    // once RegionFile can save chunks at any coordinate.
    bool fits(int x, int z) {
        return (x >= 0 && z >= 0 && x < CHUNK_CACHE_WIDTH && z < CHUNK_CACHE_WIDTH);
    }

    bool hasChunk(int x, int z) {
		// with a fixed world size, chunks outside the fitting area are always available (emptyChunks)
        if (!fits(x, z)) return true;

        if (x == xLast && z == zLast && last != NULL) {
            return true;
        }
        // Resident iff in the map (and really the chunk asked for — the
        // map key makes this exact, the old slot array could alias).
        ChunkMap::iterator it = chunks.find(ChunkPos::hashCode(x, z));
        return it != chunks.end() && it->second != NULL && it->second->isAt(x, z);
    }

    LevelChunk* create(int x, int z) {
        return getChunk(x, z);
    }

    LevelChunk* getChunk(int x, int z) {
		//static Stopwatch sw;
		//sw.start();

		if (x == xLast && z == zLast && last != NULL) {
            return last;
        }
		if (!fits(x, z)) return emptyChunk;
        //if (!level->isFindingSpawn && !fits(x, z)) return emptyChunk;
        int64_t key = ChunkPos::hashCode(x, z);
        ChunkMap::iterator it = chunks.find(key);
        if (it == chunks.end()) {
            LevelChunk* newChunk = load(x, z);
			bool updateLights = false;
            if (newChunk == NULL) {
                if (source == NULL) {
                    newChunk = emptyChunk;
                } else {
                    newChunk = source->getChunk(x, z);
                }
            } else {
				//return emptyChunk;
				updateLights = true;
            }
            // The shared emptyChunk is never stored in the map: it is not
            // owned by the cache and must never be deleted by eviction
            // (the old slot array stored it and the destructor would have
            // double-deleted it).
            if (newChunk != NULL && newChunk != emptyChunk) {
                chunks[key] = newChunk;
            }
            newChunk->lightLava();

			if (updateLights)
			{
				for (int cx = 0; cx < 16; cx++)
				{
					for (int cz = 0; cz < 16; cz++)
					{
						int height = level->getHeightmap(cx + x * 16, cz + z * 16);
						for (int cy = height; cy >= 0; cy--)
						{
							level->updateLight(LightLayer::Sky, cx + x * 16, cy, cz + z * 16, cx + x * 16, cy, cz + z * 16);
							level->updateLight(LightLayer::Block, cx + x * 16 - 1, cy, cz + z * 16 - 1, cx + x * 16 + 1, cy, cz + z * 16 + 1);
						}
					}
				}
				//level->updateLight(LightLayer::Sky, x * 16, 0, z * 16, x * 16 + 15, 128, z * 16 + 15);
				//level->updateLight(LightLayer::Block, x * 16, 0, z * 16, x * 16 + 15, 128, z * 16 + 15);
			}

            if (newChunk != NULL) {
                newChunk->load();
            }

            if (!newChunk->terrainPopulated && hasChunk(x + 1, z + 1) && hasChunk(x, z + 1) && hasChunk(x + 1, z)) postProcess(this, x, z);
            if (hasChunk(x - 1, z) && !getChunk(x - 1, z)->terrainPopulated && hasChunk(x - 1, z + 1) && hasChunk(x, z + 1) && hasChunk(x - 1, z)) postProcess(this, x - 1, z);
            if (hasChunk(x, z - 1) && !getChunk(x, z - 1)->terrainPopulated && hasChunk(x + 1, z - 1) && hasChunk(x, z - 1) && hasChunk(x + 1, z)) postProcess(this, x, z - 1);
            if (hasChunk(x - 1, z - 1) && !getChunk(x - 1, z - 1)->terrainPopulated && hasChunk(x - 1, z - 1) && hasChunk(x, z - 1) && hasChunk(x - 1, z)) postProcess(this, x - 1, z - 1);

            xLast = x;
            zLast = z;
            last = newChunk;
        } else {
            xLast = x;
            zLast = z;
            last = it->second;
        }

		//sw.stop();
		//sw.printEvery(500000, "ChunkCache::load: ");

        return last;
    }

	Biome::MobList getMobsAt(const MobCategory& mobCategory, int x, int y, int z) {
		return source->getMobsAt(mobCategory, x, y, z);
	}

    void postProcess(ChunkSource* parent, int x, int z) {
		if (!fits(x, z)) return;
        LevelChunk* chunk = getChunk(x, z);
        if (!chunk->terrainPopulated) {
            chunk->terrainPopulated = true;
            if (source != NULL) {
                source->postProcess(parent, x, z);
				chunk->clearUpdateMap();
            }
        }
    }

    //bool save(bool force, ProgressListener progressListener) {
    //    int saves = 0;
    //    int count = 0;
    //    if (progressListener != NULL) {
    //        for (int i = 0; i < chunks.length; i++) {
    //            if (chunks[i] != NULL && chunks[i].shouldSave(force)) {
    //                count++;
    //            }
    //        }
    //    }
    //    int cc = 0;
    //    for (int i = 0; i < chunks.length; i++) {
    //        if (chunks[i] != NULL) {
    //            if (force && !chunks[i].dontSave) saveEntities(chunks[i]);
    //            if (chunks[i].shouldSave(force)) {
    //                save(chunks[i]);
    //                chunks[i].unsaved = false;
    //                if (++saves == MAX_SAVES && !force) return false;
    //                if (progressListener != NULL) {
    //                    if (++cc % 10 == 0) {
    //                        progressListener.progressStagePercentage(cc * 100 / count);
    //                    }
    //                }
    //            }
    //        }
    //    }

    //    if (force) {
    //        if (storage == NULL) return true;
    //        storage.flush();
    //    }
    //    return true;
    //}

    bool tick() {
        if (storage != NULL) storage->tick();
        // v16: streaming sweep — evict chunks the player has left behind.
        // Runs here (once per game tick, throttled inside) rather than on
        // a getChunk() miss so eviction is centred on the PLAYER chunk,
        // not on whatever chunk happened to be requested: requesters
        // include MobSpawner and tile ticks, which can ask for chunks
        // far from the player and would otherwise evict the player's own
        // area.
        evictFarChunks();
        return source->tick();
    }

    bool shouldSave() {
        return true;
    }

    std::string gatherStats() {
        // v16: real resident count, not the hardcoded 1024.
        char buf[64];
        snprintf(buf, sizeof(buf), "ChunkCache: %d", (int)chunks.size());
        return std::string(buf);
    }

    virtual void getResidentChunks(std::vector<LevelChunk*>& out) {
        for (ChunkMap::iterator it = chunks.begin(); it != chunks.end(); ++it) {
            if (it->second != NULL) out.push_back(it->second);
        }
    }

	void saveAll(bool onlyUnsaved) {
		if (storage != NULL) {
			// v16: save the RESIDENT set, not the whole 0..31 world. The
			// old loop called level->getChunk(x, z) for all 1024 chunks,
			// which would re-load every evicted chunk (and, for a new
			// world, generate all of it) on every save. Evicted chunks
			// were already saved when they were evicted, so nothing is
			// lost by skipping them here.
			std::vector<LevelChunk*> resident;
			for (ChunkMap::iterator it = chunks.begin(); it != chunks.end(); ++it) {
				LevelChunk* chunk = it->second;
				if (chunk != NULL && (!onlyUnsaved || chunk->shouldSave(false)))
					resident.push_back(chunk);
			}
			storage->saveAll(level, resident);
		}
	}

    // v16 streaming: evict one resident chunk. Saving is UNCONDITIONAL
    // (same as the old slot-reuse path): a freshly generated chunk starts
    // with unsaved=false, so "save only if dirty" would drop never-saved
    // generated terrain — and regeneration is NOT bit-identical
    // (postProcess uses shared Random state), so the chunk must reach
    // disk exactly as it was. Edits survive because eviction writes the
    // chunk before deleting it; a later visit loads it back from disk.
    void evictChunk(int64_t key, LevelChunk* chunk) {
        if (chunk == NULL || chunk == emptyChunk) return;
        // THE unload-stub hazard (LevelChunk::unload() only sets
        // loaded=false; entity/tile-entity removal inside it is
        // commented out): a chunk is only deleted when it provably holds
        // no live entity and no tile entity. Entities live in TWO places
        // — Level::entities (master list) and the chunk's entityBlocks
        // index — and tile entities live in Level::tileEntities plus the
        // chunk's tileEntities map. Deleting the chunk object while either
        // list still references it would leave dangling pointers (the
        // chunk index for entities, the chunk map for tile entities, and
        // a duplicate tile entity would be created when the chunk reloads
        // and getTileEntity() finds an empty map). So such chunks are
        // PINNED: they stay resident until their entities/tile entities
        // go away through the normal paths (mob death/removal, chest
        // broken). This is bounded in v16: the world is still 512x512
        // (<= 1024 chunks, the same as before), mob caps are tiny
        // (monster 20 + creature 15 + water 10 per level), and tile
        // entities are player-placed and few. v17 MUST add real entity
        // despawn/relocation before lifting the border, or the pinned
        // trail would grow without bound in an infinite world.
        if (chunk->countEntities() > 0) return;
        if (!chunk->getTileEntityMap().empty()) return;

        if (chunk == last) {
            last = NULL;
            xLast = -999999999;
            zLast = -999999999;
        }

        chunk->unload();
        if (storage != NULL) {
            // Drop the storage's pending-unsaved entry for this chunk
            // FIRST: it holds a raw pointer that would dangle after the
            // delete below (savePendingUnsavedChunks would write through
            // it). Then persist blocks + the global entities snapshot,
            // exactly like the old slot-reuse path did.
            storage->onChunkEvicted(chunk);
            save(chunk);
            saveEntities(chunk);
        }
        chunks.erase(key);
        if (source != NULL && source->evictChunk(chunk->x, chunk->z)) {
            // Generated chunk: the generator owned it (chunkMap) and has
            // deleted it. Do NOT delete again.
            return;
        }
        // Disk-loaded chunk: not in the generator's map, the cache owns it.
        chunk->deleteBlockData();
        delete chunk;
    }

    // v16 streaming: drop resident chunks farther than
    // CHUNK_STREAM_RADIUS (Chebyshev) from the player's chunk. Called
    // from tick(); throttled to one sweep per
    // CHUNK_STREAM_SWEEP_INTERVAL_TICKS and capped per sweep so a
    // teleport drains the trailing edge over a few sweeps instead of
    // stalling one tick on dozens of disk writes.
    void evictFarChunks() {
        if (level == NULL || level->players.empty()) return;
        long now = level->getTime();
        if (now - lastEvictTime < CHUNK_STREAM_SWEEP_INTERVAL_TICKS) return;
        lastEvictTime = now;

        // Floor-divide the player position into a chunk (handles
        // negatives correctly for v17; v16 positions are >= 0).
        // (Entity*, not Player*: Player is only forward-declared here.)
        Entity* p = level->players[0];
        int pcx = (int)(p->x / 16.0f);
        if (p->x < 0.0f && p->x != pcx * 16.0f) pcx--;
        int pcz = (int)(p->z / 16.0f);
        if (p->z < 0.0f && p->z != pcz * 16.0f) pcz--;

        // Collect first, evict after: evictChunk() erases from the map
        // and deletes objects, which must not happen mid-iteration.
        std::vector<std::pair<int64_t, LevelChunk*> > far;
        for (ChunkMap::iterator it = chunks.begin(); it != chunks.end(); ++it) {
            LevelChunk* chunk = it->second;
            if (chunk == NULL) continue;
            int dx = chunk->x - pcx;
            if (dx < 0) dx = -dx;
            int dz = chunk->z - pcz;
            if (dz < 0) dz = -dz;
            int dist = dx > dz ? dx : dz; // Chebyshev
            if (dist > CHUNK_STREAM_RADIUS) far.push_back(std::make_pair(it->first, chunk));
        }
        int evicted = 0;
        for (size_t i = 0; i < far.size() && evicted < CHUNK_STREAM_MAX_EVICT_PER_SWEEP; ++i) {
            // evictChunk() may refuse a pinned chunk (entities / tile
            // entities); that still counts as visited but not evicted.
            // Count only real evictions against the cap so a sweep full
            // of pinned chunks does not starve later sweeps.
            size_t before = chunks.size();
            evictChunk(far[i].first, far[i].second);
            if (chunks.size() < before) evicted++;
        }
    }

private:
    LevelChunk* load(int x, int z) {
        if (storage == NULL) return emptyChunk;
		if (!fits(x, z))
		{
			return emptyChunk;
		}
        //try {
            LevelChunk* levelChunk = storage->load(level, x, z);
            if (levelChunk != NULL) {
                levelChunk->lastSaveTime = level->getTime();
            }
            return levelChunk;
        //} catch (Exception e) {
        //    e.printStackTrace();
        //    return emptyChunk;
        //}
    }

    void saveEntities(LevelChunk* levelChunk) {
        if (storage == NULL) return;
        //try {
            storage->saveEntities(level, levelChunk);
        //} catch (Error e) {
        //    e.printStackTrace();
        //}
    }

    void save(LevelChunk* levelChunk) {
        if (storage == NULL) return;
        //try {
            levelChunk->lastSaveTime = level->getTime();
            storage->save(level, levelChunk);
        //} catch (IOException e) {
        //    e.printStackTrace();
        //}
    }

public:
	int xLast;
    int zLast;
private:
	typedef std::map<int64_t, LevelChunk*> ChunkMap;
	//unsigned char emptyChunkBlocks[LevelChunk::ChunkBlockCount];
    LevelChunk* emptyChunk;
    ChunkSource* source;
    ChunkStorage* storage;
    ChunkMap chunks;
    Level* level;
    long lastEvictTime;

    LevelChunk* last;

};

#endif /*NET_MINECRAFT_WORLD_LEVEL_CHUNK__ChunkCache_H__*/
