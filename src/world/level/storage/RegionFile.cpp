#include "RegionFile.h"
#include "../../../platform/log.h"
#include "FolderMethods.h"

#include <cstdio>

#ifndef WIN32
#include <errno.h>
#endif

const int SECTOR_BYTES = 4096;
const int SECTOR_INTS = SECTOR_BYTES / 4;
const int SECTOR_COLS = 32;
// v17: one region file holds REGION_CHUNKS x REGION_CHUNKS chunks
// (32x32, the same 1024-entry offset table the old single file had).
const int REGION_CHUNKS = 32;
// v17: cap on simultaneously open region files (see RegionFile.h).
// The resident window spans at most 2x2 regions, so 16 is generous.
const int MAX_OPEN_REGIONS = 16;

static const char* const REGION_DAT_NAME = "chunks.dat";

static void logAssert(int actual, int expected) {
	if (actual != expected) {
		LOGI("ERROR: I/O operation failed (%d vs %d)\n", actual, expected);
	}
}

// Floor-divide by REGION_CHUNKS: C++ / truncates toward zero, which
// would put chunk -1 in region 0 (wrong — it belongs to region -1).
static int floorDivRegion(int v) {
	return v >= 0 ? v / REGION_CHUNKS : -((-v + REGION_CHUNKS - 1) / REGION_CHUNKS);
}

static int64_t regionKey(int rx, int rz) {
	return ( (static_cast<int64_t>(rx) << 32) | (uint32_t)rz );
}

RegionFile::RegionFile(const std::string& basePath)
:	basePath(basePath),
	useCounter(0)
{
	emptyChunk = new int[SECTOR_INTS];
	memset(emptyChunk, 0, SECTOR_INTS * sizeof(int));
}

RegionFile::~RegionFile()
{
	close();
	delete [] emptyChunk;
}

std::string RegionFile::regionFileName(int rx, int rz)
{
	char buf[64];
	snprintf(buf, sizeof(buf), "r.%d.%d.dat", rx, rz);
	std::string name = basePath;
	name += "/";
	name += buf;
	return name;
}

bool RegionFile::open()
{
	// v17: region files open lazily per region (getRegion). All open()
	// has to do is make sure the level directory exists.
	if (!createFolderIfNotExists(basePath.c_str())) {
		LOGI("Failed to create world directory for chunk files: %s\n", basePath.c_str());
		return false;
	}
	return true;
}

void RegionFile::closeRegion(Region* region)
{
	if (region == NULL) return;
	if (region->file) {
		fclose(region->file);
		region->file = NULL;
	}
	delete [] region->offsets;
	region->offsets = NULL;
	delete region;
}

void RegionFile::close()
{
	for (std::map<int64_t, Region*>::iterator it = regions.begin(); it != regions.end(); ++it) {
		closeRegion(it->second);
	}
	regions.clear();
}

RegionFile::Region* RegionFile::getRegion(int rx, int rz, bool create)
{
	int64_t key = regionKey(rx, rz);
	std::map<int64_t, Region*>::iterator it = regions.find(key);
	if (it != regions.end()) {
		it->second->lastUsed = ++useCounter;
		return it->second;
	}
	if (!create) return NULL;

	// Cap open regions (LRU): offset tables are written through on
	// every writeChunk and rebuilt from disk on open, so closing a
	// region loses nothing.
	if ((int)regions.size() >= MAX_OPEN_REGIONS) {
		std::map<int64_t, Region*>::iterator lru = regions.end();
		for (std::map<int64_t, Region*>::iterator rit = regions.begin(); rit != regions.end(); ++rit) {
			if (lru == regions.end() || rit->second->lastUsed < lru->second->lastUsed)
				lru = rit;
		}
		if (lru != regions.end()) {
			closeRegion(lru->second);
			regions.erase(lru);
		}
	}

	std::string filename = regionFileName(rx, rz);

	// v17 migration (region (0,0) only): the legacy single file
	// "chunks.dat" holds exactly this region's chunks (0..31). Rename
	// it to the new per-region name so old worlds keep loading. The
	// format is byte-identical, only the name changes. If the rename
	// fails, keep using the legacy file for this region so no data is
	// lost.
	if (rx == 0 && rz == 0 && !exists(filename.c_str())) {
		std::string legacy = basePath;
		legacy += "/";
		legacy += REGION_DAT_NAME;
		if (exists(legacy.c_str())) {
			if (rename(legacy.c_str(), filename.c_str()) == 0) {
				LOGI("Migrated legacy chunks.dat to %s\n", filename.c_str());
			} else {
				LOGI("Could not rename legacy chunks.dat (errno %d), using it as-is for region (0,0)\n", errno);
				filename = legacy;
			}
		}
	}

	Region* region = new Region();
	region->offsets = new int[SECTOR_INTS];
	memset(region->offsets, 0, SECTOR_INTS * sizeof(int));

	// attempt to open file
	region->file = fopen(filename.c_str(), "r+b");
	if (region->file)
	{
		// read offset table
		logAssert(fread(region->offsets, sizeof(int), SECTOR_INTS, region->file), SECTOR_INTS);

		// mark initial sector as blocked
		region->sectorFree[0] = false;

		// setup blocked sectors
		for (int sector = 0; sector < SECTOR_INTS; sector++)
		{
			int offset = region->offsets[sector];
			if (offset)
			{
				int base = offset >> 8;
				int count = offset & 0xff;

				for (int i = 0; i < count; i++)
				{
					region->sectorFree[base + i] = false;
				}
			}
		}
	}
	else
	{
		// new region
		if (!createFolderIfNotExists(basePath.c_str())) {
			LOGI("Failed to create parent directory for chunk file %s\n", filename.c_str());
			delete [] region->offsets;
			delete region;
			return NULL;
		}

		region->file = fopen(filename.c_str(), "w+b");
		if (!region->file)
		{
			LOGI("Failed to create chunk file %s (errno: %d)\n", filename.c_str(), errno);
			delete [] region->offsets;
			delete region;
			return NULL;
		}

		// write sector header (all zeroes)
		logAssert(fwrite(region->offsets, sizeof(int), SECTOR_INTS, region->file), SECTOR_INTS);

		// mark initial sector as blocked
		region->sectorFree[0] = false;
	}

	region->lastUsed = ++useCounter;
	regions[key] = region;
	return region;
}

bool RegionFile::readChunk(int x, int z, RakNet::BitStream** destChunkData)
{
	int rx = floorDivRegion(x);
	int rz = floorDivRegion(z);
	int lx = x - rx * REGION_CHUNKS;
	int lz = z - rz * REGION_CHUNKS;

	Region* region = getRegion(rx, rz, false);
	if (region == NULL) return false;

	int offset = region->offsets[lx + lz * SECTOR_COLS];

	if (offset == 0)
	{
		// not written to file yet
		return false;
	}

	int sectorNum = offset >> 8;

	fseek(region->file, sectorNum * SECTOR_BYTES, SEEK_SET);
	int length = 0;
	fread(&length, sizeof(int), 1, region->file);

	assert(length < ((offset & 0xff) * SECTOR_BYTES));
	length -= sizeof(int);
	if (length <= 0) {
		//*destChunkData = NULL;
		//return false;
	}

	unsigned char* data = new unsigned char[length];
	logAssert(fread(data, 1, length, region->file), length);
	*destChunkData = new RakNet::BitStream(data, length, false);
	//delete [] data;

	return true;
}

bool RegionFile::writeChunk(int x, int z, RakNet::BitStream& chunkData)
{
	int rx = floorDivRegion(x);
	int rz = floorDivRegion(z);
	int lx = x - rx * REGION_CHUNKS;
	int lz = z - rz * REGION_CHUNKS;

	Region* region = getRegion(rx, rz, true);
	if (region == NULL) return false;

	int slot = lx + lz * SECTOR_COLS;
	int size = chunkData.GetNumberOfBytesUsed() + sizeof(int);

	int offset = region->offsets[slot];
	int sectorNum = offset >> 8;
	int sectorCount = offset & 0xff;
	int sectorsNeeded = (size / SECTOR_BYTES) + 1;

	if (sectorsNeeded > 256)
	{
		LOGI("ERROR: Chunk is too big to be saved to file\n");
		return false;
	}

	if (sectorNum != 0 && sectorCount == sectorsNeeded) {
		// the sector can be written on top of its old data
		write(region, sectorNum, chunkData);
	}
	else
	{
		// we need a new location

		// mark old sectors as free
		for (int i = 0; i < sectorCount; i++)
		{
			region->sectorFree[sectorNum + i] = true;
		}

		// find an available slot with enough run length
		int slotPos = 0;
		int runLength = 0;
		bool extendFile = false;
		while (runLength < sectorsNeeded)
		{
			if (region->sectorFree.find(slotPos + runLength) == region->sectorFree.end())
			{
				extendFile = true;
				break;
			}
			if (region->sectorFree[slotPos + runLength] == true)
			{
				runLength++;
			}
			else
			{
				slotPos = slotPos + runLength + 1;
				runLength = 0;
			}
		}

		if (extendFile)
		{
			fseek(region->file, 0, SEEK_END);
			for (int i = 0; i < (sectorsNeeded - runLength); i++)
			{
				fwrite(emptyChunk, sizeof(int), SECTOR_INTS, region->file);
				region->sectorFree[slotPos + i] = true;
			}
		}
		region->offsets[slot] = (slotPos << 8) | sectorsNeeded;
		// mark slots as taken
		for (int i = 0; i < sectorsNeeded; i++)
		{
			region->sectorFree[slotPos + i] = false;
		}

		// write!
		write(region, slotPos, chunkData);

		// write sector data
		fseek(region->file, slot * sizeof(int), SEEK_SET);
		fwrite(&region->offsets[slot], sizeof(int), 1, region->file);
	}


	return true;
}

bool RegionFile::write(Region* region, int sector, RakNet::BitStream& chunkData)
{
	fseek(region->file, sector * SECTOR_BYTES, SEEK_SET);
	//LOGI("writing %d B to sector %d\n", chunkData.GetNumberOfBytesUsed(), sector);
	int size = chunkData.GetNumberOfBytesUsed() + sizeof(int);
	logAssert(fwrite(&size, sizeof(int), 1, region->file), 1);
	logAssert(fwrite(chunkData.GetData(), 1, chunkData.GetNumberOfBytesUsed(), region->file), chunkData.GetNumberOfBytesUsed());

	return true;
}
