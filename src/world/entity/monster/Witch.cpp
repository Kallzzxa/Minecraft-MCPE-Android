#include "Witch.h"

#include "../../item/Item.h"
#include "../../item/PotionItem.h"
#include "../../level/Level.h"
#include "../../../util/Mth.h"
#include "../../../SharedConstants.h"
#include "../projectile/ThrownPotion.h"

Witch::Witch(Level* level)
:	super(level),
	drinkCooldown(0)
{
	entityRendererId = ER_WITCH_RENDERER;
	textureName = "mob/witch.png";
	setSize(0.6f, 1.95f);
	runSpeed = 0.5f;
	attackDamage = 0;
}

void Witch::tick()
{
	super::tick();
	if (drinkCooldown > 0) drinkCooldown--;
	// Modern-MC behaviour: drink a healing potion when badly hurt.
	// Bounded by the cooldown so the witch cannot out-heal the
	// player forever; the drink is silent-ish (random.drink is
	// mapped in v19) and the heal is instant, like the potion.
	if (!level->isClientSide && health > 0 && health <= 12 && drinkCooldown == 0) {
		heal(4);
		level->playSound(this, "random.drink", 0.5f, 1.0f);
		drinkCooldown = 200; // 10 s between drinks
	}
}

void Witch::checkHurtTarget(Entity* target, float d)
{
	// Ranged attack, skeleton-style: one aimed splash potion every
	// 3 s while the target is within 16 blocks. No melee at all.
	if (d >= 16) return;

	float xd = target->x - x;
	float zd = target->z - z;
	yRot = (float)(std::atan2(zd, xd) * Mth::RADDEG) - 90;

	if (d < 6) {
		// Too close: back away from the target instead of closing
		// in (modern witches keep ~10 blocks). The path is only
		// re-plotted when empty, so updateAi's own re-path (which
		// heads TOWARD the target) does not fight it every tick;
		// once the away-path is walked the witch is out of range
		// and holds ground again. (PathfinderMob::path is private;
		// the public isPathFinding/setPath pair is the way in.)
		if (!isPathFinding()) {
			int ax = Mth::floor(x + (x - target->x));
			int az = Mth::floor(z + (z - target->z));
			Path awayPath;
			level->findPath(&awayPath, this, ax, Mth::floor(y), az, 16, false, false);
			setPath(awayPath);
		}
		holdGround = false;
	} else {
		holdGround = true;
	}

	if (attackTime == 0) {
		Mob* mobTarget = target->isMob() ? (Mob*)target : NULL;
		int aux = (mobTarget != NULL) ? choosePotion(mobTarget) : PotionItem::POISON;
		ThrownPotion* potion = new ThrownPotion(level, this, aux);
		float yd = (target->y + target->getHeadHeight() - 0.7f) - potion->y;
		float yo = Mth::sqrt(xd * xd + zd * zd) * 0.2f;
		level->playSound(this, "random.bow", 1.0f, 1.0f / (random.nextFloat() * 0.4f + 0.8f));
		level->addEntity(potion);
		potion->shoot(xd, yd + yo, zd, 1.0f, 12);
		attackTime = SharedConstants::TicksPerSecond * 3;
	}
}

int Witch::choosePotion(Mob* target)
{
	// Finish a weak target with instant harming, like modern MC.
	if (target->health <= 8) return PotionItem::HARMING;
	for (int tries = 0; tries < 4; ++tries) {
		int r = random.nextInt(4);
		int aux = PotionItem::POISON;
		if (r == 1) aux = PotionItem::SLOWNESS;
		else if (r == 2) aux = PotionItem::WEAKNESS;
		else if (r == 3) aux = PotionItem::HARMING;
		// Skip an effect the target already has (no point
		// re-poisoning a poisoned player).
		if (aux == PotionItem::POISON && target->poisonEffectTicks > 0) continue;
		if (aux == PotionItem::SLOWNESS && target->slownessEffectTicks > 0) continue;
		if (aux == PotionItem::WEAKNESS && target->weaknessEffectTicks > 0) continue;
		return aux;
	}
	return PotionItem::POISON;
}

void Witch::dropDeathLoot()
{
	int sticks = random.nextInt(3); // 0-2
	for (int i = 0; i < sticks; i++) {
		spawnAtLocation(Item::stick->id, 1);
	}
	if (random.nextInt(3) == 0) spawnAtLocation(Item::spiderEye->id, 1);
	if (random.nextInt(4) == 0) spawnAtLocation(Item::sugar->id, 1);
	if (random.nextInt(4) == 0) spawnAtLocation(Item::yellowDust->id, 1);
	if (random.nextInt(8) == 0) spawnAtLocation(Item::glassBottle->id, 1);
}

int Witch::getExperienceReward(Player* killedBy)
{
	(void) killedBy;
	return 5;
}
