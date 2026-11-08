#include "Slime.h"

#include "../../item/Item.h"
#include "../../level/Level.h"
#include "../../../util/Mth.h"
#include "../ai/goal/GoalSelector.h"
#include "../ai/control/JumpControl.h"
#include "../ai/goal/RandomStrollGoal.h"
#include "../ai/goal/MeleeAttackGoal.h"
#include "../ai/goal/target/NearestAttackableTargetGoal.h"
#include "../ai/goal/target/HurtByTargetGoal.h"

Slime::Slime( Level* level )
:	super(level),
	jumpDelay(0),
	squish(0),
	prevSquish(0)
{
	entityRendererId = ER_SLIME_RENDERER;
	this->textureName = "mob/slime.png";

	this->setSize(1.0f, 1.0f);
	runSpeed = 0.5f;
	attackDamage = 2;

	targetSelector = new GoalSelector();
	targetSelector->addGoal(1, new HurtByTargetGoal(this, false));
	targetSelector->addGoal(2, new NearestAttackableTargetGoal(this, 1, 16, 0, true));

	goalSelector = new GoalSelector();
	goalSelector->addGoal(2, new MeleeAttackGoal(this, runSpeed, false, 0));
	goalSelector->addGoal(7, new RandomStrollGoal(this, runSpeed) );

	moveControl = new MoveControl(this);
	jumpControl = new JumpControl(this);
}

Slime::~Slime() {
	delete goalSelector;
	delete targetSelector;

	delete moveControl;
	delete jumpControl;
}

int Slime::getMaxHealth() {
	return 8;
}

void Slime::aiStep() {
	// Hop movement (vanilla slime behaviour): the goals steer xd/zd
	// via moveControl, the hop supplies the vertical motion.
	if (onGround && !level->isClientSide) {
		if (--jumpDelay <= 0) {
			jumpDelay = 10 + random.nextInt(10);
			yd = 0.42f;
		}
	}

	super::aiStep();
}

void Slime::tick() {
	prevSquish = squish;
	// Squashed on the ground, stretched in the air; eased toward the
	// target each tick so the renderer can interpolate smoothly.
	float target = onGround ? 0.25f : -0.2f;
	squish += (target - squish) * 0.2f;

	super::tick();
}

float Slime::getSquish(float a) {
	return prevSquish + (squish - prevSquish) * a;
}

int Slime::getEntityTypeId() const {
	return MobTypes::Slime;
}

const char* Slime::getAmbientSound() {
	// v19: no slime sound file exists in the tree ("mob.slime" was
	// never registered, so slimes were silent). Placeholder: the
	// short "pop" squelch is the closest existing sound until a real
	// slime squish file is added.
	return "random.pop";
}

std::string Slime::getHurtSound() {
	return "random.pop";
}

std::string Slime::getDeathSound() {
	return "random.pop";
}

int Slime::getDeathLoot() {
	// Item::slimeBall was already registered in this tree (id 85).
	// Mob::dropDeathLoot drops 0-2 of it, like modern MC.
	return Item::slimeBall->id;
}
