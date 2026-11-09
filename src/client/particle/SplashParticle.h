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
// v48: tex 17 -> 18 + per-potion tint. The v47 limit was that every
// potion burst the same blue — and tinting tex 17 directly CANNOT fix
// that: Particle::render multiplies the texel by the vertex colour
// (t.color(rCol*br, ...), Tesselator clamps each channel at 1.0), and
// tex 17's pixels are blue with max channels (71,106,204)/255 =
// (0.28, 0.42, 0.80) — so a healing-pink tint could at most render a
// dark purple (0.25, 0.10, 0.28) and harming-red a near-black maroon.
// Tex 18 was empty and sits in the same row-1 droplet row, adjacent
// to 17: it now holds a WHITE copy of tex 17's exact 13-px shape
// (drawn into particles.png, same PIL precedent as v9/v11/v20/v25),
// so white texel x tint = the tint itself, and every potion colour
// renders faithfully. The tint table below is NOT invented: each RGB
// is the liquid colour of that potion's bottle icon, sampled from
// gui/items.png row 12 (drinkable icons, 45 px of liquid per cell —
// the v20/v25 bottle colours), so the burst matches the bottle the
// player threw. Water keeps its light-blue liquid colour, so the
// v47 look is preserved for water bottles. The aux arrives as the
// particle `data` int (ThrownPotion::onHit -> Level::addParticle ->
// LevelRenderer dispatch); baseAux (& 15) strips the splash /
// extended / level II flag bits, so variants tint like their base.
class SplashParticle: public Particle
{
	typedef Particle super;
public:
    SplashParticle(Level* level, float x, float y, float z, float xa, float ya, float za, int potionAux)
	: super(level, x, y, z, xa, ya, za)
	{
        gravity = 0.04f;
        tex = 18;
        // v48: per-potion burst tint (see header comment). Values are
        // PotionItem base aux 0..10 -> gui/items.png row-12 liquid RGB.
        int base = potionAux & 15;
        int cr = 90, cg = 160, cb = 220; // WATER (default): light blue
        switch (base) {
            case 1: cr = 150; cg = 60;  cb = 50;  break; // AWKWARD: brown-red
            case 2: cr = 230; cg = 60;  cb = 90;  break; // HEALING: pink-red
            case 3: cr = 80;  cg = 200; cb = 230; break; // SWIFTNESS: cyan
            case 4: cr = 230; cg = 110; cb = 30;  break; // STRENGTH: orange
            case 5: cr = 250; cg = 180; cb = 40;  break; // FIRE_RESISTANCE: gold
            case 6: cr = 230; cg = 110; cb = 180; break; // REGENERATION: pink
            case 7: cr = 110; cg = 140; cb = 50;  break; // POISON: olive green
            case 8: cr = 90;  cg = 100; cb = 120; break; // WEAKNESS: grey-blue
            case 9: cr = 140; cg = 40;  cb = 60;  break; // HARMING: dark red
            case 10: cr = 70; cg = 110; cb = 170; break; // SLOWNESS: blue
            default: break; // WATER (0) or out of range: light blue
        }
        rCol = cr / 255.0f;
        gCol = cg / 255.0f;
        bCol = cb / 255.0f;
        size *= 0.6f;
        lifetime = (int) (8 / (Mth::random() * 0.8 + 0.2));
        if (ya == 0 && (xa != 0 || za != 0)) {
            yd = ya + 0.1f;
        }
    }
};

#endif /*NET_MINECRAFT_CLIENT_PARTICLE__SplashParticle_H__*/
