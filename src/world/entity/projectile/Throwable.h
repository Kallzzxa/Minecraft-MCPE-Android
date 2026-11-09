#ifndef NET_MINECRAFT_WORLD_ENTITY_PROJECTILE__Throwable_H__
#define NET_MINECRAFT_WORLD_ENTITY_PROJECTILE__Throwable_H__

//package net.minecraft.world.entity->projectile;

class Level;
class Mob;
class Player;
class CompoundTag;

#include "../Entity.h"

class HitResult;

/*abstract*/
class Throwable: public Entity
{
	typedef Entity super;
public:
    Throwable(Level* level);
	Throwable(Level* level, Mob* mob);
	Throwable(Level* level, float x, float y, float z);

    void shoot(const Vec3& v, float pow, float uncertainty);
    void shoot(float xd, float yd, float zd, float pow, float uncertainty);

	void tick();
    void lerpMotion(float xd, float yd, float zd);

    // v40: the mob that shot this projectile (its runtime entityId,
    // set by the (Level, Mob) ctor; 0 = no owner, e.g. a fireball
    // rebuilt from a save). Session-only by design — entityIds are
    // a per-run counter, NOT saved (a saved id would point at a
    // different entity after reload), and a fireball only lives a
    // few seconds anyway. This is what lets a ghast/blaze be immune
    // to its OWN fireball while still taking damage from a reflected
    // one or another mob's: any future reflection code MUST reassign
    // ownerId to the deflector (or clear it), or the reflected ball
    // would still count as the shooter's own.
    int getOwnerId() const { return ownerId; }
    // v48: network sync only — ClientSideNetworkHandler restores
    // the owner on the client's copy of a thrown potion from the
    // AddEntityPacket data (see ThrownPotion::getAuxData), so the
    // client's copy keeps the owner's early-flight immunity and
    // does not burst on its thrower.
    void setOwnerId(int id) { ownerId = id; }

    void addAdditonalSaveData(CompoundTag* tag);
    void readAdditionalSaveData(CompoundTag* tag);

	bool shouldRenderAtSqrDistance(float distance);

	float getShadowHeightOffs();

    int getAuxData();
protected:
	virtual float getThrowPower();
	virtual float getThrowUpAngleOffset();

	virtual float getGravity();

	virtual void onHit(const HitResult& res) = 0;

private:
	void _init();

public:
	int shakeTime;
protected:
	bool inGround;
	int ownerId;
private:
	int life;
	int flightTime;
	int xTile;
	int yTile;
	int zTile;
	int lastTile;
};

#endif /*NET_MINECRAFT_WORLD_ENTITY_PROJECTILE__Throwable_H__*/
