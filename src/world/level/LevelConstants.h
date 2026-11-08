#ifndef _MINECRAFT_WORLD_LEVELCONSTANTS_H_
#define _MINECRAFT_WORLD_LEVELCONSTANTS_H_


const int LEVEL_HEIGHT = 128;
// v16: CHUNK_CACHE_WIDTH is now the WORLD width in chunks (32 -> world
// 512x512 blocks), NOT the cache size — the cache is a dynamic map
// (ChunkCache) whose resident set is bounded by CHUNK_STREAM_RADIUS
// below. It no longer needs to be a power of 2 (nothing indexes slots
// with & (WIDTH-1) anymore). The world is still 512x512 in v16: the
// RegionFile offset table only holds 32x32 chunks, so the border stays
// until v17 replaces the save format.
// Per-chunk ~82.7KB (32KB blocks + 3x16KB nibble layers + height/update maps)
// => a full 1024-chunk world is ~85MB chunk data.
const int CHUNK_CACHE_WIDTH = 32; // in chunks (was 16: world 256x256).
const int CHUNK_WIDTH = 16; // in blocks
const int CHUNK_DEPTH = 16;
const int LEVEL_WIDTH = CHUNK_CACHE_WIDTH * CHUNK_WIDTH;
const int LEVEL_DEPTH = CHUNK_CACHE_WIDTH * CHUNK_DEPTH;
const int CHUNK_COLUMNS = CHUNK_WIDTH * CHUNK_DEPTH;
const int CHUNK_BLOCK_COUNT = CHUNK_COLUMNS * LEVEL_HEIGHT;

// v16 streaming window (ChunkCache eviction):
// Chunks farther than this (Chebyshev distance, in chunks) from the
// player's chunk are evicted. 16 matches the render grid: the renderer
// builds a 33x33 render-chunk grid around the player (dist 512, see
// LevelRenderer::allChanged), i.e. exactly +-16 chunks, and fog ends at
// 256 blocks, so terrain always exists before the player can reach the
// edge of the resident window. Resident window = 33x33 = 1089 chunks
// max (~85 MiB — the same memory as the old fixed 1024-chunk world,
// the window just moves now).
const int CHUNK_STREAM_RADIUS = 16;
// Eviction runs from ChunkCache::tick(), at most this often (game ticks)
// and evicting at most this many chunks per sweep, so a teleport far
// away drains the trailing edge over a few sweeps instead of stalling
// one tick on dozens of disk writes.
const int CHUNK_STREAM_SWEEP_INTERVAL_TICKS = 10;
const int CHUNK_STREAM_MAX_EVICT_PER_SWEEP = 32;
// Spawn area generated at load (Minecraft::prepareLevel), in chunks
// around the spawn chunk: 19x19 = 361 chunks instead of all 1024.
const int CHUNK_SPAWN_GEN_RADIUS = 9;

#endif
