#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__MobSpawnerTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__MobSpawnerTile_H__

// v39: monster spawner block (tile id 52 — modern MC). An EntityTile
// like the furnace/brewing stand: placing the block creates the
// MobSpawnerTileEntity via EntityTile::onPlace (LevelChunk::setTile
// calls onPlace), breaking it removes the entity via onRemove, and
// the entity spawns blazes around itself (see
// MobSpawnerTileEntity.h). Rendering is a plain cube with the drawn
// cage texture (terrain.png cell 251 — verified unused: no tile tex,
// not in the ItemRenderer mapper, not a destroy stage (240..249),
// not a dynamic texture target; same cube+texture precedent as the
// v10 enchanting table / v20 brewing stand / v25 cauldron). There is
// no custom non-cube tile renderer in this tree to build the modern
// see-through cage with a spinning mob inside, so the cube + cage
// texture is the honest v39 shape.
//
// Breaking drops NOTHING (getResource returns 0 — modern MC: a
// spawner only drops itself with silk touch, and there is no silk
// touch in this tree). That also means fortress spawners cannot be
// farmed by breaking them.

#include "EntityTile.h"

class Level;
class Player;
class Random;

class MobSpawnerTile: public EntityTile
{
	typedef EntityTile super;
public:
	MobSpawnerTile(int id);

	int getResource(int data, Random* random);
	TileEntity* newTileEntity();
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_TILE__MobSpawnerTile_H__*/
