#include "MobSpawnerTileEntity.h"

#include "../../Level.h"
#include "../../../entity/monster/Blaze.h"
#include "../../../entity/EntityTypes.h"
#include "../../../entity/player/Player.h"
#include "../../../phys/AABB.h"
#include "../../../../nbt/CompoundTag.h"

MobSpawnerTileEntity::MobSpawnerTileEntity()
:	super(TileEntityType::MobSpawner),
	delay(MIN_DELAY)
{
}

bool MobSpawnerTileEntity::shouldSave()
{
	// Always: the spawner must survive quit/reload with its delay
	// intact (Sign precedent — unlike a chest/furnace, "empty" here
	// is not a state that can be re-created for free).
	return true;
}

void MobSpawnerTileEntity::load(CompoundTag* base)
{
	super::load(base);
	delay = base->getInt("Delay");
	if (delay < 0) delay = MIN_DELAY;
	if (delay > MAX_DELAY) delay = MAX_DELAY;
}

bool MobSpawnerTileEntity::save(CompoundTag* base)
{
	if (!super::save(base))
		return false;
	base->putInt("Delay", delay);
	return true;
}

void MobSpawnerTileEntity::tick()
{
	if (level == NULL || level->isClientSide) return;

	// Inactive with no player within 16 blocks: the delay freezes
	// (modern MC) instead of counting down while nobody is near.
	bool playerNear = false;
	for (size_t i = 0; i < level->players.size(); ++i) {
		Player* p = level->players[i];
		if (p != NULL && distanceToSqr(p->x, p->y, p->z) <= (float)(ACTIVE_RADIUS * ACTIVE_RADIUS)) {
			playerNear = true;
			break;
		}
	}
	if (!playerNear) return;

	if (--delay > 0) return;

	// Attempt now; the delay re-rolls below whatever happens, so a
	// full room retries on the same 10-20 s cadence instead of
	// spinning every tick.
	delay = MIN_DELAY + level->random.nextInt(MAX_DELAY - MIN_DELAY + 1);

	// Cap: no more spawns while MAX_NEARBY blazes are already in
	// the box around this spawner.
	AABB box((float)(x - SPAWN_RADIUS), (float)(y - 1), (float)(z - SPAWN_RADIUS),
		(float)(x + SPAWN_RADIUS + 1), (float)(y + 3), (float)(z + SPAWN_RADIUS + 1));
	EntityList list;
	if (level->getEntitiesOfType(MobTypes::Blaze, box, list) >= MAX_NEARBY) return;

	// One blaze at a random air spot within +-4 blocks, at the
	// spawner level or one above. A few tries: the room is small
	// and the spawner block / chest / walls are solid.
	for (int t = 0; t < 4; ++t) {
		int sx = x + level->random.nextInt(SPAWN_RADIUS * 2 + 1) - SPAWN_RADIUS;
		int sz = z + level->random.nextInt(SPAWN_RADIUS * 2 + 1) - SPAWN_RADIUS;
		int sy = y + level->random.nextInt(2);
		if (level->isSolidBlockingTile(sx, sy, sz)) continue;
		// The spot above must be air too (a blaze is ~1.8 tall);
		// isSolidBlockingTile covers the feet, check the head.
		if (level->isSolidBlockingTile(sx, sy + 1, sz)) continue;

		Blaze* b = new Blaze(level);
		b->moveTo((float)sx + 0.5f, (float)sy, (float)sz + 0.5f,
			level->random.nextFloat() * 360.0f, 0.0f);
		level->addEntity(b);
		return;
	}
}
