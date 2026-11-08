#ifndef _MINECRAFT_WORLD_LEVELCONSTANTS_H_
#define _MINECRAFT_WORLD_LEVELCONSTANTS_H_


const int LEVEL_HEIGHT = 128;
const int CHUNK_CACHE_WIDTH = 32; // in chunks (was 16: world 256x256).
// 32 -> world 512x512 blocks, 1024 chunks resident after prepareLevel.
// Per-chunk ~82.7KB (32KB blocks + 3x16KB nibble layers + height/update maps)
// => ~85MB chunk data (was ~21MB). 64 chunks (1024x1024) would need ~339MB
// + 400k renderer VBOs, so 32 is the largest safe step for a phone.
// NOTE: must stay a power of 2 (ChunkCache indexes slots with & (WIDTH-1)).
const int CHUNK_WIDTH = 16; // in blocks
const int CHUNK_DEPTH = 16;
const int LEVEL_WIDTH = CHUNK_CACHE_WIDTH * CHUNK_WIDTH;
const int LEVEL_DEPTH = CHUNK_CACHE_WIDTH * CHUNK_DEPTH;
const int CHUNK_COLUMNS = CHUNK_WIDTH * CHUNK_DEPTH;
const int CHUNK_BLOCK_COUNT = CHUNK_COLUMNS * LEVEL_HEIGHT;

#endif
