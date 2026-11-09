#ifndef NET_MINECRAFT_CLIENT_PARTICLE__SplashParticle_H__
#define NET_MINECRAFT_CLIENT_PARTICLE__SplashParticle_H__

//package net.minecraft.client.particle;

#include "Particle.h"
#include "../../world/level/Level.h"

// v13: splash droplets when an entity enters water. This class used to
// extend WaterDropParticle, but that header never existed in this tree
// (so the class could never compile and the "splash" particle was
// commented out everywhere). It now extends Particle directly, modelled
// on BubbleParticle: a small droplet with gravity, a short life, and a
// little upward kick when it is thrown sideways (entering water fast).
// v47: tex 33 -> 17. Tex 33 is NOT a droplet — it is the fishing-bobber
// sprite (white/red bobber + black hook), which is why v43 removed the
// water-entry spawn that used it. The real droplet cells are row 1 of
// particles.png (tex 16-22, all blue): tex 16/19/20/21/22 are only 1-3
// opaque pixels in the bottom two rows of the 8x8 cell (near-invisible
// at burst scale), while tex 17 is 13 blue pixels scattered across the
// whole cell — the only row-1 cell that reads as a droplet burst. Tex 32
// (bubble ring) was rejected: it would conflate the burst with the
// vanilla bubble loop, and BubbleParticle itself deletes itself outside
// water, so it cannot be reused for a burst on land. Audit: the only
// remaining spawner of this particle is ThrownPotion::onHit (the v43
// water-entry spawn is gone), so this tex change affects ONLY the
// thrown splash-potion impact burst. Colour stays white (1,1,1) — the
// sprite pixels are already blue, so no tint is needed.
class SplashParticle: public Particle
{
	typedef Particle super;
public:
    SplashParticle(Level* level, float x, float y, float z, float xa, float ya, float za)
	: super(level, x, y, z, xa, ya, za)
	{
        gravity = 0.04f;
        tex = 17;
        rCol = 1.0f;
        gCol = 1.0f;
        bCol = 1.0f;
        size *= 0.6f;
        lifetime = (int) (8 / (Mth::random() * 0.8 + 0.2));
        if (ya == 0 && (xa != 0 || za != 0)) {
            yd = ya + 0.1f;
        }
    }
};

#endif /*NET_MINECRAFT_CLIENT_PARTICLE__SplashParticle_H__*/
