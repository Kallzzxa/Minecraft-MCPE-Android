#ifndef NET_MINECRAFT_CLIENT_PARTICLE__SplashParticle_H__
#define NET_MINECRAFT_CLIENT_PARTICLE__SplashParticle_H__

//package net.minecraft.client.particle;

#include "Particle.h"
#include "../../world/level/Level.h"

// v13: splash droplets when an entity enters water. This class used to
// extend WaterDropParticle, but that header never existed in this tree
// (so the class could never compile and the "splash" particle was
// commented out everywhere). It now extends Particle directly, modelled
// on BubbleParticle: a small white droplet (tex 33, next to the bubble
// sprite at 32 in particles.png) with gravity, a short life, and a
// little upward kick when it is thrown sideways (entering water fast).
class SplashParticle: public Particle
{
	typedef Particle super;
public:
    SplashParticle(Level* level, float x, float y, float z, float xa, float ya, float za)
	: super(level, x, y, z, xa, ya, za)
	{
        gravity = 0.04f;
        tex = 33;
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
