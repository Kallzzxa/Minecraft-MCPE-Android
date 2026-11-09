#include "ThrownPotion.h"

#include "../EntityTypes.h"
#include "../Mob.h"
#include "../../item/PotionItem.h"
#include "../../level/Level.h"
#include "../../../nbt/CompoundTag.h"
#include "../../phys/HitResult.h"
#include "../../phys/AABB.h"

ThrownPotion::ThrownPotion(Level* level)
:	super(level),
	potionAux(PotionItem::POISON),
	splashOwner(false)
{
	entityRendererId = ER_THROWN_POTION_RENDERER;
	setSize(0.25f, 0.25f);
}

ThrownPotion::ThrownPotion(Level* level, Mob* mob)
:	super(level, mob),
	potionAux(PotionItem::POISON),
	splashOwner(false)
{
	entityRendererId = ER_THROWN_POTION_RENDERER;
	setSize(0.25f, 0.25f);
}

ThrownPotion::ThrownPotion(Level* level, Mob* mob, int potionAux)
:	super(level, mob),
	// v26: strip ONLY the splash bit — the redstone / glowstone
	// variant flags survive the throw, so a splash potion of
	// swiftness II still applies level II on impact (v25's
	// baseAux() here would have thrown the variant away).
	potionAux(PotionItem::stripSplash(potionAux)),
	splashOwner(false)
{
	entityRendererId = ER_THROWN_POTION_RENDERER;
	setSize(0.25f, 0.25f);
}

ThrownPotion::ThrownPotion(Level* level, Mob* mob, int potionAux, bool splashOwner)
:	super(level, mob),
	// v26: same as above — keep the upgrade variant flags.
	potionAux(PotionItem::stripSplash(potionAux)),
	splashOwner(splashOwner)
{
	entityRendererId = ER_THROWN_POTION_RENDERER;
	setSize(0.25f, 0.25f);
}

void ThrownPotion::onHit(const HitResult& res)
{
	(void) res;
	if (!level->isClientSide) {
		// Real splash: every mob within 3 blocks of the impact
		// gets the effect — a potion landing at the player's feet
		// still works, and nearby mobs can be caught in it too.
		// (The thrower is skipped, UNLESS this is a player throw
		// — v25: a player CAN splash themselves, modern MC.)
		AABB box(x - 3.0f, y - 3.0f, z - 3.0f, x + 3.0f, y + 3.0f, z + 3.0f);
		EntityList& entities = level->getEntities(this, box);
		for (unsigned int i = 0; i < entities.size(); i++) {
			Entity* e = entities[i];
			if (!e->isMob()) continue;
			if (e->entityId == ownerId && !splashOwner) continue;
			PotionItem::applyEffect(potionAux, (Mob*)e);
		}
	}
	// Glass-break sound (registered in v19) + splash particles,
	// both sides so the throw is visible on the client too.
	// v47: the splash sprite is the real blue droplet
	// (particles.png tex 17, SplashParticle) — tex 33 was the
	// fishing bobber, so the burst used to explode into bobbers.
	// v48: the potion aux rides along as the particle `data` int,
	// so the burst tints per potion type (SplashParticle tex 18,
	// white droplet shape + bottle-liquid tint — tinting the blue
	// tex 17 could never make pink/red). Burst count, velocities,
	// gravity, lifetime and size are unchanged.
	level->playSound(this, "random.glass", 1.0f, 1.0f);
	for (int i = 0; i < 8; i++) {
		level->addParticle(PARTICLETYPE(splash), x, y, z,
			(level->random.nextFloat() - 0.5f) * 0.2f,
			level->random.nextFloat() * 0.2f,
			(level->random.nextFloat() - 0.5f) * 0.2f,
			potionAux);
	}
	if (!level->isClientSide) remove();
}

void ThrownPotion::addAdditonalSaveData(CompoundTag* tag)
{
	super::addAdditonalSaveData(tag);
	tag->putByte("PotionAux", (char) potionAux);
	tag->putByte("SplashOwner", (char) (splashOwner ? 1 : 0));
}

// v48: MP sync — pack the base potion aux into the AddEntityPacket
// data int (see the header comment for the layout). The client's
// copy is built by the default ctor (POISON), so without this the
// flying bottle and the burst tint would always read poison.
int ThrownPotion::getAuxData()
{
	return getOwnerId() | ((PotionItem::baseAux(potionAux) & 0xf) << 24);
}

void ThrownPotion::setPotionAux(int aux)
{
	potionAux = PotionItem::stripSplash(aux);
}

void ThrownPotion::readAdditionalSaveData(CompoundTag* tag)
{
	super::readAdditionalSaveData(tag);
	if (tag->contains("PotionAux")) potionAux = tag->getByte("PotionAux") & 0xff;
	if (tag->contains("SplashOwner")) splashOwner = tag->getByte("SplashOwner") != 0;
}
