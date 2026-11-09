#include "Minecraft.h"

#if defined(APPLE_DEMO_PROMOTION)
    #define NO_NETWORK
#endif

#if defined(RPI)
	#define CREATORMODE
#endif

#include "../network/RakNetInstance.h"
#include "../network/ClientSideNetworkHandler.h"
#include "../network/ServerSideNetworkHandler.h"
//#include "../network/Packet.h"
#include "../world/entity/player/Inventory.h"
#include "../world/level/chunk/ChunkCache.h"
#include "../world/level/tile/Tile.h"
#include "../world/level/storage/LevelStorageSource.h"
#include "../world/level/storage/LevelStorage.h"
#include "../world/level/storage/ExternalFileLevelStorage.h"
#include "../world/level/storage/FolderMethods.h"
#include "../world/level/dimension/NetherDimension.h"
#include "../world/level/dimension/EndDimension.h"
#include "../world/entity/monster/EnderDragon.h"
#include "../world/entity/item/EndCrystal.h"
#include "../world/level/levelgen/EndLevelSource.h"
#include "../world/level/tile/FireTile.h"
#include "player/input/KeyboardInput.h"
#ifndef STANDALONE_SERVER
#include "player/input/touchscreen/TouchInputHolder.h"
#endif
#include "player/LocalPlayer.h"
#include "gamemode/CreativeMode.h"
#include "gamemode/SurvivalMode.h"
#include "player/LocalPlayer.h"
#ifndef STANDALONE_SERVER
#include "particle/ParticleEngine.h"
#include "gui/Screen.h"
#include "gui/Font.h"
#include "gui/screens/RenameMPLevelScreen.h"
#include "sound/SoundEngine.h"
#endif
#include "../platform/CThread.h"
#include "../platform/input/Mouse.h"
#include "../platform/input/Controller.h"
#include "../AppPlatform.h"

#ifdef _UWP
#include "../AppPlatform_uwp.h"
#endif

#include "../Performance.h"
#include "../LicenseCodes.h"
#include "../util/PerfTimer.h"
#include "../util/PerfRenderer.h"
#include "player/input/MouseBuildInput.h"

#include "../world/Facing.h"

#include "../network/packet/PlaceBlockPacket.h"

#include "player/input/IInputHolder.h"
#ifndef STANDALONE_SERVER
#include "player/input/touchscreen/TouchscreenInput.h"

#include "player/input/ControllerTurnInput.h"
#include "player/input/XperiaPlayInput.h"

#endif

#include "player/input/MouseTurnInput.h"
#include "../world/entity/MobFactory.h"
#include "../world/level/MobSpawner.h"
#include "../util/Mth.h"
#include "../network/packet/InteractPacket.h"
#ifndef STANDALONE_SERVER
#include "gui/screens/PrerenderTilesScreen.h"
#include "renderer/Textures.h"
#include "gui/screens/DeathScreen.h"
#endif

#include "../network/packet/RespawnPacket.h"
#include "IConfigListener.h"
#include "../world/entity/MobCategory.h"
#ifndef STANDALONE_SERVER
#include "gui/screens/FurnaceScreen.h"
#endif
#include "../world/Difficulty.h"
#include "../server/ServerLevel.h"
#ifdef CREATORMODE
#include "../server/CreatorLevel.h"
#endif
#include "../network/packet/AdventureSettingsPacket.h"
#include "../network/packet/SetSpawnPositionPacket.h"
#include "../network/command/CommandServer.h"
#include "gamemode/CreatorMode.h"
#ifndef STANDALONE_SERVER
#include "gui/screens/ArmorScreen.h"
#endif
#include "../world/level/levelgen/synth/ImprovedNoise.h"
#ifndef STANDALONE_SERVER
#include "renderer/tileentity/TileEntityRenderDispatcher.h"
#endif

#ifndef STANDALONE_SERVER
#include "renderer/ptexture/DynamicTexture.h"
#include "renderer/GameRenderer.h"
#include "renderer/ItemInHandRenderer.h"
#include "renderer/LevelRenderer.h"
#include "renderer/entity/EntityRenderDispatcher.h"
#include "gui/Screen.h"
#include "gui/Font.h"
#include "gui/screens/RenameMPLevelScreen.h"
#include "sound/SoundEngine.h"
#endif

static void checkGlError(const char* tag) {
#ifdef GLDEBUG
	while (1) {
        const int errCode = glGetError();
        if (errCode == GL_NO_ERROR) break;

		LOGE("################\nOpenGL-error @ %s : #%d\n", tag, errCode);
	}
#endif /*GLDEBUG*/
}

/*static*/
const char* Minecraft::progressMessages[] = {
	"Locating server",
	"Building terrain",
	"Preparing",
	"Saving chunks"
};

int Minecraft::customDebugId = Minecraft::CDI_NONE;

#if defined(_MSC_VER)
	#pragma warning( disable : 4355 ) // 'this' pointer in initialization list which is perfectly legal
#endif

bool Minecraft::useAmbientOcclusion = false;

Minecraft::Minecraft()
:	user(NULL),
	level(NULL),
	overworldLevel(NULL),
	netherLevel(NULL),
	endLevel(NULL),
	pendingDimensionSwitch(false),
	pendingDimension(0),
	hasReturnPos(false),
	returnX(0), returnY(0), returnZ(0),
	portalCooldownTicks(0),
	pendingPortalBuild(false),
	pendingEndDragon(false),
	pendingRespawnAfterSwitch(false),
	ticksSincePlayerPersist(0),
	player(NULL),
	cameraTargetPlayer(NULL),
	levelRenderer(NULL),
	gameRenderer(NULL),
#ifndef STANDALONE_SERVER
	particleEngine(NULL),
	_perfRenderer(NULL),
#endif
	_commandServer(NULL),
#ifndef STANDALONE_SERVER
	textures(NULL),
#endif
	lastTickTime(-1),
	lastTime(0),
	ticksSinceLastUpdate(0),
	gameMode(NULL),
	mouseGrabbed(false),
	missTime(0),
	pause(false),
	_running(false),
	timer(20),
#ifndef STANDALONE_SERVER
	gui(this),
#endif
	netCallback(NULL),
#ifndef STANDALONE_SERVER
	screen(NULL),
	font(NULL),
#endif
	screenMutex(false),
#ifndef STANDALONE_SERVER
	scheduledScreen(NULL),
	hasScheduledScreen(false),
	soundEngine(NULL),
#endif
	ticks(0),
	isGeneratingLevel(false),
	_hasSignaledGeneratingLevelFinished(true),
	generateLevelThread(NULL),
	progressStagePercentage(0),
	progressStageStatusId(0),
	isLookingForMultiplayer(false),
	_licenseId(LicenseCodes::WAIT_PLATFORM_NOT_READY),
	inputHolder(0),
	_supportsNonTouchscreen(false),
#ifndef STANDALONE_SERVER
	screenChooser(this),
#endif
	width(1), height(1),
	//_respawnPlayerTicks(-1),
#ifdef __APPLE__
    _isSuperFast(false),
#endif
	_powerVr(false),
	commandPort(4711),
	reserved_d1(0),reserved_d2(0),
	reserved_f1(0),reserved_f2(0)
{
//#ifdef ANDROID

#if defined(NO_NETWORK)
    raknetInstance = new IRakNetInstance();
#else
	raknetInstance = new RakNetInstance();
#endif
#ifndef STANDALONE_SERVER
	soundEngine = new SoundEngine(20.0f);
	soundEngine->init(this, &options);
#endif
	//setupPieces();
}

Minecraft::~Minecraft()
{
	delete netCallback;
	delete raknetInstance;
#ifndef STANDALONE_SERVER
	delete levelRenderer;
	delete gameRenderer;
	delete particleEngine;

	delete soundEngine;
#endif
	delete gameMode;
#ifndef STANDALONE_SERVER
	delete font;
	delete textures;

	if (screen != NULL) {
		delete screen;
		screen = NULL;
	}
#endif
	if (level != NULL) {
		Level* deletedLevel = level;
		level->saveGame();
		if (level->getChunkSource())
			level->getChunkSource()->saveAll(true);
		delete level->getLevelStorage();
		delete level;
		level = NULL;
		// v21: the inactive dimension level is owned here too.
		if (deletedLevel == overworldLevel) overworldLevel = NULL;
		if (deletedLevel == netherLevel) netherLevel = NULL;
		if (deletedLevel == endLevel) endLevel = NULL;
	}
	if (overworldLevel != NULL) {
		delete overworldLevel->getLevelStorage();
		delete overworldLevel;
		overworldLevel = NULL;
	}
	if (netherLevel != NULL) {
		delete netherLevel->getLevelStorage();
		delete netherLevel;
		netherLevel = NULL;
	}
	if (endLevel != NULL) {
		delete endLevel->getLevelStorage();
		delete endLevel;
		endLevel = NULL;
	}

	//delete player;
	delete user;
	delete inputHolder;

	delete storageSource;
	delete _perfRenderer;
	delete _commandServer;

	MobFactory::clearStaticTestMobs();
#ifndef STANDALONE_SERVER
	EntityRenderDispatcher::destroy();
#endif
}

// Only called by server
void Minecraft::selectLevel( const std::string& levelId, const std::string& levelName, const LevelSettings& settings )
{
#if defined(CREATORMODE)
	level = new CreatorLevel(
#else
	level = new ServerLevel(
#endif
		storageSource->selectLevel(levelId, false),
		levelName,
		settings,
		SharedConstants::GeneratorVersion);

	// note: settings is useless beyond this point, since it's
	//       either copied to LevelData (or LevelData read from file)
	setLevel(level, "Generating level");
	// v21: this is the overworld level; the Nether level is created
	// on first entry and tracked separately (see changeDimension).
	// v22: same for the End level.
	overworldLevel = level;
	netherLevel = NULL;
	endLevel = NULL;
	// v22: grace period after loading: a player reloaded while
	// standing inside a portal (e.g. quit in the End, saved at the
	// return position inside the end portal frame) must NOT be
	// bounced straight back through it — the enter-only detection
	// in LocalPlayer::tick needs a few seconds to latch
	// _wasInPortal/_wasInEndPortal first.
	portalCooldownTicks = 100;
	currentLevelId = levelId;
	setIsCreativeMode(level->getLevelData()->getGameType() == GameType::Creative);
	_running = true;
}

void Minecraft::setLevel(Level* level, const std::string& message /* ="" */, LocalPlayer* forceInsertPlayer /* = NULL */) {
	cameraTargetPlayer = NULL;
	LOGI("Seed is %ld\n", level->getSeed());

	if (level != NULL) {
		level->raknetInstance = raknetInstance;
        gameMode->initLevel(level);

		if (!player && forceInsertPlayer)
		{
			player = forceInsertPlayer;
			player->resetPos(false);
			//level->addEntity(forceInsertPlayer);
		}
		else if (player != NULL) {
			player->resetPos(false);
			if (level != NULL) {
				level->addEntity(player);
			}
		}
		this->level = level;
		_hasSignaledGeneratingLevelFinished = false;
#ifdef STANDALONE_SERVER
		const bool threadedLevelCreation = false;
#else
		const bool threadedLevelCreation = true;
#endif

		if (threadedLevelCreation) {
			// Threaded
			// "Lock"
			isGeneratingLevel = true;
			generateLevelThread = new CThread(Minecraft::prepareLevel_tspawn, this);
		} else {
			// Non-threaded
			generateLevel("Currently not used", level);
		}
    } else {
        player = NULL;
    }

    this->lastTickTime = 0;
	this->_running = true;
}

// ------------------------------------------------------------------
// v21: Nether dimension switching. v22: generalised to the End.
//
// Design: the world keeps up to THREE Level objects alive for the
// session — the overworld (created by selectLevel), the Nether
// (created on first entry, storage <world>/DIM-1) and the End
// (created on first entry, storage <world>/DIM1). The player object
// is reused: it is detached from the old level (Level::
// detachEntity, which does NOT set removed=true — removeEntity
// would freeze the player forever) and re-added to the new one by
// setLevel. Nether coordinates scale 1:8 like modern MC; the End
// arrival is always the island centre. A switch is never run inside
// a level tick: LocalPlayer::tick only sets a pending flag, and
// Minecraft::tick() performs the switch between level ticks, when
// isGeneratingLevel is false. The dimension storages never write a
// Player compound (see
// ExternalFileLevelStorage::setOmitPlayerFromLevelDat), so the live
// player state always belongs to the overworld tag —
// persistPlayerToOverworld() writes it there on every entry, every
// 60 s in-tick, and on quit (leaveGame()).
// ------------------------------------------------------------------
void Minecraft::toggleDimension() {
	if (level == NULL || overworldLevel == NULL) return;
	int target = (level->dimension->id == Dimension::NETHER)
		? overworldLevel->dimension->id : Dimension::NETHER;
	requestDimensionChange(target);
}

void Minecraft::requestDimensionChange(int targetDimension) {
	if (level == NULL || player == NULL) return;
	if (level->isClientSide) return;  // multiplayer: servers own dimensions
	if (isGeneratingLevel) return;
	if (portalCooldownTicks > 0) return;
	if (targetDimension == level->dimension->id) return;
	// v22: generalised from the v21 Nether-only guards. Going TO a
	// dimension needs the overworld to exist; coming BACK needs the
	// player to actually be in a non-overworld dimension.
	if ((targetDimension == Dimension::NETHER || targetDimension == Dimension::THE_END)
			&& overworldLevel == NULL) return;
	if (targetDimension != Dimension::NETHER && targetDimension != Dimension::THE_END
			&& level != netherLevel && level != endLevel) return;
	pendingDimension = targetDimension;
	pendingDimensionSwitch = true;
}

void Minecraft::changeDimension(int targetDimension) {
	if (level == NULL || player == NULL || isGeneratingLevel) return;
	if (targetDimension == level->dimension->id) return;
	if (level->isClientSide) return;

	Level* fromLevel = level;
	fromLevel->saveGame();
	if (fromLevel->getChunkSource())
		fromLevel->getChunkSource()->saveAll(true);

	if (targetDimension == Dimension::NETHER) {
		// Remember where to come back to (exact portal spot).
		returnX = player->x; returnY = player->y; returnZ = player->z;
		hasReturnPos = true;
		// 1:8 coordinate scale, like modern MC.
		int nx = (int)(player->x / 8.0f);
		int nz = (int)(player->z / 8.0f);
		Level* nether = getOrCreateNetherLevel(nx, nz);
		if (nether == NULL) return;
		fromLevel->detachEntity(player);
		player->setLevel(nether);
		player->dimension = Dimension::NETHER;
		player->setPos(nx + 0.5f, 70.0f, nz + 0.5f);
		pendingPortalBuild = true;
		setLevel(nether, "Entering the Nether", player);
		// v22: crash safety (see the End branch below) — same
		// one-line benefit for the Nether, pays the v21 debt.
		persistPlayerToOverworld();
	} else if (targetDimension == Dimension::THE_END) {
		// v22: remember where to come back to (exact portal spot).
		// Unlike the Nether there is NO coordinate scale: the End
		// arrival is always the island centre, on the spawn
		// platform (0.5, 67, 0.5). No portal is built on arrival —
		// the platform is part of the generated terrain.
		returnX = player->x; returnY = player->y; returnZ = player->z;
		hasReturnPos = true;
		Level* end = getOrCreateEndLevel();
		if (end == NULL) return;
		fromLevel->detachEntity(player);
		player->setLevel(end);
		player->dimension = Dimension::THE_END;
		player->setPos(0.5f, 67.0f, 0.5f);
		pendingPortalBuild = false;
		pendingEndDragon = true;
		setLevel(end, "Entering the End", player);
		// v22: crash safety — the live player (with whatever it
		// carried into the End) is now only in the End level's
		// memory. Write it into the overworld tag at the return
		// position right away, so a crash in the End keeps it.
		persistPlayerToOverworld();
	} else {
		// Back to the overworld, at the stored return position.
		float dx = hasReturnPos ? returnX : (float)overworldLevel->getLevelData()->getXSpawn();
		float dy = hasReturnPos ? returnY : (float)overworldLevel->getLevelData()->getYSpawn();
		float dz = hasReturnPos ? returnZ : (float)overworldLevel->getLevelData()->getZSpawn();
		fromLevel->detachEntity(player);
		player->setLevel(overworldLevel);
		player->dimension = overworldLevel->dimension->id;
		player->setPos(dx, dy, dz);
		// v22: only a Nether return needs a destination portal
		// (found or built). An End return lands exactly on the
		// stored return position — the end portal frame the player
		// came through — and must NOT get a Nether portal built
		// next to it.
		pendingPortalBuild = (fromLevel == netherLevel);
		setLevel(overworldLevel, "Leaving the Nether", player);
	}
	portalCooldownTicks = 100;  // 5 seconds
}

Level* Minecraft::getOrCreateNetherLevel(int nx, int nz) {
	if (netherLevel != NULL) {
		// Re-point the stored spawn at the new arrival so
		// prepareLevel generates around the portal (old levels only;
		// the gen area is derived from the shared spawn).
		netherLevel->getLevelData()->setSpawn(nx, 70, nz);
		return netherLevel;
	}

	ExternalFileLevelStorage* owStorage = (ExternalFileLevelStorage*)overworldLevel->getLevelStorage();
	std::string dimPath = owStorage->getLevelPath() + "/DIM-1";

	// Pre-seed DIM-1/level.dat so the Nether level is NOT new:
	// prepareLevel then generates around the portal destination
	// instead of calling setInitialSpawn (which would wander the
	// spawn to (256,64,256)). Same seed/gameType as the overworld.
	createFolderIfNotExists(dimPath.c_str());
	{
		FILE* f = fopen((dimPath + "/level.dat").c_str(), "rb");
		if (f == NULL) {
			LevelData data(
				LevelSettings(overworldLevel->getSeed(), overworldLevel->getLevelData()->getGameType()),
				overworldLevel->getLevelData()->getLevelName() + " Nether",
				SharedConstants::GeneratorVersion);
			data.setSpawn(nx, 70, nz);
			data.setDimension(Dimension::NETHER);
			ExternalFileLevelStorage::saveLevelData(dimPath, data, NULL);
		} else {
			fclose(f);
		}
	}

	ExternalFileLevelStorage* storage = new ExternalFileLevelStorage(currentLevelId + "_nether", dimPath);
	storage->setOmitPlayerFromLevelDat(true);
	LevelSettings settings(overworldLevel->getSeed(), overworldLevel->getLevelData()->getGameType());
	netherLevel = new ServerLevel(
		storage,
		overworldLevel->getLevelData()->getLevelName(),
		settings,
		SharedConstants::GeneratorVersion,
		new NetherDimension());
	netherLevel->getLevelData()->setDimension(Dimension::NETHER);
	netherLevel->getLevelData()->setSpawn(nx, 70, nz);
	return netherLevel;
}

Level* Minecraft::getOrCreateEndLevel() {
	if (endLevel != NULL) {
		return endLevel;
	}

	ExternalFileLevelStorage* owStorage = (ExternalFileLevelStorage*)overworldLevel->getLevelStorage();
	std::string dimPath = owStorage->getLevelPath() + "/DIM1";

	// Pre-seed DIM1/level.dat so the End level is NOT new:
	// prepareLevel then generates around the stored spawn (the
	// island centre) instead of calling setInitialSpawn (which
	// would wander the spawn to (256,64,256)). Same seed/gameType
	// as the overworld. Same pattern as getOrCreateNetherLevel.
	createFolderIfNotExists(dimPath.c_str());
	{
		FILE* f = fopen((dimPath + "/level.dat").c_str(), "rb");
		if (f == NULL) {
			LevelData data(
				LevelSettings(overworldLevel->getSeed(), overworldLevel->getLevelData()->getGameType()),
				overworldLevel->getLevelData()->getLevelName() + " The End",
				SharedConstants::GeneratorVersion);
			data.setSpawn(0, 67, 0);
			data.setDimension(Dimension::THE_END);
			ExternalFileLevelStorage::saveLevelData(dimPath, data, NULL);
		} else {
			fclose(f);
		}
	}

	ExternalFileLevelStorage* storage = new ExternalFileLevelStorage(currentLevelId + "_end", dimPath);
	storage->setOmitPlayerFromLevelDat(true);
	LevelSettings settings(overworldLevel->getSeed(), overworldLevel->getLevelData()->getGameType());
	endLevel = new ServerLevel(
		storage,
		overworldLevel->getLevelData()->getLevelName(),
		settings,
		SharedConstants::GeneratorVersion,
		new EndDimension());
	endLevel->getLevelData()->setDimension(Dimension::THE_END);
	endLevel->getLevelData()->setSpawn(0, 67, 0);
	return endLevel;
}

void Minecraft::ensureEndDragon() {
	if (endLevel == NULL || level != endLevel || player == NULL) return;
	// Already defeated: the exit portal exists at the platform
	// centre. Never respawn the dragon in this world again.
	if (level->getTile(0, EndLevelSource::EXIT_PORTAL_Y, 0) == Tile::endPortal->id) return;
	// Already alive: don't spawn a second one (e.g. re-entering
	// mid-fight after a quit — the live level keeps its dragon).
	for (unsigned int i = 0; i < level->entities.size(); i++) {
		if (level->entities[i] != NULL
				&& level->entities[i]->getEntityTypeId() == MobTypes::EnderDragon) {
			return;
		}
	}
	EnderDragon* dragon = new EnderDragon(level);
	dragon->setPos(0.5f, 82.0f, 0.5f);
	level->addEntity(dragon);
	// v28: one initial end crystal on each pillar top, so the first
	// fight has crystals to shoot down (modern MC). Only here — on
	// the very first spawn: re-entry mid-fight (dragon alive) and
	// post-death (portal exists) both return above, so these are
	// never re-added, and a crystal the player destroys stays
	// destroyed. Initial crystals do NOT count toward the v27
	// respawn (EndCrystal::initial), or the survivors would resummon
	// the dragon ~1 s after the first death and clear the exit
	// portal before the player can use it. Worlds whose first fight
	// already started in v22–v27 get no retroactive crystals (the
	// dragon is alive there, so this branch never runs) — that is
	// deliberate, not a bug.
	for (int i = 0; i < EndLevelSource::getNumPillars(); i++) {
		int px, py, pz;
		if (!EndLevelSource::getPillarTop(i, px, py, pz)) continue;
		level->addEntity(new EndCrystal(level, px + 0.5f, (float)(py + 1), pz + 0.5f, true));
	}
}

void Minecraft::persistPlayerToOverworld() {
	// Only meaningful while the player is in another dimension:
	// the overworld tag is the one the next load reads.
	if (level == NULL || level->dimension == NULL
			|| overworldLevel == NULL || player == NULL
			|| level->dimension->id == overworldLevel->dimension->id) {
		return;
	}
	float sx = player->x, sy = player->y, sz = player->z;
	int sdim = player->dimension;
	if (hasReturnPos) player->setPos(returnX, returnY, returnZ);
	player->dimension = overworldLevel->dimension->id;
	bool inPlayers = false;
	for (unsigned int i = 0; i < overworldLevel->players.size(); i++) {
		if (overworldLevel->players[i] == player) { inPlayers = true; break; }
	}
	if (!inPlayers) overworldLevel->players.push_back(player);
	overworldLevel->saveLevelData();
	for (unsigned int i = 0; i < overworldLevel->players.size(); i++) {
		if (overworldLevel->players[i] == player) {
			overworldLevel->players.erase(overworldLevel->players.begin() + i);
			break;
		}
	}
	player->setPos(sx, sy, sz);
	player->dimension = sdim;
}

void Minecraft::findOrCreatePortal(float px, float py, float pz) {
	if (level == NULL || player == NULL) return;
	int cx = (int)px, cy = (int)py, cz = (int)pz;

	// 1. Look for an existing portal within 16 blocks (horizontal),
	// scanning near the arrival height first.
	for (int r = 0; r <= 16; r++) {
		for (int dx = -r; dx <= r; dx++) for (int dz = -r; dz <= r; dz++) {
			if (r > 0 && std::max(abs(dx), abs(dz)) != r) continue;
			int y0 = cy - 16; if (y0 < 1) y0 = 1;
			int y1 = cy + 16; if (y1 > 126) y1 = 126;
			for (int y = y0; y <= y1; y++) {
				if (level->getTile(cx + dx, y, cz + dz) == Tile::portalTile->id) {
					player->setPos(cx + dx + 0.5f, (float)y, cz + dz + 0.5f);
					return;
				}
			}
		}
	}

	// 2. None found: build a 4x5 obsidian frame on solid ground near
	// the arrival point (spiral out to 8 blocks, scan down from
	// y=110 for solid ground with 4 air above it).
	for (int r = 0; r <= 8; r++) {
		for (int dx = -r; dx <= r; dx++) for (int dz = -r; dz <= r; dz++) {
			if (r > 0 && std::max(abs(dx), abs(dz)) != r) continue;
			for (int y = 110; y >= 8; y--) {
				int t = level->getTile(cx + dx, y, cz + dz);
				if (t == 0 || !Tile::tiles[t]->isSolidRender()) continue;
				bool clear = true;
				for (int k = 1; k <= 4; k++) {
					int a = level->getTile(cx + dx, y + k, cz + dz);
					if (a != 0 && a != Tile::fire->id) { clear = false; break; }
				}
				if (!clear) break;  // low ceiling here; try the next column
				int bx = cx + dx, baseY = y + 1, bz = cz + dz;
				for (int i = -1; i <= 2; i++) {
					level->setTile(bx + i, baseY, bz, Tile::obsidian->id);
					level->setTile(bx + i, baseY + 4, bz, Tile::obsidian->id);
				}
				for (int j = 1; j <= 3; j++) {
					level->setTile(bx - 1, baseY + j, bz, Tile::obsidian->id);
					level->setTile(bx + 2, baseY + j, bz, Tile::obsidian->id);
					level->setTile(bx,     baseY + j, bz, Tile::portalTile->id);
					level->setTile(bx + 1, baseY + j, bz, Tile::portalTile->id);
				}
				player->setPos(bx + 1.0f, (float)(baseY + 1), bz + 0.5f);
				return;
			}
		}
	}
	// 3. Last resort: the player stays at the arrival position set
	// by changeDimension (no portal could be built nearby).
}

void Minecraft::cleanupLastJoinedServerFolders()
{
	LOGI("Cleaning up LastJoinedServer folder\n");
	
	LevelStorageSource* storageSource = getLevelSource();
	if (!storageSource) {
		LOGI("ERROR: Could not get storage source for cleanup\n");
		return;
	}
	
	LevelSummaryList levels;
	storageSource->getLevelList(levels);
	
	int deletedCount = 0;
	for (unsigned int i = 0; i < levels.size(); ++i) {
		const LevelSummary& level = levels[i];
		
		if (level.id.find("_LastJoinedServer") != std::string::npos) {
			LOGI("Deleting _LastJoinedServer world folder: %s\n", level.id.c_str());
			storageSource->deleteLevel(level.id);
			deletedCount++;
		}
	}
	
	LOGI(" Cleanup completed. \n", deletedCount);
}

void Minecraft::leaveGame(bool renameLevel /*=false*/)
{
    if (isGeneratingLevel || !_hasSignaledGeneratingLevelFinished)
        return;
    
	isGeneratingLevel = false;
	bool saveLevel = level && (!level->isClientSide || renameLevel);

	raknetInstance->disconnect();
	if (saveLevel) {
		// If server or wanting to save level as client, save all unsaved chunks!
		level->getChunkSource()->saveAll(true);
	}

	// Clean up all _LastJoinedServer world folders
	cleanupLastJoinedServerFolders();

	LOGI("Clearing levels\n");

	cameraTargetPlayer = NULL;
#ifndef STANDALONE_SERVER
	levelRenderer->setLevel(NULL);
	particleEngine->setLevel(NULL);
#endif
	LOGI("Erasing callback\n");
	delete netCallback;
	netCallback = NULL;

	// v22: if quitting from the Nether OR the End, the live player
	// state (pos, inventory, XP — including dimension loot) only
	// exists in that level's memory; the overworld level.dat still
	// holds the pre-trip tag, and the dimension storages
	// deliberately never write a Player compound. Write the live
	// player into the overworld tag at the return position before
	// deleting anything, or the loot would be lost on the next load.
	// (v21 had this inline for the Nether only; it is now the
	// shared persistPlayerToOverworld, also called on every
	// dimension entry and every 60 s in-tick, so a crash — not just
	// a clean quit — keeps the loot.)
	persistPlayerToOverworld();

	LOGI("Erasing level\n");
	if (level != NULL) {
		Level* deletedLevel = level;
		delete level->getLevelStorage();
		delete level;
		level = NULL;
		// v21: the inactive dimension level is owned here too.
		if (deletedLevel == overworldLevel) overworldLevel = NULL;
		if (deletedLevel == netherLevel) netherLevel = NULL;
		if (deletedLevel == endLevel) endLevel = NULL;
	}
	if (overworldLevel != NULL) {
		delete overworldLevel->getLevelStorage();
		delete overworldLevel;
		overworldLevel = NULL;
	}
	if (netherLevel != NULL) {
		delete netherLevel->getLevelStorage();
		delete netherLevel;
		netherLevel = NULL;
	}
	if (endLevel != NULL) {
		delete endLevel->getLevelStorage();
		delete endLevel;
		endLevel = NULL;
	}
	//delete player;
	player = NULL;
	cameraTargetPlayer = NULL;

	_running = false;
#ifndef STANDALONE_SERVER
	if (renameLevel) {
		setScreen(new RenameMPLevelScreen(LevelStorageSource::TempLevelId));
	}
	else
		screenChooser.setScreen(SCREEN_STARTMENU);
#endif
}

void Minecraft::prepareLevel(const std::string& title) {
	LOGI("status: 1\n");
	progressStageStatusId = 1;

	Stopwatch A, B, C, D;
	A.start();

	Stopwatch L;

	// Dont update lights if we load the level (ok, actually just with leveldata version=1.+(?))
	if (!level->isNew())
		level->setUpdateLights(false);

	// v16: generate ONLY the spawn area (radius CHUNK_SPAWN_GEN_RADIUS
	// chunks), not the whole 1024-chunk world. The rest generates on
	// demand as the player walks (ChunkCache streams it in). For a new
	// world the spawn is the centre (setInitialSpawn() runs below); for
	// an old world use the stored spawn (a bed spawn can be off-centre).
	int spawnCX, spawnCZ;
	if (level->isNew()) {
		spawnCX = CHUNK_CACHE_WIDTH / 2;
		spawnCZ = CHUNK_CACHE_WIDTH / 2;
	} else {
		Pos spawn = level->getSharedSpawnPos();
		// Floor-divide (C++ / truncates toward zero, which would put
		// a negative spawn one chunk off).
		spawnCX = spawn.x >= 0 ? spawn.x / 16 : -((-spawn.x + 15) / 16);
		spawnCZ = spawn.z >= 0 ? spawn.z / 16 : -((-spawn.z + 15) / 16);
	}
	// v17: the old clamps pinned the spawn area to chunks 0..31 (the
	// 512x512 border). For an old world whose stored/bed spawn sits
	// outside that range, the clamps made cx1 < cx0 and generated
	// NOTHING at load. In an infinite world the spawn area is simply
	// spawn +- CHUNK_SPAWN_GEN_RADIUS, wherever the spawn is.
	int cx0 = spawnCX - CHUNK_SPAWN_GEN_RADIUS;
	int cx1 = spawnCX + CHUNK_SPAWN_GEN_RADIUS;
	int cz0 = spawnCZ - CHUNK_SPAWN_GEN_RADIUS;
	int cz1 = spawnCZ + CHUNK_SPAWN_GEN_RADIUS;
	int Max = (cx1 - cx0 + 1) * (cz1 - cz0 + 1);
	int pp = 0;
	for (int cx = cx0; cx <= cx1; cx++) {
        for (int cz = cz0; cz <= cz1; cz++) {
            progressStagePercentage = 100 * pp++ / Max;
            //printf("level generation progress %d\n", progressStagePercentage);
			B.start();
            level->getTile(cx * 16 + 8, 64, cz * 16 + 8);
			B.stop();
			L.start();
			if (level->isNew())
				while (level->updateLights())
					;
			L.stop();
        }
    }
	A.stop();
	level->setUpdateLights(true);

	C.start();
	for (int x = cx0; x <= cx1; x++)
	{
		for (int z = cz0; z <= cz1; z++)
		{
			LevelChunk* chunk = level->getChunk(x, z);
			if (chunk && !chunk->createdFromSave)
			{
				chunk->unsaved = false;
				chunk->clearUpdateMap();
			}
		}
	}
	C.stop();

	LOGI("status: 3\n");
	progressStageStatusId = 3;
	if (level->isNew()) {
		level->setInitialSpawn(); // @note: should obviously be called from Level itself
		level->saveLevelData();
		level->getChunkSource()->saveAll(false);
		level->saveGame();
	} else {
		level->saveLevelData();
		level->loadEntities();
	}

	progressStagePercentage = -1;
	progressStageStatusId = 2;
	LOGI("status: 2\n");

	D.start();
	level->prepare();
	D.stop();

	A.print("Generate level: ");
	L.print(" - light: ");
	B.print(" - getTl: ");
	C.print(" - clear: ");
	D.print(" - prepr: ");
	progressStageStatusId = 0;
}

void Minecraft::update() {
	//LOGI("Enter Update\n");

	if (Options::debugGl)
		LOGI(">>>>>>>>>>\n");

	TIMER_PUSH("root");

	//if (level) {
	//	LOGI("numplayers: %d\n", level->players.size());
	//	for (int i = 0; i < level->players.size(); ++i) {
	//		Player* p = level->players[i];
	//		bool inEnt = std::find(level->entities.begin(), level->entities.end(), p) != level->entities.end();
	//		LOGI("  %p, %d, %d - in? %d\n", p, p->entityId, p->owner.ToUint32(p->owner), inEnt);
	//	}
	//}

	if (pause && level != NULL) {
		float lastA = timer.a;
		timer.advanceTime();
		timer.a = lastA;
	} else {
		timer.advanceTime();
	}

	if (raknetInstance) {
		raknetInstance->runEvents(netCallback);
	}

	TIMER_PUSH("tick");
	int toTick = timer.ticks;
	for (int i = 0; i < toTick; ++i, ++ticks)
		tick(i, toTick-1);

	TIMER_POP_PUSH("updatelights");
	if (level && !isGeneratingLevel) {
		level->updateLights();
	}
	TIMER_POP();

	#ifndef STANDALONE_SERVER
		if (gameMode != NULL) gameMode->render(timer.a);
		TIMER_PUSH("sound");
		soundEngine->update(player, timer.a);
		TIMER_POP_PUSH("render");
		gameRenderer->render(timer.a);
		TIMER_POP();
	#else
	CThread::sleep(1);
	#endif
#ifndef STANDALONE_SERVER
	Multitouch::resetThisUpdate();
#endif
	TIMER_POP();
#ifndef STANDALONE_SERVER
	checkGlError("Update finished");

	if (options.renderDebug) {
		if (!PerfTimer::enabled) {
			PerfTimer::reset();
			PerfTimer::enabled = true;
		}

		//TIMER_PUSH("debugfps");
		_perfRenderer->renderFpsMeter(1);
		checkGlError("render debug");
		//TIMER_POP();
	} else {
		PerfTimer::enabled = false;
	}
#endif
	//LOGI("Exit Update\n");
}

void Minecraft::tick(int nTick, int maxTick) {
	if (missTime > 0) missTime--;
#ifndef STANDALONE_SERVER
	if (!screen && player) {
		if (player->health <= 0) {
			setScreen(new DeathScreen());
		}
	}
#endif
	TIMER_PUSH("gameMode");
	if (level && !pause) {
		gameMode->tick();
	}

	TIMER_POP_PUSH("commandServer");
	if (level && _commandServer) {
		_commandServer->tick();
	}

	TIMER_POP_PUSH("input");
	tickInput();
#ifndef STANDALONE_SERVER
	TIMER_POP_PUSH("gui");
	gui.tick();
#endif
	//
	// Ongoing level generation in a (perhaps) different thread. When it's
	// ready, _levelGenerated() is called once and any threads are deleted.
	//
	if (isGeneratingLevel) {
		return;
	} else if (!_hasSignaledGeneratingLevelFinished) {
		if (generateLevelThread) {
			delete generateLevelThread;
			generateLevelThread = NULL;
		}
		_levelGenerated();
	}

	// v21: deferred dimension switch (requested from
	// LocalPlayer::tick while standing in a portal). Runs between
	// level ticks, never inside one; changeDimension starts a new
	// generation, so this tick must end here.
	if (pendingDimensionSwitch && !pause) {
		pendingDimensionSwitch = false;
		changeDimension(pendingDimension);
		return;
	}
	if (portalCooldownTicks > 0) portalCooldownTicks--;

	// v22: crash safety — while the player is in the Nether or the
	// End, rewrite the live player into the overworld tag every 60 s
	// (1200 ticks), so a crash loses at most a minute of dimension
	// loot. persistPlayerToOverworld is a no-op in the overworld.
	if (++ticksSincePlayerPersist >= 1200) {
		ticksSincePlayerPersist = 0;
		persistPlayerToOverworld();
	}

	//
	// Normal game loop, run before or efter level generation
	//
	if (level != NULL)
	{
		if (!pause) {
#ifndef STANDALONE_SERVER
			TIMER_POP_PUSH("gameRenderer");
			gameRenderer->tick(nTick, maxTick);

			TIMER_POP_PUSH("levelRenderer");
			levelRenderer->tick();
#endif
			level->difficulty = options.difficulty;
			if (level->isClientSide) level->difficulty = Difficulty::EASY;

			TIMER_POP_PUSH("level");
			level->tickEntities();
			level->tick();
#ifndef STANDALONE_SERVER
			TIMER_POP_PUSH("animateTick");
			if (player) {
				level->animateTick(Mth::floor(player->x), Mth::floor(player->y), Mth::floor(player->z));
			}
#endif
		}
	}
#ifndef STANDALONE_SERVER
	textures->loadAndBindTexture("terrain.png");
	if (!pause && !(screen && !screen->renderGameBehind())) {
		#if !defined(RPI)
			#ifdef __APPLE__
			if (isSuperFast())
			#endif
			{
			if (nTick == maxTick) {
				TIMER_POP_PUSH("textures");
				textures->tick(true);
			}
			}
		#endif
	}
	TIMER_POP_PUSH("particles");
	particleEngine->tick();
	if (screen) {
		screenMutex = true;
		screen->tick();
		screenMutex = false;
	}

	// @note: fix to keep "isPressed" and "isReleased" as long as necessary.
	//      Most likely keyboard and mouse could/should be reset here as well.
	Multitouch::reset();
#endif
	TIMER_POP();
}

class InputRAII {
public:
	~InputRAII() {
#ifndef STANDALONE_SERVER
		Mouse::reset();
		Keyboard::reset();
#endif
	}
};

void Minecraft::tickInput() {
#ifndef STANDALONE_SERVER
	InputRAII raiiInput;

	if (screen && !screen->passEvents) {
		screenMutex = true;
		screen->updateEvents();
		//screen->updateSetScreen();
		screenMutex = false;
		if (hasScheduledScreen) {
			setScreen(scheduledScreen);
			scheduledScreen = NULL;
			hasScheduledScreen = false;
		}
		return;
	}

	if (!player) {
		return;
	}

#ifdef RPI
	bool mouseDiggable = true;
	bool allowGuiClicks = !mouseGrabbed;
#else
	bool mouseDiggable = !gui.isInside(Mouse::getX(), Mouse::getY());
	bool allowGuiClicks = true;
#endif

	TIMER_PUSH("mouse");
	while (Mouse::next()) {
		//if (Mouse::getButtonState(MouseAction::ACTION_LEFT))
		//	LOGI("mouse-down-at: %d, %d\n", Mouse::getX(), Mouse::getY());
//#ifndef _UWP
        	int passedTime = getTimeMs() - lastTickTime;
        	if (passedTime > 200) continue; // @note: As long Mouse::clear CLEARS the whole buffer, it's safe to break here
		// But since it might be rewritten anyway (and hopefully there aren't a lot of messages, we just continue.
//#endif

		const MouseAction& e = Mouse::getEvent();

#ifdef RPI // If clicked when not having focus, get focus @keyboard
		if (!mouseGrabbed) {
			if (!screen && e.data == MouseAction::DATA_DOWN) {
				grabMouse();
			}
		}
#endif

		if (allowGuiClicks && e.action == MouseAction::ACTION_LEFT && e.data == MouseAction::DATA_DOWN) {
			gui.handleClick(MouseAction::ACTION_LEFT, Mouse::getX(), Mouse::getY());
		}

		if (e.action == MouseAction::ACTION_WHEEL) {
			Inventory* v = player->inventory;
			bool isMouseLocked = mouseGrabbed;
			int numSlots = isMouseLocked ? gui.getNumSlots() : gui.getNumSlots() - 1;
			int slot = (v->selected - e.dy + numSlots) % numSlots;
			v->selectSlot(slot);
		}
		/*
		if (mouseDiggable && options.useMouseForDigging) {
			if (Mouse::getEventButton() == MouseAction::ACTION_LEFT && Mouse::getEventButtonState()) {
				handleMouseClick(MouseAction::ACTION_LEFT);
				lastClickTick = ticks;
			}
			if (Mouse::getEventButton() == MouseAction::ACTION_RIGHT && Mouse::getEventButtonState()) {
				handleMouseClick(MouseAction::ACTION_RIGHT);
				lastClickTick = ticks;
			}
		}
		*/
	}

	TIMER_POP_PUSH("keyboard");
	while (Keyboard::next()) {
		int key = Keyboard::getEventKey();
		bool isPressed = (Keyboard::getEventKeyState() == KeyboardAction::KEYDOWN);
		player->setKey(key, isPressed);

		if (isPressed) {
			gui.handleKeyPressed(key);

			#if defined(WIN32) || defined(RPI)//|| defined(_DEBUG) || defined(DEBUG)
				if (key >= '0' && key <= '9') {
					int digit = key - '0';
					int slot = digit - 1;

					bool isMouseLocked = mouseGrabbed;
					int maxSlot = isMouseLocked ? gui.getNumSlots() - 1 : gui.getNumSlots() - 2;
					
					if (slot >= 0 && slot <= maxSlot)
						player->inventory->selectSlot(slot);

					#if defined(WIN32) && !defined(WINAPI_FAMILY)
						if (digit >= 1 && GetAsyncKeyState(VK_CONTROL) < 0) {
							// Set adventure settings here!
							AdventureSettingsPacket p(level->adventureSettings);
							p.toggle((AdventureSettingsPacket::Flags)(1 << slot));
							p.fillIn(level->adventureSettings);
							raknetInstance->send(p);
						}
						if (digit == 0) {
							Pos pos((int)player->x, (int)player->y-1, (int)player->z);
							SetSpawnPositionPacket p(pos);
							raknetInstance->send(p);
						}
					#endif
				}
			#endif
			#if defined(RPI)
				if (key == Keyboard::KEY_E) {
					screenChooser.setScreen(SCREEN_BLOCKSELECTION);
				}
				if (!screen && key == Keyboard::KEY_O || key == 250) {
					releaseMouse();
				}
			#endif
			#if defined(WIN32)
// Its for testing or whatewer so not really that needed (F_DISABLED)
//				if (key == Keyboard::KEY_F) {
//					options.isFlying = !options.isFlying;
//					player->noPhysics = options.isFlying;
//				}

				if (key == Keyboard::KEY_T) {
					options.thirdPersonView = !options.thirdPersonView;
					/*
					ImprovedNoise noise;
					for (int i = 0; i < 16; ++i)
						printf("%d\t%f\n", i, noise.grad2(i, 3, 8));
					*/
				}

				if (key == Keyboard::KEY_O) {
					useAmbientOcclusion = !useAmbientOcclusion;
					options.ambientOcclusion = useAmbientOcclusion;
					levelRenderer->allChanged();
				}

				if (key == Keyboard::KEY_L)
					options.viewDistance = (options.viewDistance + 1) % 4;

				if (key == Keyboard::KEY_U) {
					onGraphicsReset();
				}

				if (key == Keyboard::KEY_B || key == 108) // Toggle the game mode
					setIsCreativeMode(!isCreativeMode());

				if (key == Keyboard::KEY_P) // Step forward in time
					level->setTime( level->getTime() + 1000);

				if (key == Keyboard::KEY_G) {
					setScreen(new ArmorScreen());
					/*
					std::vector<AABB>& boxs = level->getCubes(NULL, AABB(128.1f, 73, 128.1f, 128.9f, 74.9f, 128.9f));
					LOGI("boxes: %d\n", (int)boxs.size());
					*/
				}

				if (key == Keyboard::KEY_Y) {
					textures->reloadAll();
					player->hurtTo(2);
				}
				if (key == Keyboard::KEY_Z || key == 108) {
					for (int i = 0; i < 1; ++i) {
						Mob* mob = NULL;
						int forceId = 0;//MobTypes::Sheep;

						int types[] = {
							MobTypes::Sheep,
							MobTypes::Pig,
							MobTypes::Chicken,
							MobTypes::Cow,
						};

						int mobType = (forceId > 0)? forceId : types[Mth::random(sizeof(types) / sizeof(int))];
						mob = MobFactory::CreateMob(mobType, level);

						//((Animal*)mob)->setAge(-1000);
						float dx = 4 - 8 * Mth::random() + 4 * Mth::sin(Mth::DEGRAD * player->yRot);
						float dz = 4 - 8 * Mth::random() + 4 * Mth::cos(Mth::DEGRAD * player->yRot);
						if (mob && !MobSpawner::addMob(level, mob, player->x + dx, player->y, player->z + dz, Mth::random()*360, 0, true))
							delete mob;
					}
				}

				if (key == Keyboard::KEY_X) {
					const EntityList& entities = level->getAllEntities();
					for (int i = entities.size()-1; i >= 0; --i) {
						Entity* e = entities[i];
						if (!e->isPlayer())
							level->removeEntity(e);
					}
				}

				if (key == Keyboard::KEY_C /*|| key == 4*/) {
					player->inventory->clearInventoryWithDefault();
					// @todo: Add saving here for benchmarking
				}
				if (key == Keyboard::KEY_H) {
					setScreen( new PrerenderTilesScreen() );
				}

				if (key == Keyboard::KEY_O) {
					for (int i = Inventory::MAX_SELECTION_SIZE; i < player->inventory->getContainerSize(); ++i)
						if (player->inventory->getItem(i))
							player->inventory->dropSlot(i, false);
				}
				if (key == Keyboard::KEY_F3) {
					options.renderDebug = !options.renderDebug;
				}
				if (key == Keyboard::KEY_M) {
					options.difficulty = (options.difficulty == Difficulty::PEACEFUL)?
						Difficulty::NORMAL : Difficulty::PEACEFUL;
					//setIsCreativeMode( !isCreativeMode() );
				}

				if (options.renderDebug) {
					if (key >= '0' && key <= '9') {
						_perfRenderer->debugFpsMeterKeyPress(key - '0');
					}
				}
			#endif

			if (key == Keyboard::KEY_E) {
				if (screen) {
					setScreen(NULL);
				} else {
					screenChooser.setScreen(SCREEN_BLOCKSELECTION);
				}
			}

			if (key == Keyboard::KEY_Q) {
				if (player && player->inventory) {
					ItemInstance* currentItem = player->inventory->getSelected();
					if (currentItem && currentItem->count > 0) {
						player->inventory->dropSlot(player->inventory->selected, false);
					}
				}
			}

			if (key == Keyboard::KEY_ESCAPE)
				pauseGame(false);

			#ifndef OPENGL_ES
				if (key == Keyboard::KEY_P) {
					static bool isWireFrame = false;
					isWireFrame = !isWireFrame;
					glPolygonMode(GL_FRONT, isWireFrame? GL_LINE : GL_FILL);
					//glPolygonMode(GL_BACK, isWireFrame? GL_LINE : GL_FILL);
				}
			#endif
		}
		#ifdef WIN32
		#endif


		//if (!isPressed) LOGI("Key released: %d\n", key);

		if (!options.useMouseForDigging) {
#ifndef _UWP
			int passedTime = getTimeMs() - lastTickTime;
			if (passedTime > 200) continue;
#endif

			// Destroy and attack is on same button
			if (key == options.keyDestroy.key && isPressed) {
				BuildActionIntention bai(BuildActionIntention::BAI_REMOVE | BuildActionIntention::BAI_ATTACK);
				handleBuildAction(&bai);
			}
			else // Build and use/interact is on same button
			if (key == options.keyUse.key && isPressed) {
				BuildActionIntention bai(BuildActionIntention::BAI_BUILD | BuildActionIntention::BAI_INTERACT);
				handleBuildAction(&bai);
			}
		}
	}

	TIMER_POP_PUSH("tickbuild");
	BuildActionIntention bai;
	// @note: This might be a problem. This method is polling based here, but
	//        event based when using mouse (or keys..), and in java. Not quite
	//        sure just yet what way to go.
	bool buildHandled = inputHolder->getBuildInput()->tickBuild(player, &bai);
	if (buildHandled) {
		if (!bai.isRemoveContinue())
			handleBuildAction(&bai);
	}

	bool isTryingToDestroyBlock = (options.useMouseForDigging
			?	(Mouse::isButtonDown(MouseAction::ACTION_LEFT) && (mouseDiggable || hitResult.type == ENTITY))
			:	Keyboard::isKeyDown(options.keyDestroy.key))
		||	(buildHandled && bai.isRemove());

	TIMER_POP_PUSH("handlemouse");
#ifdef RPI
	handleMouseDown(MouseAction::ACTION_LEFT, isTryingToDestroyBlock);
	handleMouseClick(buildHandled && bai.isInteract()
		|| options.useMouseForDigging && Mouse::isButtonDown(MouseAction::ACTION_RIGHT));
#else
	handleMouseDown(MouseAction::ACTION_LEFT, isTryingToDestroyBlock || (buildHandled && bai.isInteract()));
#endif

	lastTickTime = getTimeMs();

	// we have (hopefully) handled the keyboard & mouse queue and it
	// can now be emptied. If wanted, the reset could be changed to:
	// index -= numRead; // then this code doesn't have to be placed here
	// + it prepares for tick not handling all or any events.
	// update: RAII'ing instead, see above
	//Keyboard::reset();
	//Mouse::reset();

	TIMER_POP();
#endif
}

void Minecraft::handleMouseDown(int button, bool down) {
#ifndef STANDALONE_SERVER
#ifndef RPI
	if(player->isUsingItem()) {
		if(!down && !Keyboard::isKeyDown(options.keyUse.key)) {
			gameMode->releaseUsingItem(player);
		}
		return;
	}
#endif
	if(player->isSleeping()) {
		return;
	}
    if (button == MouseAction::ACTION_LEFT && missTime > 0) {
		return;
	}
	if (down && hitResult.type == TILE && button == MouseAction::ACTION_LEFT && !hitResult.indirectHit) {
        int x = hitResult.x;
        int y = hitResult.y;
        int z = hitResult.z;
        gameMode->continueDestroyBlock(x, y, z, hitResult.f);
        particleEngine->crack(x, y, z, hitResult.f);
		player->swing();
    } else if (down && hitResult.type == ENTITY && button == MouseAction::ACTION_LEFT) {
		// Create attack action for entity
		BuildActionIntention bai(BuildActionIntention::BAI_ATTACK);
		handleBuildAction(&bai);
    } else {
        gameMode->stopDestroyBlock();
    }
#endif
}

void Minecraft::handleMouseClick(int button) {
//	BuildActionIntention bai(
//		(button == MouseAction::ACTION_LEFT)?
//			BuildActionIntention::BAI_REMOVE
//		:	BuildActionIntention::BAI_BUILD);
//
//	handleBuildAction(&bai);
}

void Minecraft::handleBuildAction(BuildActionIntention* action) {
#ifndef STANDALONE_SERVER
	if (action->isRemove()) {
		if (missTime > 0) return;
		player->swing();
	}
	if(player->isUsingItem())
		return;
    bool mayUse = true;

	if (!hitResult.isHit()) {
		if (action->isRemove() && !gameMode->isCreativeType()) {
			missTime = 10;
		}
    } else if (hitResult.type == ENTITY) {
        // Validate entity pointer before using
        if (!hitResult.entity || hitResult.entity->removed) {
            return;
        }
        if (action->isAttack()) {
			player->swing();
			//LOGI("attacking!\n");
			InteractPacket packet(InteractPacket::Attack, player->entityId, hitResult.entity->entityId);
			raknetInstance->send(packet);
            gameMode->attack(player, hitResult.entity);
		} else if (action->isInteract()) {
			if (hitResult.entity->interactPreventDefault())
				mayUse = false;
			//LOGI("interacting!\n");
			InteractPacket packet(InteractPacket::Interact, player->entityId, hitResult.entity->entityId);
			raknetInstance->send(packet);
            gameMode->interact(player, hitResult.entity);
        }
    } else if (hitResult.type == TILE) {
		int x = hitResult.x;
        int y = hitResult.y;
        int z = hitResult.z;
        int face = hitResult.f;

		int oldTileId = level->getTile(x, y, z);
        Tile* oldTile = Tile::tiles[oldTileId];

		//bool tryDestroyBlock = false;

		if (action->isRemove()) {
			if (!oldTile)
				return;

			//LOGI("tile: %s - %d, %d, %d. b: %f - %f\n", oldTile->getDescriptionId().c_str(), x, y, z, oldTile->getBrightness(level, x, y, z), oldTile->getBrightness(level, x, y+1, z));
            level->extinguishFire(x, y, z, hitResult.f);
			gameMode->startDestroyBlock(x, y, z, hitResult.f);
        }
		else {
			ItemInstance* item = player->inventory->getSelected();
 			if (gameMode->useItemOn(player, level, item, x, y, z, face, hitResult.pos)) {
			    mayUse = false;
			    player->swing();
			#ifdef RPI
				} else if (item && item->id == ((Item*)Item::sword_iron)->id) {
					player->swing();
			#endif
			}
			if (item && item->count <= 0) {
				player->inventory->clearSlot(player->inventory->selected);
			}
			//} else if (item && item->count != oldCount) {
			//	  gameRenderer->itemInHandRenderer->itemPlaced();
			//}
		}
	}
	if (mayUse && action->isInteract()) {
		ItemInstance* item = player->inventory->getSelected();
		if (item && !player->isUsingItem()) {
			if (gameMode->useItem(player, level, item)) {
				gameRenderer->itemInHandRenderer->itemUsed();
			}
			if (item && item->count <= 0) {
				player->inventory->clearSlot(player->inventory->selected);
			}
		}
	}
#endif
}

bool Minecraft::isOnlineClient()
{
    return (level != NULL && level->isClientSide);
}

bool Minecraft::isOnline()
{
	return netCallback != NULL;
}

void Minecraft::pauseGame(bool isBackPaused) {
#ifndef STANDALONE_SERVER
	if (screen != NULL) return;
	screenChooser.setScreen(isBackPaused? SCREEN_PAUSEPREV : SCREEN_PAUSE);
#endif
}
void Minecraft::gameLostFocus() {
#ifndef STANDALONE_SERVER
	if(screen != NULL) {
		screen->lostFocus();
	}
#endif
}


void Minecraft::setScreen( Screen* screen )
{
#ifndef	STANDALONE_SERVER
	Mouse::reset();
	Multitouch::reset();
	Multitouch::resetThisUpdate();

#ifdef _UWP
	if (platform()) {
		static_cast<AppPlatform_uwp*>(platform())->clearInputBuffer();
	}
#endif

	// v42: release held movement keys on EVERY platform, not just UWP.
	// On Android this call was compiled out, so TouchscreenInput::xa/ya
	// kept their last values while a screen covered the game
	// (LocalPlayer::aiStep skips input->tick() while covered) and
	// updateAi() kept copying the frozen values into xxa/yya, so the
	// player kept walking with the crafting (or any other) screen open
	// until it closed. releaseAllKeys() is a no-op on the IMoveInput
	// base; the touch override also clears sprint + the tracked
	// forward pointer, so no phantom sprint on close.
	if (inputHolder && inputHolder->getMoveInput()) {
		inputHolder->getMoveInput()->releaseAllKeys();
	}

	if (screenMutex) {
		hasScheduledScreen = true;
		scheduledScreen = screen;
		return;
	}

	if (screen != NULL && screen->isErrorScreen())
		return;
	if (screen == NULL && level == NULL)
		screen = screenChooser.createScreen(SCREEN_STARTMENU);

	if (this->screen != NULL) {
		this->screen->removed();
		delete this->screen;
	}

	this->screen = screen;
	if (screen != NULL) {
		releaseMouse();
		//ScreenSizeCalculator ssc = new ScreenSizeCalculator(options, width, height);
		int screenWidth = (int)(width * Gui::InvGuiScale); //ssc.getWidth();
		int screenHeight = (int)(height * Gui::InvGuiScale); //ssc.getHeight();
		screen->init(this, screenWidth, screenHeight);

		if (screen->isInGameScreen() && level) {
			level->saveLevelData();
            level->saveGame();
        }

		//noRender = false;
	} else {
		grabMouse();
	}
#endif
}

void Minecraft::grabMouse()
{
#ifndef STANDALONE_SERVER
	if (mouseGrabbed) return;
	
	if (!platform()->supportsNonTouchscreen()) {
		return;
	}
	
	mouseGrabbed = true;
	_reloadInput();
	mouseHandler.grab();
#endif
}

void Minecraft::releaseMouse()
{
#ifndef STANDALONE_SERVER
	if (!mouseGrabbed) return;
	if (player) player->releaseAllKeys();
	mouseGrabbed = false;
	_reloadInput();
	mouseHandler.release();
#endif
}

bool Minecraft::useTouchscreen() {
#ifdef RPI
	return false;
#endif
	#if defined(ANDROID) || defined(__APPLE__) || defined(_UWP)
		if (mouseGrabbed) return false;
		return options.useTouchScreen || !_supportsNonTouchscreen;
	#else
		if (mouseGrabbed) return false;
		return options.useTouchScreen || !_supportsNonTouchscreen;
	#endif
}
bool Minecraft::supportNonTouchScreen() {
	return _supportsNonTouchscreen;
}
void Minecraft::init()
{
	options.minecraft = this;
	options.initDefaultValues();
#ifndef STANDALONE_SERVER
	checkGlError("Init enter");

	_supportsNonTouchscreen = platform()->supportsNonTouchscreen();
	
	if (platform()->supportsTouchscreen()) {
		options.useTouchScreen = true;
		LOGI("Touchscreen detected - forcing touch mode for mobile UI\n");
	}

	textures = new Textures(&options, platform());
	textures->addDynamicTexture(new WaterTexture());
	textures->addDynamicTexture(new WaterSideTexture());
	gui.texturesLoaded(textures);

	levelRenderer = new LevelRenderer(this);
	gameRenderer = new GameRenderer(this);
	particleEngine = new ParticleEngine(level, textures);

	// Platform specific initialization here
	font = new Font(&options, "font/default8.png", textures);

	_perfRenderer = new PerfRenderer(this, font);

	checkGlError("Init complete");
#endif

	user = new User("TestUser", "");
	setIsCreativeMode(false); // false means it's Survival Mode
	reloadOptions();

}

void Minecraft::setSize(int w, int h) {
#ifndef STANDALONE_SERVER
    transformResolution(&w, &h);

	width  = w;
	height = h;

	if (width >= 1000) {
        #ifdef __APPLE__
            Gui::GuiScale = (width > 2000)? 8.0f : 4.0f;
        #else
            Gui::GuiScale = 4.0f;
        #endif
    }
	else if (width >= 800) {
#ifdef __APPLE__
        Gui::GuiScale = 4.0f;
#else
		Gui::GuiScale = 3.0f;
#endif
    }
	else if (width >= 400)
		Gui::GuiScale = 2.0f;
	else
		Gui::GuiScale = 1.0f;

	Gui::InvGuiScale = 1.0f / Gui::GuiScale;
	int screenWidth  = (int)(width  * Gui::InvGuiScale);
	int screenHeight = (int)(height * Gui::InvGuiScale);

	if (platform()) {
		float pixelsPerMillimeter = options.getProgressValue(&Options::Option::PIXELS_PER_MILLIMETER);
		pixelCalc.setPixelsPerMillimeter(pixelsPerMillimeter);
		pixelCalcUi.setPixelsPerMillimeter(pixelsPerMillimeter * Gui::InvGuiScale);
	}

	Config config = createConfig(this);
	gui.onConfigChanged(config);
    
	if (screen)
		screen->setSize(screenWidth, screenHeight);

	if (inputHolder)
		inputHolder->onConfigChanged(config);
	//LOGI("Setting size: %d, %d: %f\n", width, height, Gui::InvGuiScale);

#ifdef WIN32
	char resbuf[128];
	sprintf(resbuf, "            %d x %d @ scale %.2f", width, height, Gui::GuiScale);
	//gui.addMessage(resbuf);
#endif
#endif /* STANDALONE_SERVER */
}

void Minecraft::reloadOptions() {
	options.update();
	// Sync the renderer's AO switch from the option. The static defaults to
	// false and only the PC KEY_O toggle changed it before, so AO never
	// turned on on Android even when options.ambientOcclusion was true.
	useAmbientOcclusion = options.ambientOcclusion;
	options.save();
	bool wasTouchscreen = options.useTouchScreen;
	options.useTouchScreen = useTouchscreen();
	options.save();

	if ((wasTouchscreen != options.useTouchScreen) || (inputHolder == 0))
		_reloadInput();

	user->name = options.username;

	LOGI("Reloading-options\n");

    // @todo @fix Android and iOS behaves a bit differently when leaving
    //            an options screen (Android recreates OpenGL surface)
    setSize(width, height);
}

void Minecraft::_reloadInput() {
#ifndef STANDALONE_SERVER
	delete inputHolder;

	if (useTouchscreen()) {
		inputHolder = new TouchInputHolder(this, &options);
	} else {
		#if defined(ANDROID) || defined(__APPLE__) 
			inputHolder = new CustomInputHolder(
				new XperiaPlayInput(&options),
				new ControllerTurnInput(2, ControllerTurnInput::MODE_DELTA),
				new IBuildInput());
		#else
			inputHolder = new CustomInputHolder(
				new KeyboardInput(&options),
				new MouseTurnInput(MouseTurnInput::MODE_DELTA, width/2, height/2),
				new MouseBuildInput());
		#endif
	}

	mouseHandler.setTurnInput(inputHolder->getTurnInput());
	if (level && player) {
		player->input = inputHolder->getMoveInput();
	}
	Mouse::reset2();
#endif
}


//
// Multiplayer
//
void Minecraft::locateMultiplayer() {
	isLookingForMultiplayer = true;

	raknetInstance->pingForHosts(19132);
	netCallback = new ClientSideNetworkHandler(this, raknetInstance);
}

void Minecraft::cancelLocateMultiplayer() {
	isLookingForMultiplayer = false;

	raknetInstance->stopPingForHosts();

	delete netCallback;
	netCallback = NULL;
}

bool Minecraft::joinMultiplayer( const PingedCompatibleServer& server )
{
	if (isLookingForMultiplayer && netCallback) {
		isLookingForMultiplayer = false;
		return raknetInstance->connect(server.address.ToString(false), server.address.GetPort());
	}
	return false;
}

void Minecraft::hostMultiplayer(int port) {
    // Tear down last instance
    raknetInstance->disconnect();
    delete netCallback;
    netCallback = NULL;

#if !defined(NO_NETWORK)
	netCallback = new ServerSideNetworkHandler(this, raknetInstance);
    #ifdef STANDALONE_SERVER
        raknetInstance->host(user->name, port, 16);
    #else
        raknetInstance->host(user->name, port);
    #endif
#endif
}

//
// Level generation
//
/*static*/

	void* Minecraft::prepareLevel_tspawn(void *p_param)
{
	Minecraft* mc = (Minecraft*) p_param;
	mc->generateLevel("Currently not used", mc->level);
	return 0;
}

void Minecraft::generateLevel( const std::string& message, Level* level )
{
	Stopwatch s;
	s.start();
	prepareLevel(message);
	s.stop();
	s.print("Level generated: ");

	// "Unlock"
	isGeneratingLevel = false;
}

void Minecraft::_levelGenerated()
{
#ifndef STANDALONE_SERVER
	if (player == NULL) {
		player = (LocalPlayer*) gameMode->createPlayer(level);
		gameMode->initPlayer(player);
	}

	if (player) {
		player->input = inputHolder->getMoveInput();
	}

	if (levelRenderer != NULL) levelRenderer->setLevel(level);
	if (particleEngine != NULL) particleEngine->setLevel(level);

	gameMode->adjustPlayer(player);
	gui.onLevelGenerated();
#endif

	level->validateSpawn();
	level->loadPlayer(player, true);
	// if we are client side, we trust the server to have given us a correct position
	if (player && !level->isClientSide) {
		player->resetPos(false);
	}

	// v21: after a dimension switch, move the player into the
	// destination portal (found nearby, or freshly built) once
	// generation has finished.
	if (pendingPortalBuild && player) {
		pendingPortalBuild = false;
		findOrCreatePortal(player->x, player->y, player->z);
	}

	// v22: after entering the End, spawn the dragon (once per world
	// — see ensureEndDragon) once generation has finished and the
	// island actually exists.
	if (pendingEndDragon && player) {
		pendingEndDragon = false;
		ensureEndDragon();
	}

	// v22: the player died in the Nether/End and asked to respawn;
	// the switch back to the overworld just finished generating, so
	// the respawn itself runs now, at the overworld spawn/bed.
	if (pendingRespawnAfterSwitch && player) {
		pendingRespawnAfterSwitch = false;
		resetPlayer(player);
	}

	this->cameraTargetPlayer = player;

	if (raknetInstance->isServer())
		raknetInstance->announceServer(user->name);

	if (netCallback) {
		netCallback->levelGenerated(level);
	}

#if defined(WIN32) || defined(RPI)
	if (_commandServer) {
		delete _commandServer;
	}
	_commandServer = new CommandServer(this);
	_commandServer->init(commandPort);
#endif

	// Hack to (hopefully) get the players to show (note: in LevelListener
	// instead, since adding yourself always generates a entityAdded)
	//EntityRenderDispatcher::getInstance()->onGraphicsReset();
	_hasSignaledGeneratingLevelFinished = true;
}

Player* Minecraft::respawnPlayer(int playerId) {
	for (unsigned int i = 0; i < level->players.size(); ++i) {
		if (level->players[i]->entityId == playerId) {
			resetPlayer(level->players[i]);
			return level->players[i];
		}
	}
	return NULL;
}

void Minecraft::resetPlayer(Player* player) {
	level->validateSpawn();
	player->reset();

	Pos p;
	if(player->hasRespawnPosition()) {
		p = player->getRespawnPosition();
	}
	else {
		p = level->getSharedSpawnPos();
	}
	player->setPos((float)p.x + 0.5f, (float)p.y + 1.0f, (float)p.z + 0.5f);
	player->resetPos(true);

	if (isCreativeMode())
		player->inventory->clearInventoryWithDefault();
}

void Minecraft::respawnPlayer() {
	// v22: dying in the Nether/End respawns the player in the
	// OVERWORLD (modern MC), not in the dimension. The switch runs
	// first (deferred generation); the respawn itself runs in
	// _levelGenerated() via pendingRespawnAfterSwitch, once the
	// overworld is live again. If the switch cannot even be
	// requested (e.g. still generating), fall through to the old
	// in-dimension respawn instead of stranding the player.
	if (level != NULL && level->dimension != NULL
			&& overworldLevel != NULL
			&& level->dimension->id != overworldLevel->dimension->id) {
		pendingRespawnAfterSwitch = true;
		requestDimensionChange(overworldLevel->dimension->id);
		if (pendingDimensionSwitch) return;
		pendingRespawnAfterSwitch = false;
	}

	resetPlayer(player);

	// tell server (or other client) that we re-spawned
	RespawnPacket packet(player);
	raknetInstance->send(packet);
}

void Minecraft::onGraphicsReset()
{
#ifndef STANDALONE_SERVER
	textures->clear();
	
	font->onGraphicsReset();
	gui.onGraphicsReset();

	if (levelRenderer) levelRenderer->onGraphicsReset();
	if (gameRenderer) gameRenderer->onGraphicsReset();

	EntityRenderDispatcher::getInstance()->onGraphicsReset();
	TileEntityRenderDispatcher::getInstance()->onGraphicsReset();
#endif
}

int Minecraft::getProgressStatusId() {
	return progressStageStatusId;
}

const char* Minecraft::getProgressMessage()
{
	return progressMessages[progressStageStatusId];
}

bool Minecraft::isLevelGenerated()
{
	return level != NULL && !isGeneratingLevel;
}

LevelStorageSource* Minecraft::getLevelSource()
{
	return storageSource;
}

int Minecraft::getLicenseId() {
	if (!LicenseCodes::isReady(_licenseId))
		_licenseId = platform()->checkLicense();
	return _licenseId;
}

void Minecraft::audioEngineOn() {
#ifndef STANDALONE_SERVER
    soundEngine->enable(true);
#endif
}
void Minecraft::audioEngineOff() {
#ifndef STANDALONE_SERVER
    soundEngine->enable(false);
#endif
}

void Minecraft::setIsCreativeMode(bool isCreative)
{
#ifdef CREATORMODE
	delete gameMode;
	gameMode = new CreatorMode(this);
	_isCreativeMode = true;
#else
	if (!gameMode || isCreative != _isCreativeMode)
	{
		delete gameMode;
		if (isCreative) gameMode = new CreativeMode(this);
		else			gameMode = new SurvivalMode(this);
		_isCreativeMode = isCreative;
	}
#endif
	if (player)
		gameMode->initAbilities(player->abilities);
}

bool Minecraft::isCreativeMode() {
	return _isCreativeMode;
}

bool Minecraft::isKindleFire(int kindleVersion) {
	if (kindleVersion != 1)
		return false;

	std::string model = platform()->getPlatformStringVar(PlatformStringVars::DEVICE_BUILD_MODEL);
	std::string modelLower(model);
	std::transform(modelLower.begin(), modelLower.end(), modelLower.begin(), tolower);

	return (modelLower.find("kindle") != std::string::npos) && (modelLower.find("fire") != std::string::npos);
}

bool Minecraft::transformResolution(int* w, int* h)
{
	bool changed = false;

	// Kindle Fire 1: reporting wrong height in
	// certain cases (e.g. after screen lock)
	if (isKindleFire(1) && *h >= 560 && *h <= 620) {
		*h = 580;
		changed = true;
	}

	return changed;
}

ICreator* Minecraft::getCreator()
{
#ifdef CREATORMODE
	return ((CreatorMode*)gameMode)->getCreator();
#else
	return NULL;
#endif
}

void Minecraft::optionUpdated( const Options::Option* option, bool value ) {
	if(netCallback != NULL && option == &Options::Option::SERVER_VISIBLE) {
		ServerSideNetworkHandler* ss = (ServerSideNetworkHandler*) netCallback;
		ss->allowIncomingConnections(value);
	}
}

void Minecraft::optionUpdated( const Options::Option* option, float value ) {
#ifndef STANDALONE_SERVER
	if(option == &Options::Option::PIXELS_PER_MILLIMETER) {
		pixelCalcUi.setPixelsPerMillimeter(value * Gui::InvGuiScale);
		pixelCalc.setPixelsPerMillimeter(value);
	}
#endif
}

void Minecraft::optionUpdated( const Options::Option* option, int value ) {

}
