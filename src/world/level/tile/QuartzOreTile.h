#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__QuartzOreTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__QuartzOreTile_H__

// v21: nether quartz ore (id 153, modern MC). Texture 172
// (terrain.png): netherrack with white quartz chunks. Drops one
// nether quartz (Item::netherQuartz, already registered since the
// original tree). Follows RedStoneOreTile's getResource override.

#include "OreTile.h"
#include "../../item/Item.h"

class QuartzOreTile: public OreTile
{
	typedef OreTile super;
public:
	QuartzOreTile(int id, int tex)
	:	super(id, tex)
	{
	}

	int getResource(int data, Random* random) {
		(void) data; (void) random;
		return Item::netherQuartz->id;
	}
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_TILE__QuartzOreTile_H__*/
