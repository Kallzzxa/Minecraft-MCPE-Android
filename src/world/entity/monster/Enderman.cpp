#include "Enderman.h"

#include "../../item/Item.h"
#include "../../level/Level.h"
#include "../../../util/Mth.h"
#include "../ai/goal/GoalSelector.h"
#include "../ai/control/JumpControl.h"
#include "../ai/goal/RandomStrollGoal.h"
#include "../ai/goal/MeleeAttackGoal.h"
#include "../ai/goal/target/NearestAttackableTargetGoal.h"
#include "../ai/goal/target/HurtByTargetGoal.h"

Enderman::Enderman( Level* level )
:	super(level)
{
	entityRendererId = ER_ENDERMAN_RENDERER;
	this->textureName = "mob/enderman.png";

	this->setSize(0.6f, 2.9f);
	runSpeed = 0.6f;
	attackDamage = 6;

	targetSelector = new GoalSelector();
	targetSelector->addGoal(1, new HurtByTargetGoal(this, false));
	targetSelector->addGoal(2, new NearestAttackableTargetGoal(this, 1, 16, 0, true));

	goalSelector = new GoalSelector();
	goalSelector->addGoal(2, new MeleeAttackGoal(this, runSpeed, false, 0));
	goalSelector->addGoal(7, new RandomStrollGoal(this, runSpeed) );

	moveControl = new MoveControl(this);
	jumpControl = new JumpControl(this);
}

Enderman::~Enderman() {
	delete goalSelector;
	delete targetSelector;

	delete moveControl;
	delete jumpControl;
}

int Enderman::getMaxHealth() {
	return 20;
}

bool Enderman::hurt(Entity* source, int dmg) {
	bool ok = super::hurt(source, dmg);
	// Vanilla-style escape: when hurt it often teleports away.
	if (ok && !level->isClientSide && random.nextFloat() < 0.5f) {
		teleportRandomly();
	}
	return ok;
}

bool Enderman::teleportRandomly() {
	for (int i = 0; i < 8; ++i) {
		float nx = x + (float)(random.nextInt(33) - 16);
		float nz = z + (float)(random.nextInt(33) - 16);
		int ix = Mth::floor(nx);
		int iz = Mth::floor(nz);
		int topY = level->getTopSolidBlock(ix, iz);
		// Need two blocks of air above solid ground to stand in.
		if (level->isSolidBlockingTile(ix, topY + 1, iz)) continue;
		if (level->isSolidBlockingTile(ix, topY + 2, iz)) continue;

		float ox = x, oy = y, oz = z;
		moveTo(nx + 0.5f, (float)(topY + 1), nz + 0.5f, yRot, xRot);

		// Puff of smoke at both ends of the jump, like vanilla.
		for (int p = 0; p < 8; ++p) {
			float rx = (random.nextFloat() - 0.5f) * 1.0f;
			float ry = random.nextFloat() * 2.0f;
			float rz = (random.nextFloat() - 0.5f) * 1.0f;
			level->addParticle(PARTICLETYPE(smoke), ox + rx, oy + ry, oz + rz, 0, 0, 0);
			level->addParticle(PARTICLETYPE(smoke), x + rx, y + ry, z + rz, 0, 0, 0);
		}
		return true;
	}
	return false;
}

int Enderman::getEntityTypeId() const {
	return MobTypes::Enderman;
}

const char* Enderman::getAmbientSound() {
	return "mob.enderman";
}

std::string Enderman::getHurtSound() {
	return "mob.enderman.hit";
}

std::string Enderman::getDeathSound() {
	return "mob.enderman.death";
}

int Enderman::getDeathLoot() {
	// Mob::dropDeathLoot drops 0-2 of it, like modern MC.
	return Item::ender_pearl->id;
}
