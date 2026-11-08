#if !defined(DEMO_MODE) && !defined(APPLE_DEMO_PROMOTION)

#ifndef NET_MINECRAFT_WORLD_LEVEL_STORAGE__ExternalFileLevelStorage_H__
#define NET_MINECRAFT_WORLD_LEVEL_STORAGE__ExternalFileLevelStorage_H__

//package net.minecraft.world.level.storage;

#include <vector>
#include <list>
#include <stdint.h>

//#include "com/mojang/nbt/CompoundTag.h"
#include "LevelStorage.h"
#include "../chunk/storage/ChunkStorage.h"

class Player;
class Dimension;
class RegionFile;


typedef struct UnsavedLevelChunk
{
	// v17: was "int pos" = x + z * CHUNK_CACHE_WIDTH, which collides
	// for negative / large chunk coordinates. Now the 64-bit ChunkPos
	// hash (see ExternalFileLevelStorage::tick).
	int64_t			pos;
	RakNet::TimeMS	addedToList;
	LevelChunk*		chunk;
} UnsavedLevelChunk;

typedef std::list<UnsavedLevelChunk> UnsavedChunkList;

/*public*/
class ExternalFileLevelStorage:
		public LevelStorage,
		public ChunkStorage
		//public PlayerIO
{
public:
    ExternalFileLevelStorage(const std::string& levelId, const std::string& fullPath);
	virtual ~ExternalFileLevelStorage();

	// v21: the world directory this storage reads/writes
	// (Minecraft needs it to place the Nether's DIM-1 subdir).
	std::string getLevelPath() const { return levelPath; }

	// v21: when true, level.dat is written WITHOUT a Player compound
	// (createTag() instead of createTag(players)). The Nether level
	// uses this: its storage must never capture the player tag, or
	// re-entering the Nether would restore a stale inventory/position
	// over the live player (Level::loadPlayer applies the tag once).
	// The live player state always belongs to the overworld tag.
	void setOmitPlayerFromLevelDat(bool omit) { omitPlayerFromLevelDat = omit; }

    LevelData* prepareLevel(Level* level);

	//throws LevelConflictException
    void checkSession() {}

    ChunkStorage* createChunkStorage(Dimension* dimension) { return this; }

    void saveLevelData(LevelData& levelData, std::vector<Player*>* players);
    // PlayerIO getPlayerIO() { return this; }
	// CompoundTag loadPlayerDataTag(std::string playerName) { return NULL; }

    void closeAll() {}

	static bool readLevelData(const std::string& directory, LevelData& dest);
	static bool readPlayerData(const std::string& filename, LevelData& dest);
	static bool writeLevelData(const std::string& datFileName, LevelData& dest, const std::vector<Player*>* players);
	static void saveLevelData(const std::string& directory, LevelData& levelData, std::vector<Player*>* players);

    int savePendingUnsavedChunks(int maxCount);

	//
	// ChunkStorage methods
	//
	virtual LevelChunk* load(Level* level, int x, int z);
	void save(Level* level, LevelChunk* levelChunk);
	// v16: ChunkCache calls this before deleting an evicted chunk, so
	// the pending-unsaved list never holds a dangling chunk pointer
	// (savePendingUnsavedChunks would otherwise write through it).
	virtual void onChunkEvicted(LevelChunk* levelChunk);
	// @note, loadEntities and saveEntities dont use second parameter
	void loadEntities(Level* level, LevelChunk* levelChunk);
	void saveEntities(Level* level, LevelChunk* levelChunk);
	void saveGame(Level* level);
    void saveAll(Level* level, std::vector<LevelChunk*>& levelChunks);

	virtual void tick();
	virtual void flush() {}
private:
	std::string levelId;
	std::string levelPath;
	LevelData* loadedLevelData;
	RegionFile* regionFile;
	RegionFile* entitiesFile;

	Level* level;
	int tickCount;
	int loadedStorageVersion;
	UnsavedChunkList unsavedChunkList;
	int lastSavedEntitiesTick;
	bool omitPlayerFromLevelDat;
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_STORAGE__ExternalFileLevelStorage_H__*/

#endif /*DEMO_MODE*/
