#include "EndCrystal.h"

#include "../../level/Level.h"
#include "../../level/tile/Tile.h"
#include "../../level/levelgen/EndLevelSource.h"
#include "../monster/EnderDragon.h"
#include "../Mob.h"
#include "../../../util/Mth.h"

EndCrystal::EndCrystal(Level* level)
:	super(level),
	exploding(false)
{
	entityRendererId = ER_END_CRYSTAL_RENDERER;
	setSize(2.0f, 2.0f);
	fireImmune = true;
}

EndCrystal::EndCrystal(Level* level, float x, float y, float z)
:	super(level),
	exploding(false)
{
	// Same init as the 1-arg ctor (no delegating ctors in C++98).
	entityRendererId = ER_END_CRYSTAL_RENDERER;
	setSize(2.0f, 2.0f);
	fireImmune = true;
	setPos(x, y, z);
}

void EndCrystal::tick()
{
	// No baseTick: the crystal never falls, burns, drowns or takes
	// fall damage — it floats exactly where it was placed. Keep only
	// the frame bookkeeping the renderer spins / bobs from.
	tickCount++;
	xo = x;
	yo = y;
	zo = z;
	xRotO = xRot;
	yRotO = yRot;

	if (level->isClientSide || removed) return;
	if (tickCount % 10 == 0) tryHealDragon();
	if (tickCount % 20 == 0) {
		// Support gone (bedrock/obsidian can't break in survival,
		// but a regenerated chunk must not leave a floater): vanish
		// quietly, without exploding.
		int sx = Mth::floor(x);
		int sy = Mth::floor(y) - 1;
		int sz = Mth::floor(z);
		int t = level->getTile(sx, sy, sz);
		if (t != Tile::unbreakable->id && t != Tile::obsidian->id) {
			remove();
			return;
		}
		tryRespawnDragon();
	}
}

bool EndCrystal::isPickable()
{
	return !removed;
}

void EndCrystal::addAdditonalSaveData(CompoundTag* tag)
{
	(void) tag; // no extra state — Pos/Motion/Rotation suffice
}

void EndCrystal::readAdditionalSaveData(CompoundTag* tag)
{
	(void) tag; // no extra state
}

bool EndCrystal::hurt(Entity* source, int damage)
{
	(void) source;
	(void) damage;
	if (removed || exploding) return false;
	markHurt();
	if (!level->isClientSide) {
		// Guard BEFORE exploding: the blast's own entity scan calls
		// hurt() on everything in radius, including this crystal —
		// without the flag that would recurse until the stack dies.
		// Power 6, no fire — modern MC's crystal explosion. The
		// blast respects explosion resistance, so the obsidian
		// pillars (2000) and bedrock survive it.
		exploding = true;
		level->explode(NULL, x, y, z, 6.0f, false);
	}
	remove();
	return true;
}

/*static*/
bool EndCrystal::isOnRespawnSpot(Level* level, float x, float y, float z)
{
	int bx = Mth::floor(x);
	int by = Mth::floor(y) - 1;
	int bz = Mth::floor(z);
	if (level->getTile(bx, by, bz) != Tile::unbreakable->id) return false;
	return EndLevelSource::isPillarTop(bx, by, bz)
		|| EndLevelSource::isExitRingCell(bx, by, bz);
}

void EndCrystal::tryHealDragon()
{
	// Nearest live dragon within 32 blocks, +1 HP (2 HP/s). The
	// heal goes through Mob::heal so the dragon's override (cap 200,
	// not Mob's 20) applies.
	Mob* nearest = NULL;
	float nearestD2 = 32.0f * 32.0f;
	for (unsigned int i = 0; i < level->entities.size(); i++) {
		Entity* e = level->entities[i];
		if (e == NULL || e->removed
			|| e->getEntityTypeId() != MobTypes::EnderDragon) continue;
		float dx = e->x - x;
		float dy = e->y - y;
		float dz = e->z - z;
		float d2 = dx * dx + dy * dy + dz * dz;
		if (d2 < nearestD2) {
			nearestD2 = d2;
			nearest = (Mob*) e;
		}
	}
	if (nearest != NULL) nearest->heal(1);
}

void EndCrystal::tryRespawnDragon()
{
	// Only after a dragon has been defeated: the exit portal tile at
	// the platform centre is the "already defeated" flag (same flag
	// ensureEndDragon reads). During the first fight there is no
	// portal, so crystals placed then can never trigger this.
	if (level->getTile(0, EndLevelSource::EXIT_PORTAL_Y, 0) != Tile::endPortal->id) return;
	// A dragon is already fighting: nothing to respawn.
	for (unsigned int i = 0; i < level->entities.size(); i++) {
		Entity* e = level->entities[i];
		if (e != NULL && !e->removed
			&& e->getEntityTypeId() == MobTypes::EnderDragon) return;
	}
	// Count the crystals standing on respawn spots (pillar caps +
	// exit-portal ring). Four or more resummon the dragon.
	EndCrystal* spots[16];
	int n = 0;
	for (unsigned int i = 0; i < level->entities.size() && n < 16; i++) {
		Entity* e = level->entities[i];
		if (e == NULL || e->removed
			|| e->getEntityTypeId() != EntityTypes::IdEndCrystal) continue;
		if (isOnRespawnSpot(level, e->x, e->y, e->z)) {
			spots[n++] = (EndCrystal*) e;
		}
	}
	if (n < 4) return;
	// Consume the triggering crystals (modern MC consumes the four),
	// clear the exit portal, and spawn a fresh dragon at full HP.
	// remove() only marks them; Level::tickEntities deletes them on
	// its next pass, so removing several mid-iteration is safe.
	for (int i = 0; i < n; i++) spots[i]->remove();
	level->setTile(0, EndLevelSource::EXIT_PORTAL_Y, 0, 0);
	EnderDragon* dragon = new EnderDragon(level);
	dragon->setPos(0.5f, 82.0f, 0.5f);
	level->addEntity(dragon);
	level->playSound(0.5, 82.0, 0.5, "random.explode", 1.0f, 0.5f);
}
