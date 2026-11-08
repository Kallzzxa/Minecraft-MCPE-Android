#ifndef NET_MINECRAFT_CLIENT__Minecraft_H__
#define NET_MINECRAFT_CLIENT__Minecraft_H__

#include "Options.h"
#ifndef STANDALONE_SERVER
#include "MouseHandler.h"
#endif
#include "Timer.h"
#include "player/input/ITurnInput.h"
#ifndef STANDALONE_SERVER
#include "gui/Gui.h"
#include "gui/screens/ScreenChooser.h"
#endif
//#include "../network/RakNetInstance.h"
#include "../world/phys/HitResult.h"

class User;
class Level;
class LocalPlayer;
class IInputHolder;
class Mob;
class Player;
class LevelRenderer;
class GameRenderer;
class ParticleEngine;
class Entity;
class ICreator;
class GameMode;
class Textures;
class CThread;
class SoundEngine;
class Screen;
class Font;
class LevelStorageSource;
class BuildActionIntention;
class PerfRenderer;
class LevelSettings;
class IRakNetInstance;
class NetEventCallback;
class CommandServer;
struct PingedCompatibleServer;
//class ExternalFileLevelStorageSource;


#include "../App.h"
#include "PixelCalc.h"
class AppPlatform;
class AppPlatform_android;

class Minecraft: public App
{
protected:
	Minecraft();
public:
	virtual ~Minecraft();

	void init();
	void setSize(int width, int height);
	void reloadOptions();

	bool supportNonTouchScreen();
	bool useTouchscreen();
	void grabMouse();
	void releaseMouse();

	void handleBuildAction(BuildActionIntention*);

	void toggleDimension();
	bool isCreativeMode();
	void setIsCreativeMode(bool isCreative);
	void setScreen(Screen*);

	virtual void selectLevel(const std::string& levelId, const std::string& levelName, const LevelSettings& settings);
	virtual void setLevel(Level* level, const std::string& message = "", LocalPlayer* forceInsertPlayer = NULL);

	void generateLevel( const std::string& message, Level* level );
	LevelStorageSource* getLevelSource();

	bool isLookingForMultiplayer;
	void locateMultiplayer();
	void cancelLocateMultiplayer();
	bool joinMultiplayer(const PingedCompatibleServer& server);
	void hostMultiplayer(int port=19132);
	Player* respawnPlayer(int playerId);
	void respawnPlayer();
	void resetPlayer(Player* player);
	void doActuallyRespawnPlayer();

	void update();

	void tick(int nTick, int maxTick);
	void tickInput();

	bool isOnlineClient();
	bool isOnline();
	void pauseGame(bool isBackPaused);
	void gameLostFocus();

	void prepareLevel(const std::string& message);

	void cleanupLastJoinedServerFolders();
	void leaveGame(bool renameLevel = false);

	int getProgressStatusId();
	const char* getProgressMessage();

	ICreator* getCreator();

	// void onGraphicsLost() {}
	void onGraphicsReset();

	bool isLevelGenerated();
	int getLicenseId();

    void audioEngineOn();
    void audioEngineOff();
    
	bool isPowerVR() { return _powerVr; }
	bool isKindleFire(int kindleVersion);
	bool transformResolution(int* w, int* h);
	void optionUpdated(const Options::Option* option, bool value);
	void optionUpdated(const Options::Option* option, float value);
	void optionUpdated(const Options::Option* option, int value);
#ifdef __APPLE__
    bool _isSuperFast;
    bool isSuperFast() { return _isSuperFast; }
#endif

protected:
	void _levelGenerated();

private:
	static void* prepareLevel_tspawn(void *p_param);

	void handleMouseClick(int button);
	void handleMouseDown(int button, bool down);

	void _reloadInput();
public:
	int width;
	int height;

	// Vars that the platform is allowed to use in the future
	int commandPort;
	int reserved_d1, reserved_d2;
	float reserved_f1, reserved_f2;

	Options options;

	static bool useAmbientOcclusion;
	//static bool threadInterrupt;

	volatile bool pause;

	LevelRenderer*  levelRenderer;
	GameRenderer*   gameRenderer;
	ParticleEngine* particleEngine;
	SoundEngine*    soundEngine;

	GameMode* gameMode;
#ifndef STANDALONE_SERVER
	Textures* textures;
	ScreenChooser screenChooser;
	Font* font;
#endif
	IRakNetInstance*  raknetInstance;
	NetEventCallback* netCallback;

	int lastTime;
	int lastTickTime;
	float ticksSinceLastUpdate;

	User*  user;
	Level* level;
	// v21: Nether dimension switching. overworldLevel/netherLevel are
	// the two Level objects of the current world; `level` points at
	// whichever is active. The Nether level is created on first entry
	// (storage <world>/DIM-1) and kept alive for the session, so
	// switching back and forth never reloads it from scratch.
	Level* overworldLevel;
	Level* netherLevel;
	// The level id (world folder name) the Nether storage id is
	// derived from (<id>_nether).
	std::string currentLevelId;
	// A switch requested by LocalPlayer::tick (standing in a portal)
	// is deferred to the top of update(), never run inside a level
	// tick — see changeDimension().
	bool pendingDimensionSwitch;
	int pendingDimension;
	// Where the player entered the Nether from (return position).
	bool hasReturnPos;
	float returnX, returnY, returnZ;
	// Ticks until portal teleporting is allowed again (prevents
	// instantly bouncing back through the destination portal).
	int portalCooldownTicks;
	// After a switch, the destination portal is found or built in
	// _levelGenerated(), once generation has finished.
	bool pendingPortalBuild;

	// v21: request a switch to the other dimension (guards: must be
	// single-player, not generating, cooldown elapsed). The switch
	// itself runs in update() via changeDimension().
	void requestDimensionChange(int targetDimension);
	// v21: perform the switch now (caller must be outside a level
	// tick and isGeneratingLevel must be false).
	void changeDimension(int targetDimension);
	// v21: return the existing Nether level, or create it (pre-seeded
	// level.dat at the arrival portal, own storage under DIM-1).
	Level* getOrCreateNetherLevel(int nx, int nz);
	// v21: after generation, move the player into an existing portal
	// near (px, py, pz), or build a fresh 4x5 obsidian frame there.
	void findOrCreatePortal(float px, float py, float pz);

	LocalPlayer*	player;
	IInputHolder*	inputHolder;
	Mob*			cameraTargetPlayer;
#ifndef STANDALONE_SERVER
	Gui gui;
#endif
	CThread* generateLevelThread;
	Screen* screen;
	static int customDebugId;

	static const int CDI_NONE = 0;
	static const int CDI_GRAPHICS = 1;
#ifndef STANDALONE_SERVER
	MouseHandler mouseHandler;
#endif
	bool mouseGrabbed;

    PixelCalc pixelCalc;
    PixelCalc pixelCalcUi;

	HitResult hitResult;
	volatile int progressStagePercentage;

	// This field is initialized in main()
	// It sets the base path to where worlds can be written (sdcard on android)
	std::string externalStoragePath;
	std::string externalCacheStoragePath;
protected:
	Timer timer;
    // @note @attn @warn: this is dangerous as fuck!
	volatile bool isGeneratingLevel;
	bool _hasSignaledGeneratingLevelFinished;

	LevelStorageSource* storageSource;
	bool _running;
	bool _powerVr;

private:
	volatile int progressStageStatusId;
	static const char* progressMessages[];

	int missTime;
	int ticks;
	bool screenMutex;
	bool hasScheduledScreen;
	Screen* scheduledScreen;

	int _licenseId;
	bool _supportsNonTouchscreen;

	bool _isCreativeMode;
	//int _respawnPlayerTicks;
	Player* _pendingRemovePlayer; // @attn @todo @fix: remove this shait and fix the respawn behaviour

	PerfRenderer* _perfRenderer;
	CommandServer* _commandServer;
};

#endif /*NET_MINECRAFT_CLIENT__Minecraft_H__*/
