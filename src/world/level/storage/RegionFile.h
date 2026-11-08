#ifndef NET_MINECRAFT_WORLD_LEVEL_STORAGE__RegionFile_H__
#define NET_MINECRAFT_WORLD_LEVEL_STORAGE__RegionFile_H__

#include <map>
#include <string>
#include <stdint.h>
#include "../../../raknet/BitStream.h"

typedef std::map<int, bool> FreeSectorMap;

// v17 — PER-REGION SAVE FILES: the old format kept the whole world's
// chunks in ONE file ("chunks.dat") with a single fixed offsets[1024]
// table (32x32 chunks). readChunk/writeChunk indexed offsets[x + z*32]
// with NO bounds check, so a negative chunk coordinate indexed BEFORE
// the array (heap corruption) and a coordinate >= 32 collided with /
// ran past another chunk's slot. That single table was THE hard blocker
// for an infinite world (see ROADMAP v15).
//
// The on-disk chunk format inside a file is UNCHANGED (4 KiB sectors,
// first sector = 1024-entry offset table, same payload layout), but
// there is now one file per 32x32-chunk REGION, modern-.mca-style:
// "r.<rx>.<rz>.dat" in the level directory, where (rx, rz) is the
// region coordinate (floor-divide the chunk coordinate by 32). A chunk
// at ANY coordinate — negative, or far past 32 — maps to exactly one
// region file and one slot inside it, so saving can no longer corrupt
// memory or collide chunks.
//
// Migration: the legacy single file "chunks.dat" holds exactly region
// (0,0) (chunks 0..31). The first time region (0,0) is opened, if
// "r.0.0.dat" does not exist yet but "chunks.dat" does, it is RENAMED
// to "r.0.0.dat" (same directory, same bytes — the format is identical,
// only the name changes), so old 512x512 worlds keep loading with all
// their chunks intact. If the rename ever fails, the legacy file is
// used as-is for region (0,0) so no data is lost.
//
// Region files are opened lazily and capped (MAX_OPEN_REGIONS): the
// offset table is written through on every writeChunk, and sectorFree
// is rebuilt from it on open, so closing a region loses nothing. The
// cap bounds open file descriptors on long walks (without it, every
// region ever visited would stay open).
class RegionFile
{
public:
	RegionFile(const std::string& basePath);
	virtual ~RegionFile();

	bool open();
	bool readChunk(int x, int z, RakNet::BitStream** destChunkData);
	bool writeChunk(int x, int z, RakNet::BitStream& chunkData);
private:
	struct Region
	{
		FILE* file;
		int* offsets;
		FreeSectorMap sectorFree;
		long lastUsed;
		Region() : file(NULL), offsets(NULL), lastUsed(0) {}
	};

	Region* getRegion(int rx, int rz, bool create);
	bool write(Region* region, int sector, RakNet::BitStream& chunkData);
	void close();
	void closeRegion(Region* region);
	std::string regionFileName(int rx, int rz);

	std::string basePath;
	std::map<int64_t, Region*> regions;
	int* emptyChunk;
	long useCounter;
};


#endif /*NET_MINECRAFT_WORLD_LEVEL_STORAGE__RegionFile_H__*/
