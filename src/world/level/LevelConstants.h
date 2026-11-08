#ifndef _MINECRAFT_WORLD_LEVELCONSTANTS_H_
#define _MINECRAFT_WORLD_LEVELCONSTANTS_H_


const int LEVEL_HEIGHT = 128;
// v17: CHUNK_CACHE_WIDTH / LEVEL_WIDTH / LEVEL_DEPTH no longer cap the
// world — the world is infinite now (RegionFile saves per-region files,
// ChunkCache has no border). These constants survive only as the
// DEFAULT SPAWN CENTRE: a new world spawns at chunk (16, 16) = block
// (256, 256), the old world centre, and prepareLevel generates around
// it. Nothing indexes slots with & (WIDTH-1) anymore.
// Per-chunk ~82.7KB (32KB blocks + 3x16KB nibble layers + height/update maps)
// => the old full 1024-chunk world was ~85MB chunk data; the streaming
// resident window (33x33 chunks) is the same order of magnitude.
const int CHUNK_CACHE_WIDTH = 32; // in chunks (legacy world width; spawn centre only).
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

// v18 pre-generation ring (ChunkCache::pregenerateNearChunks): chunks
// this close to the player chunk are generated ahead of time, a few per
// eviction sweep, so the terrain the player is about to walk into
// already exists before the renderer asks for it. Radius 3 covers the
// renderer's near-rebuild band (32 blocks = 2 chunks) plus one chunk of
// margin for sprinting/diagonal movement; the cap spreads the work so a
// teleport fills the ring over a few sweeps instead of stalling one
// tick. The renderer still generates on demand (Chunk::rebuild reads
// through Region -> Level::getTile -> getChunk), so a chunk that is not
// pre-generated yet can never show as a hole — pre-gen only moves the
// generation cost out of the render pass and into the game tick.
const int CHUNK_PREGEN_RADIUS = 3;
const int CHUNK_PREGEN_MAX_PER_SWEEP = 4;

#endif
