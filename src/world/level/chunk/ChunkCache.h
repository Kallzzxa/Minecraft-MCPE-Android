#ifndef NET_MINECRAFT_WORLD_LEVEL_CHUNK__ChunkCache_H__
#define NET_MINECRAFT_WORLD_LEVEL_CHUNK__ChunkCache_H__

//package net.minecraft.world.level.chunk;

// v17 — BORDER LIFTED: the world is no longer 512x512. fits() (the
// 0..31 world border) is gone: getChunk() now loads or generates a real
// chunk at ANY coordinate, and RegionFile saves it in the per-region
// file for that coordinate. The resident set is still bounded by the
// v16 streaming window (CHUNK_STREAM_RADIUS 16 around the player), so
// memory stays ~85 MiB — the window just moves without limit now.
// Eviction no longer pins chunks with entities / tile entities (that
// pinned trail would grow without bound in an infinite world): entities
// are despawned and tile entities detached + re-attached on reload
// (see evictChunk / reattachTileEntities). The v16 streaming core is
// otherwise unchanged.

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
#include "../tile/entity/TileEntity.h"

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
		// v18: route each generated chunk through source->evictChunk()
		// (which erases it from chunkMap AND deletes it) instead of
		// deleting it directly — the old loop deleted the object while
		// chunkMap still pointed at it (harmless only because the
		// source was deleted immediately after without touching the
		// map). Disk-loaded chunks are not in chunkMap, so
		// evictChunk() returns false for them and the cache deletes
		// them itself, exactly like evictChunk() does.
		for (ChunkMap::iterator it = chunks.begin(); it != chunks.end(); ++it)
		{
			if (it->second)
			{
				if (source != NULL && source->evictChunk(it->second->x, it->second->z)) {
					// Deleted by the generator (removed from chunkMap).
				} else {
					it->second->deleteBlockData();
					delete it->second;
				}
			}
		}
		chunks.clear();
		delete source;
		delete emptyChunk;
	}

    // v17: NO world border anymore — a chunk "fits" at any coordinate
    // (the old 0..31 check was the 512x512 border; RegionFile now saves
    // per-region files, so there is nothing to reject). hasChunk() now
    // means exactly "resident in the cache": the shared emptyChunk is
    // no longer the answer for outside coordinates, getChunk() loads or
    // generates a real chunk there instead.
    bool hasChunk(int x, int z) {
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
        // v17: no fits() border anymore — any coordinate loads from
        // disk or generates, exactly like 0..31 did in v16.
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

            // v17: re-attach tile entities that were detached when this
            // chunk was evicted (their objects live on in
            // Level::tileEntities with their contents; the chunk map is
            // only an index). Must run before postProcess / any
            // getTileEntity() call, or the chunk would create a fresh
            // EMPTY tile entity for the same chest/furnace.
            reattachTileEntities(newChunk);

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

    // v17 streaming: evict one resident chunk. Saving is UNCONDITIONAL
    // (same as the old slot-reuse path): a freshly generated chunk starts
    // with unsaved=false, so "save only if dirty" would drop never-saved
    // generated terrain — and regeneration is NOT bit-identical
    // (postProcess uses shared Random state), so the chunk must reach
    // disk exactly as it was. Edits survive because eviction writes the
    // chunk before deleting it; a later visit loads it back from disk.
    //
    // v17 — NO MORE PINNING (the v16 debt, now paid): in v16 a chunk
    // holding a live entity or a tile entity was never evicted, because
    // LevelChunk::unload() is a stub and deleting the chunk would leave
    // dangling pointers. That was bounded while the world was 512x512
    // (<= 1024 chunks), but in an infinite world the pinned trail
    // would grow without bound as the player walks. So eviction now
    // clears the chunk properly first:
    //  - ENTITIES: every non-player entity in the chunk is despawned
    //    (Level::removeEntity marks it removed; Level::tickEntities
    //    deletes it on the next game tick and removes it from the
    //    chunk index + the level entity list). This is safe because
    //    Entity::save() already refuses removed entities, so a
    //    despawned mob is NOT written back to entities.dat and can
    //    never resurrect / duplicate on reload. Despawn is also the
    //    modern-MC behaviour at distance: hostile mobs already
    //    self-despawn past 96 blocks (Mob::checkDespawn), and passive
    //    animals NEVER self-despawn (Animal::removeWhenFarAway() is
    //    false) — without this they would accumulate forever as new
    //    chunks generate animals via postProcess. Dropped items and XP
    //    orbs despawn the same way (this tree has no per-chunk entity
    //    save — entities.dat is one global snapshot — so keeping them
    //    alive detached would leave them ticking in unloaded chunks;
    //    an item dropped and then left >16 chunks behind is lost,
    //    documented in ROADMAP). The PLAYER chunk is never evicted
    //    (distance 0), but a player entity in the chunk pins it as a
    //    safety net — a player must never be despawned.
    //  - TILE ENTITIES: detached, NOT deleted. Their objects stay alive
    //    in Level::tileEntities (which owns the chest/furnace contents)
    //    and are re-attached to the chunk map when the chunk reloads
    //    (reattachTileEntities). The eviction's saveEntities() call
    //    below writes the detached tile entities to entities.dat, so
    //    contents survive a full save/load cycle too. Empty chests /
    //    idle furnaces report shouldSave()==false and are simply
    //    re-created empty on reload — empty is empty, nothing is lost.
    void evictChunk(int64_t key, LevelChunk* chunk) {
        if (chunk == NULL || chunk == emptyChunk) return;

        std::vector<Entity*> chunkEntities;
        chunk->collectEntities(chunkEntities);
        // Player check FIRST, before touching anything: a player in
        // the chunk pins it (a player must never be despawned).
        for (size_t i = 0; i < chunkEntities.size(); ++i) {
            if (chunkEntities[i] != NULL && chunkEntities[i]->isPlayer()) return;
        }
        for (size_t i = 0; i < chunkEntities.size(); ++i) {
            Entity* e = chunkEntities[i];
            if (e == NULL) continue;
            level->removeEntity(e);
        }
        chunk->detachTileEntities();

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
            // exactly like the old slot-reuse path did. The snapshot
            // still contains the detached tile entities (contents
            // saved) but NOT the just-despawned entities (Entity::save
            // refuses removed entities).
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

    // v17: re-attach tile entities detached by evictChunk() when their
    // chunk loads again. Runs inside getChunk() right after the chunk
    // is loaded, before postProcess or any getTileEntity() call, so the
    // chunk map holds the ORIGINAL tile entity (with its contents)
    // instead of a fresh empty one. The hasTileEntityAt() guard makes a
    // duplicate attach impossible (e.g. a tile entity loaded from
    // entities.dat at level start for the same position).
    void reattachTileEntities(LevelChunk* chunk) {
        if (chunk == NULL || chunk == emptyChunk || level == NULL) return;
        for (size_t i = 0; i < level->tileEntities.size(); ++i) {
            TileEntity* te = level->tileEntities[i];
            if (te == NULL || te->isRemoved()) continue;
            // (x >> 4) is floor-division by 16, matching the chunk the
            // tile entity belongs to for negative coordinates too.
            if ((te->x >> 4) != chunk->x || (te->z >> 4) != chunk->z) continue;
            if (chunk->hasTileEntityAt(te)) continue;
            chunk->setTileEntity(te->x & 15, te->y, te->z & 15, te);
        }
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
            // evictChunk() may refuse a chunk holding a player entity;
            // that still counts as visited but not evicted. Count only
            // real evictions against the cap so a sweep full of refused
            // chunks does not starve later sweeps.
            size_t before = chunks.size();
            evictChunk(far[i].first, far[i].second);
            if (chunks.size() < before) evicted++;
        }

        // v18: pre-generate the ring around the player chunk (see
        // pregenerateNearChunks). Runs on the same 10-tick sweep as
        // eviction, after it — the two windows are disjoint (pre-gen
        // radius 3, eviction radius 16), so pre-gen can never resurrect
        // a chunk this sweep just evicted.
        pregenerateNearChunks(pcx, pcz);
    }

    // v18: generate the chunks around the player chunk ahead of time,
    // a few per sweep, nearest ring first. Verified need (see ROADMAP
    // v18): the player can never physically walk into air — collision
    // (Level::getCubes -> getTile -> getChunk) and the renderer
    // (Chunk::rebuild reads through Region, whose constructor calls
    // level->getChunk) both generate a chunk synchronously on demand,
    // and updateDirtyChunks() rebuilds every render chunk within 32
    // blocks of the player every frame. What pre-gen fixes is the
    // COST of that on-demand generation landing inside the render
    // pass: walking into new terrain used to burst several chunk
    // generations (Perlin + features + mob spawns) into one frame's
    // near-chunk rebuilds, and far render chunks then queued behind
    // the 3-per-frame rebuild budget. Generating up to
    // CHUNK_PREGEN_MAX_PER_SWEEP missing chunks per sweep, in the game
    // tick, spreads the same work out so the frame stays smooth.
    // Skipped when there is no generator (MP client cache, source ==
    // NULL): getChunk() there would only return the shared emptyChunk
    // and hasChunk() would stay false, so the ring would retry every
    // sweep for nothing.
    void pregenerateNearChunks(int pcx, int pcz) {
        if (source == NULL) return;
        int generated = 0;
        for (int ring = 0; ring <= CHUNK_PREGEN_RADIUS && generated < CHUNK_PREGEN_MAX_PER_SWEEP; ++ring) {
            for (int dx = -ring; dx <= ring && generated < CHUNK_PREGEN_MAX_PER_SWEEP; ++dx) {
                for (int dz = -ring; dz <= ring && generated < CHUNK_PREGEN_MAX_PER_SWEEP; ++dz) {
                    // Inner rings were already covered by smaller `ring`
                    // values — only the perimeter of this ring is new.
                    if (dx > -ring && dx < ring && dz > -ring && dz < ring) continue;
                    int cx = pcx + dx;
                    int cz = pcz + dz;
                    if (hasChunk(cx, cz)) continue;
                    getChunk(cx, cz);
                    ++generated;
                }
            }
        }
    }

private:
    LevelChunk* load(int x, int z) {
        if (storage == NULL) return emptyChunk;
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
