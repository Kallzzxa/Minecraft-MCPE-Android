#include "ExperienceOrb.h"
#include "../EntityTypes.h"
#include "../../entity/player/Player.h"
#include "../../level/Level.h"
#include "../../level/material/Material.h"
#include "../../level/tile/Tile.h"
#include "../../../util/Mth.h"
#include "../../../nbt/CompoundTag.h"

// v8: experience orb. Same lifetime as a dropped item (5 minutes).
const int ExperienceOrb::LIFETIME = 5 * 60 * SharedConstants::TicksPerSecond;

// Magnet range: orbs fly to the player from this far away (modern MC: ~6).
static const float ORB_MAGNET_RANGE = 6.0f;

ExperienceOrb::ExperienceOrb( Level* level, float x, float y, float z, int value )
:	super(level),
	value(value),
	age(0),
	lifeTime(LIFETIME)
{
	entityRendererId = ER_EXPERIENCE_ORB_RENDERER;

	setSize(0.25f, 0.25f);

	heightOffset = bbHeight / 2.0f;
	setPos(x, y, z);

	yRot = (float) (Mth::random() * 360);

	xd = (float) (Mth::random() * 0.2f - 0.1f);
	yd = +0.2f;
	zd = (float) (Mth::random() * 0.2f - 0.1f);
	makeStepSound = false;
}

ExperienceOrb::ExperienceOrb( Level* level )
:	super(level),
	value(1),
	age(0),
	lifeTime(LIFETIME)
{
	entityRendererId = ER_EXPERIENCE_ORB_RENDERER;

	setSize(0.25f, 0.25f);
	heightOffset = bbHeight / 2.0f;
}

ExperienceOrb::~ExperienceOrb()
{
}

void ExperienceOrb::tick()
{
	super::tick();
	xo = x;
	yo = y;
	zo = z;

	// Gravity, like ItemEntity.
	yd -= 0.04f;

	// Magnet: accelerate towards the nearest player in range.
	// (Player::aiStep touches entities within 1 block of its bounding
	// box, which is what actually collects the orb.)
	Player* nearest = NULL;
	float nearestDistSqr = ORB_MAGNET_RANGE * ORB_MAGNET_RANGE;
	for (unsigned int i = 0; i < level->players.size(); i++) {
		Player* p = level->players[i];
		if (!p || !p->isAlive()) continue;
		float dx = p->x - x;
		float dy = (p->y + p->getHeadHeight()) - y;
		float dz = p->z - z;
		float d2 = dx * dx + dy * dy + dz * dz;
		if (d2 < nearestDistSqr) {
			nearestDistSqr = d2;
			nearest = p;
		}
	}
	if (nearest != NULL) {
		float dist = Mth::sqrt(nearestDistSqr);
		if (dist > 0.001f) {
			// Pull harder the closer the orb gets, like modern MC.
			float pull = 0.06f + (1.0f - dist / ORB_MAGNET_RANGE) * 0.12f;
			xd += (nearest->x - x) / dist * pull;
			yd += ((nearest->y + nearest->getHeadHeight()) - y) / dist * pull;
			zd += (nearest->z - z) / dist * pull;
			// Cap speed so orbs don't overshoot the player forever.
			float speed2 = xd * xd + yd * yd + zd * zd;
			if (speed2 > 0.49f) {
				float s = 0.7f / Mth::sqrt(speed2);
				xd *= s;
				yd *= s;
				zd *= s;
			}
		}
	}

	move(xd, yd, zd);

	float friction = 0.98f;
	if (onGround) {
		friction = 0.6f * 0.98f;
		int t = level->getTile(Mth::floor(x), Mth::floor(bb.y0) - 1, Mth::floor(z));
		if (t > 0) {
			friction = Tile::tiles[t]->friction * 0.98f;
		}
	}
	// Don't damp the magnet pull as hard while flying to the player.
	if (nearest == NULL) {
		xd *= friction;
		yd *= 0.98f;
		zd *= friction;
	} else {
		xd *= 0.98f;
		yd *= 0.98f;
		zd *= 0.98f;
	}

	if (onGround) {
		yd *= -0.5f;
	}

	age++;
	if (age >= lifeTime) {
		remove();
	}
}

void ExperienceOrb::playerTouch( Player* player )
{
	if (level->isClientSide) return;

	if (player->isAlive()) {
		player->addExperience(value);
		level->playSound(this, "random.pop", 0.3f, 1.5f + sharedRandom.nextFloat() * 0.5f);
		remove();
	}
}

int ExperienceOrb::getEntityTypeId() const
{
	return EntityTypes::IdExperienceOrb;
}

void ExperienceOrb::addAdditonalSaveData(CompoundTag* entityTag) {
	entityTag->putShort("Value", (short) value);
	entityTag->putShort("Age", (short) age);
}

void ExperienceOrb::readAdditionalSaveData(CompoundTag* tag) {
	if (tag->contains("Value")) value = tag->getShort("Value");
	if (tag->contains("Age")) age = tag->getShort("Age");
}
