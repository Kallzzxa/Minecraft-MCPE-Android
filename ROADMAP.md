# MCPE Android — Roadmap to Modern Minecraft

Goal: bring this MCPE 0.6.1 build (`Kallzzxa/Minecraft-MCPE-Android`) as close
as possible to modern Minecraft PE (Bedrock), one testable version at a time.
Each version is ONE coherent batch of features, shipped as a new APK
(`minecraft-mcpe-android-0.6.1-debug-v<N>.apk`, versionCode N).

## Rules

- One batch per version. Small enough to implement + CI in ~10–20 minutes.
- Never break what already works: right-side jump button, hunger + meat
  drops + furnace, sprint, 512×512 world, v4 lighting, 0.8× fog,
  v7 hostile mob drops, v8 XP orbs + levels + HUD, v9 carrots/potatoes.
- If a feature proves unfeasible after reasonable investigation, mark it
  skipped here with the reason and move on. Don't grind.
- World 1024×1024 ONLY with measurements (v3/v6 rejected it on paper;
  v15 re-measured it from the code — see "v15" below: 4096 chunks
  ≈ 322 MB chunk RAM + 399,384 renderer VBO buffers, still not viable).
  A truly infinite world like modern Minecraft needs the fixed chunk
  cache replaced with a dynamic chunk map + streaming — a multi-version
  engine job, NOT one batch. v15 (this version) proved it IS feasible:
  terrain gen, biomes, the renderer grid and fog are already
  coordinate-free; the blockers are a fixed 32×32 save-offset table and
  ~10 clamp sites, all bounded (see "v15" for the full map). v16 built
  the streaming core, v17 removed the border + dynamic save/load, v18
  verifies. No version promises "infinite" until v18 proves it on
  device (v17 lifts the border in code; the claim still waits on the
  v18 device verification).
- Everything is BUILD-ONLY until tested on the user's realme 8.

## Done

- **v1** — First Android build (NDK r17c + aapt2/d8, then Gradle).
- **v2** — Right-side jump button (AREA_JUMP_RIGHT), left-handed mirror.
- **v3** — World 256→512×512; authentic light ramp, AO default ON,
  render distance far (256), fog 0.5×.
- **v4** — Light ramp floor 0.05→0.12 (v3 was too dark in shadows).
- **v5** — Hunger system: 20-point bar below hearts, -1 HP/4s at 0,
  regen at hunger ≥ 18; sheep wool + raw mutton, cow leather + raw beef,
  chicken feather + raw chicken; raw mutton→cooked mutton in furnace;
  mutton items (id 168/169) with custom icons.
- **v6** — Fog START 0.5×→0.8× render distance (~205 blocks clear),
  render-chunk grid cap 400→512; sprint (double-tap D-pad up, 1.4×);
  activity-based hunger (exhaustion: walk 0.01/block, sprint 0.1/block,
  jump 0.05/jump; 4.0 = 1 hunger point); regen verified live
  (Player::tick → foodData.tick in single-player).
- **v7 — Hostile mob drops (FALLBACK for boats, see "Skipped / blocked").**
  Zombie drops rotten flesh (new FoodItem id 170, +4 hunger, icon drawn
  into `gui/items.png` cell (6, 7), lang `item.rottenFlesh.name` already
  existed); the zombie's legacy feather drop was removed. Creeper
  gunpowder (`Item::sulphur`, id 33) and spider string (`Item::string`,
  id 31) were ALREADY wired via `Mob::dropDeathLoot` → `getDeathLoot`
  (0–2 each) — verified by code inspection, no change needed. Caveats:
  no hunger status-effect system exists yet, so rotten flesh has no
  80%-hunger downside (modern MC has one); a creeper that explodes
  drops nothing (it `remove()`s instead of dying — matches modern MC).

- **v8 — XP orbs + levels + HUD.** New `ExperienceOrb` entity
  (`src/world/entity/item/ExperienceOrb.*`, id 84, follows the
  `ItemEntity` pattern: falls with gravity, magnet-flies to the player
  within 6 blocks, collected on touch, 5-min lifetime) rendered as a
  pulsing billboarded green quad (`ExperienceOrbRenderer`, no texture —
  `gui/icons.png` has no XP-orb sprite). Mobs killed by the player drop
  orbs: passive animals 1–3 XP (`Animal::getExperienceReward`), hostile
  mobs 3–5 XP (`Monster::getExperienceReward`), water animals 1–3 XP
  (pre-existing `WaterAnimal::getExperienceReward`, now a real override
  of `Mob::getExperienceReward`); orbs split into 1–3 XP pieces.
  `Player` tracks `experienceLevel` / `experienceProgress` /
  `totalExperience` with the classic MC curve (L<16: 2L+7, L<31: 5L−38,
  else 9L−158), saved in player NBT (`XpLevel`/`XpProgress`/`XpTotal`).
  HUD: green XP bar (182×5, plain fills) above the hotbar + level
  number centered above it. XP is deliberately KEPT on death for now
  (death drops nothing in this build). Enchanting NOT included — v10.
  Night-spawn check (v8 scope): `Level::tick` → `MobSpawner::tick`
  every 2 ticks when `spawnMobs` (Survival), `_spawnEnemies=true`,
  default difficulty NORMAL — chain intact, nothing broken, no code
  change; a live night test on device is still the real proof.

- **v9 — Carrots & potatoes.** New `RootCropTile` (subclass of `CropTile`:
  same farmland growth, 8 data values, but only 4 visual stages — data maps
  2:1 onto new terrain.png sprites drawn into row-11 cells 184–191, which
  were unused magenta placeholders; harvest drops the plant itself, 1–3
  when mature, 1 when immature, and NO wheat seeds — `CropTile`'s
  seed-spawning `spawnResources` is deliberately NOT inherited).
  Tile ids 141 (carrots) / 142 (potatoes), matching modern MC. New
  `PlantableFoodItem` (FoodItem + SeedItem planting on farmland): carrot
  (item id 171, +3 hunger, icon cell (6, 9)), potato (id 172, +1, (6, 10));
  baked potato (id 173, FoodItem +5, icon (8, 9)) via a new furnace recipe
  (raw potato → baked potato). Icons drawn into blank `gui/items.png`
  cells; lang `tile.carrots/potatoes.name`, `item.carrot/potato/potatoBaked.name`.
  Zombies keep their 0–2 rotten flesh and gain a rare bonus: 2.5% chance of
  1 carrot or 1 potato (50/50) via a `Zombie::dropDeathLoot` override.
  `FarmTile::isUnderCrops` now also counts carrots/potatoes so farmland
  under them does not dry out and revert to dirt. Eating uses the existing
  FoodItem path (no change). Caveats: crop sprites are simple drawn
  sprouts (4 stages, not 8 distinct like wheat); no golden carrot /
  poisonous potato (no status-effect system yet); zombie drop rate is
  code-only until a live kill test on device.

- **v10 — Enchanting.** New `EnchantTableTile` (tile 116, modern MC id;
  top face reuses the bookshelf texture, sides are obsidian; lang
  `tile.enchantmentTable.name` already existed) + crafting recipe
  (book / diamond — `Item::emerald` is the diamond gem in this tree's
  old naming — / obsidian; the recipe was commented out in
  `Recipes.cpp` and is now live). Using the table opens `EnchantScreen`,
  which enchants the item held in the hand — no container/tile-entity
  plumbing. It shows up to 3 random offers for enchantments the held
  item can take, each costing 1–3 whole XP levels (v8 levels, spent via
  new `Player::spendExperienceLevels`; the partial-progress bar resets
  when levels are spent). One enchantment per item, stored on the
  `ItemInstance` (`Ench`/`EnchLvl` in the item tag, so it survives
  save/load, copy and split; enchanted items never stack with
  differently-enchanted ones) and shown in the item name as
  e.g. `Diamond Sword [Sharpness II]`. Four working enchantments:
  Sharpness (+1 damage/level, `ItemInstance::getAttackDamage`),
  Efficiency (+50% dig speed/level, `DiggerItem::getDestroySpeed` —
  this also fixed `Inventory::getDestroySpeed` passing NULL instead of
  the selected item, which would have hidden every per-item modifier),
  Protection (−1 damage per total level across worn armor, capped at
  half the hit, `Player::getDamageAfterArmorAbsorb`), Unbreaking
  (level/(level+1) chance to take no durability damage,
  `ItemInstance::hurt`). Applicability: Sharpness swords only,
  Efficiency pickaxe/shovel/axe only (hoes are plain `Item`s here, not
  `DiggerItem`, so no Efficiency on hoes), Protection armor only,
  Unbreaking any damageable item. Caveats: no lapis cost, no anvil, no
  enchanted books, no re-enchanting an already-enchanted item (the
  table refuses it); no bookshelf-power mechanic (offer levels are
  purely random 1–3, bookshelves nearby change nothing); the table
  texture is a reuse (bookshelf top + obsidian sides), not a new sprite;
  all of it is build-only until a live test on device.

- **v11 — Redstone basics.** Redstone dust item (id 331, the
  registration commented out since the original tree) + wire tile
  (id 55, flat slab, on/off signal — no 0–15 levels, no up/down
  steps), redstone torch (id 76, always-ON source, light 7, red
  sparkles), lever (id 69, tap to toggle, bit 8 = on, wall/floor
  mount like a torch), redstone lamp (ids 123/124 off/on pair, swaps
  tiles on power change). Power uses the tree's existing signal
  framework (`isSignalSource` / `getSignal` / `hasNeighborSignal` /
  `neighborChanged` + `setData` propagation); TNT already listened
  to it, so TNT now explodes from lever/torch/wire power with no
  TNT change, and wooden doors/trapdoors also react to the signal.
  Mining redstone ore now actually drops 4–5 dust (its `getResource`
  returned 0 before — no drop at all), and smelting ore in a furnace
  gives dust too (that recipe already existed but its item was NULL).
  Crafting: redstone torch (redstone + stick), lever (stick + stone),
  lamp (glowstone + 4 redstone). All four in the creative inventory;
  new terrain.png textures drawn into unused magenta cells
  (199 wire off, 200 wire on, 201 lamp off, 202 lamp on, 203 torch,
  215 lever). Caveats: signal is on/off only; wire does NOT transmit
  power through the solid block below it (deliberate — otherwise a
  powered wire would see its own power reflected back and latch on
  forever), so a lamp/TNT on top of a block with wire on the block's
  side does not trigger — put the wire directly next to the target;
  torch never turns off when powered (no burnout/toggle-off logic);
  no pistons, repeaters, comparators, buttons, pressure plates, or
  rails (out of scope); lamp/torch/wire textures are simple drawn
  sprites, not the modern ones; all of it is build-only until a live
  test on device.

- **v12 — Slime + Enderman (FALLBACK for minecarts, see
  "Skipped / blocked").** New `Slime` monster (`MobTypes::Slime` 37):
  1×1×1 cube, 8 HP, hop movement (jump every 10–20 ticks while on the
  ground, goals steer it like a zombie), squash-and-stretch animation
  (`getSquish`, eased toward 0.25 on ground / −0.2 in air, applied by
  `SlimeRenderer::scale`), drops 0–2 slimeballs via
  `Mob::dropDeathLoot` (`Item::slimeBall` id 85 was already registered
  in this tree). New `Enderman` monster (`MobTypes::Enderman` 38):
  0.6×2.9, 20 HP, 6 damage, tall thin humanoid model (30px arms/legs)
  with a black texture + purple eyes, teleports away 50% of the time
  when hurt (up to 8 tries for a safe spot within 16 blocks, with
  smoke puffs at both ends), drops 0–2 ender pearls (new `Item`
  id 368, icon drawn into blank `gui/items.png` cell (6, 8), lang
  `item.enderPearl.name` already existed). Both use the zombie goal
  setup (melee + stroll, nearest-attackable-target + hurt-by-target),
  spawn at night through `Biome` (slime weight 10, clusters 1–3;
  enderman weight 1, always alone — the two spawn lines were
  commented out since the original tree), and give 3–5 XP via
  `Monster::getExperienceReward`. New textures `mob/slime.png` /
  `mob/enderman.png` (64×32, drawn, not modern sprites). Caveats:
  single-size slime only (no splitting into smaller slimes on death);
  no enderman stare/teleport-to-player behaviour (no look-at-player
  goal exists in this tree — it attacks on sight like a zombie);
  no ender-pearl throwing (no throwable use for the item yet);
  all of it is build-only until a live night test on device.

- **v13 — Graphics polish (RE-PRIORITISED ahead of the witch — see
  "Next").** Investigation first: clouds ALREADY existed
  (`LevelRenderer::renderClouds`, +X drift 0.03/tick, y 128, only when
  `fancyGraphics` — default ON); water ALREADY animated (dynamic
  `WaterTexture`/`WaterSideTexture` wave simulation, ticked every game
  tick via `Textures::tick` from `Minecraft::tick`); the sky was a flat
  colour ONLY — no sun, no moon, no sunrise tint (`renderSky` draws
  just the sky-colour plane; `getSunAngle` existed but no renderer
  used it; no sun.png/moon.png assets, only `environment/clouds.png`);
  block-break particles ALREADY existed and were wired
  (`GameMode::destroyBlock` → `ParticleEngine::destroy`, digging
  crack puffs from `Minecraft.cpp`); bubbles underwater ALREADY
  existed (`Entity::baseTick`, `Mob::tick`). What v13 shipped:
  sun + moon — solid-colour camera-facing billboards
  (`LevelRenderer::renderSunMoon`, called from `renderSky`), sun
  travels east→west from `Level::getSunAngle`, moon exactly opposite,
  both fade in/out near the horizon, dimmer low in the sky; sunrise /
  sunset orange tint blended into `Level::getSkyColor` while the sun
  is near the horizon; clouds drift diagonally now (+X 0.05 and +Z
  0.03 blocks/tick, was +X 0.03 only) so the movement is actually
  visible; splash particles on entering water — `SplashParticle`
  used to extend a `WaterDropParticle` header that never existed in
  this tree (uncompilable dead code, "splash" commented out of
  `LevelRenderer::addParticle` and `Entity::baseTick`), so it now
  extends `Particle` directly (tex 33, next to the bubble sprite)
  and both call sites are live (`ParticleType::splash` added to the
  enum); dust puffs on a hard landing (`Entity::checkFallDamage`,
  fall > 3 blocks, up to 8 grey smoke puffs at the feet) and while
  sprinting (`LocalPlayer::tick`, ~35% of ticks while moving on the
  ground). Caveats: no shader, no HD textures, no dynamic shadows
  (out of scope); the sun/moon are flat solid-colour squares, not
  textured discs; landing/sprint dust reuses the grey smoke sprite;
  no rain/weather system exists in this tree (rainLevel/thunderLevel
  are hardcoded 0 in `Level::getCloudColor`), so no rain splashes;
  all of it is build-only until a live test on device.

- **v14 — Modern HUD + sprint fix (USER PRIORITY 2026-11-08 23:59, from
  2 screenshots — jumped the queue ahead of music).**
  Sprint bug, diagnosed from code (not guessed): v6 detected a
  double-tap from `_buttons[N]` LEVEL transitions sampled once per
  game tick, but that level comes from the pointer's CURRENT position
  via `TouchAreaModel::getPointerId`, and the two diagonal corner
  cells of the 3×3 D-pad (`aUpLeft`/`aUpRight`) are created but never
  `addArea`'d — so a held thumb rolling a few px into a corner
  dropped the level for a tick (phantom release), and sliding back
  onto N re-pressed inside the 7-tick window (phantom double-tap),
  sprinting on a single press-and-hold. The analog bounding-rect
  steering made it worse: once moving forward, the whole 3×3 box
  steers, so the thumb naturally wanders into the corners while
  "holding forward". Fix (`TouchscreenInput::tick`): count ONLY
  discrete finger-down edges (`Multitouch::isPressed`, the true DOWN
  edge from the Android event stream, latched until
  `Multitouch::reset()` at the end of the game tick); remember the
  pressing pointer id, so the hold survives the thumb sliding inside
  the D-pad while it keeps steering forward (upper half of the box);
  a genuine lift records the release tick, the 2nd edge within 7
  ticks (~350 ms) of it sprints. Still 1.4× (`LocalPlayer`), still no
  sprint in water (new explicit guard — v6 actually sprinted while
  swimming) or while flying. Caveat: tap edges are sampled at 20 tps,
  so a double-tap faster than one 50 ms tick collapses into one edge
  (humanly impossible — a thumb tap is longer than that).
  HUD (`Gui.cpp`, layout only — no hunger/XP/gameplay change):
  hearts + hunger moved from the top-left corner to one row above the
  hotbar (hearts left-aligned, hunger right-aligned with the hotbar);
  XP bar is now exactly as wide as the hotbar (was fixed 182 px,
  overhanging the 160 px touch hotbar) directly above it, green on
  black with a 1 px inset; level number green + centered above the
  bar — on the hearts/hunger row when the hotbar is wide enough to
  leave a gap (≥174 px), otherwise one row above the hearts so it
  never overlaps an icon (touch hotbar is 8 slots = 160 px, hearts +
  hunger already fill that row); armor moved to the row above the
  bubbles, bubbles to the row above the hearts. Hotbar polish:
  dark translucent backing under the classic bar texture, thin frame
  around every slot, selected slot = explicit white frame + faint
  white fill (replaces the gui.png selection box), item counts now
  white (was light grey). All of it is build-only until a live test
  on device (press-hold = walk, double-tap-hold = sprint).

- **v15 — Infinite world, stage 1: investigation + design (NO gameplay
  change — the APK plays exactly like v14).** The user asked for a world
  with no borders, 100% like modern MC (screenshot: terrain ends in a
  vertical edge, blue void outside). Read the whole chunk pipeline to
  find out whether infinite is feasible and what it costs.
  ### What the edge in the screenshot actually is
  Outside the 512×512 world, `ChunkCache::getChunk` returns the shared
  `emptyChunk` (air) — `ChunkCache.h:73` — so there is simply no terrain
  there: the last real chunk ends in a vertical wall and the sky shows
  through as blue void. Removing the border means making `getChunk`
  generate (or load from disk) a real chunk at ANY coordinate instead
  of returning air.
  ### Map of every fixed-world assumption (file:line — what it assumes)
  1. `src/world/level/LevelConstants.h:6,14-15` — the root:
     `CHUNK_CACHE_WIDTH = 32` chunks, `LEVEL_WIDTH = LEVEL_DEPTH = 512`
     blocks. Comment says it MUST stay a power of 2 (`ChunkCache`
     indexes slots with `& (WIDTH-1)`).
  2. `src/world/level/chunk/ChunkCache.h:27,34,45,55-57,74-76,202-203,
     214,257` — the fixed cache itself: `LevelChunk*
     chunks[32*32]` (1024 slots); `fits()` rejects any chunk outside
     [0,32); `getChunk()` returns `emptyChunk` outside; slot index is
     `(x & 31) + (z & 31) * 32` (would silently WRAP to a wrong chunk
     if the `fits()` guard were just deleted); `saveAll()` loops
     x/z 0..31; `load()` rejects outside coords. Slot REUSE already
     exists (`getChunk()`: evicted slot → `unload()` + `save()` +
     `saveEntities()`), but it never frees RAM — see (3).
  3. `src/world/level/levelgen/RandomLevelSource.h` (`ChunkMap` =
     `std::map<int, LevelChunk*>`) + `RandomLevelSource.cpp:getChunk()`
     — the hidden second copy of the world: EVERY generated chunk is
     kept in `chunkMap` forever (the destructor cleanup is commented
     out, and `ChunkCache` slot eviction does not delete the chunk —
     the same object lives on in `chunkMap`). Today that is bounded
     by 1024 because the cache only ever asks for 0..31; with
     streaming it would grow WITHOUT BOUND unless eviction deletes
     from `chunkMap` too. Key is `ChunkPos::hashCode`
     (`src/world/level/ChunkPos.h`): 15 bits per axis + sign bit —
     chunks beyond ±32767 (±524,288 blocks) COLLIDE onto other
     chunks' keys. Must become a 64-bit key
     (`((int64)x << 32) | (uint32)z`) when the map becomes the
     streaming store (also used by `ChunkPos::operator<` for
     `Level::_chunksToPoll`, so the change ripples there too).
     GOOD NEWS — generation itself is already infinite-capable:
     per-chunk seed `(x*341872712 + z*132899541)`, Perlin heights,
     `hasChunk()` always returns true, `postProcess()` works off
     `xt*16` offsets — no clamp anywhere. `BiomeSource`
     (`src/world/level/biome/BiomeSource.cpp:getBiomeBlock`,
     `getTemperatureBlock`) is likewise coordinate-free Perlin noise;
     its 16×16 arrays are per-chunk scratch, not world-sized.
  4. `src/world/level/Level.cpp:2239-2243` — `inRange()` (x/z < 512);
     only caller is `ClientSideNetworkHandler.cpp:798` (multiplayer
     block updates). `setInitialSpawn()` (431-443): spawn = world
     centre (256,256), wander-clamped to [4,508];
     `validateSpawn()` (452-463): same [4,508] clamp on load.
     `tick()` (281-282): chunk-poll list only accepts chunks 0..31
     (already player-relative ±4 chunks, so just drop the filter).
     `clip()` (947): hit test gated to `LEVEL_WIDTH` — and uses
     `LEVEL_WIDTH` for the Z check too (harmless while the world is
     square, wrong the moment it is not).
  5. `src/world/entity/Entity.cpp:887-888` — `loadFromTag()` clamps
     every loaded entity's position into `[padding, 512-padding]`.
     This is the invisible wall on LOAD (an entity saved past the
     border teleports back inside). No clamp exists in
     `Entity::move`/`baseTick` — movement itself is already unbounded.
  6. `src/world/level/storage/ExternalFileLevelStorage.cpp:279-280`
     — v1 player-load clamps pos to [0.5, 511.5]; `:297-299` —
     `tick()` scans x/z 0..31 for unsaved chunks (must iterate the
     resident set instead); `save()`/`load()` delegate to `RegionFile`.
  7. `src/world/level/storage/RegionFile.h/.cpp` — THE hard blocker
     for saving an infinite world. `chunks.dat` holds ONE fixed
     offset table: `offsets[1024]` (`SECTOR_INTS = 1024`,
     `SECTOR_COLS = 32`); `readChunk`/`writeChunk` index
     `offsets[x + z*32]` with NO bounds check — a negative chunk
     coordinate indexes BEFORE the array (heap corruption), a
     coordinate ≥ 32 collides with / runs past another chunk's slot.
     The whole world's chunks live in this single file, so the format
     itself must change (one region file per 32×32 chunk region,
     modern-`.mca`-style, or a hashed offset table). Old worlds need
     a migration: `chunks.dat` 0..31 maps to region (0,0); the
     storage-version machinery already exists for this
     (`SharedConstants::StorageVersion`, `ChunkVersion_Light = 1`,
     `ChunkVersion_Entity = 2` in `ExternalFileLevelStorage.cpp`).
     `LevelData.h` needs NO change: it stores no world size, only
     seed + spawn xyz (32-bit ints) + time + dimension + game type.
  8. `src/client/Minecraft.cpp:408-434` — `prepareLevel()` generates
     the ENTIRE world at load: `getTile()` per chunk over all 1024
     chunks + full light update + `clearUpdateMap()`. For infinite
     this must become "generate only the spawn area" (radius ~9
     chunks); the rest generates on demand as the player walks.
  9. `src/client/renderer/LevelRenderer.cpp:68-69` — the VBO pool is
     sized by `LEVEL_WIDTH`: `maxChunksWidth = 2*512/16+1 = 65` →
     `numListsOrBuffers = 65*65*(128/16)*3 = 101,400` GL buffers.
     The render grid itself is ALREADY player-relative and
     infinite-friendly: `allChanged()` (156-176) builds a 33×33×8
     grid (dist 512, `CHUNK_SIZE` 16) needing only 8,712×3 = 26,136
     buffers, and `resortChunks()` (247-295) wraps that grid around
     the player with modulo arithmetic. Fix = size the pool from the
     render distance (33-wide → 26,136), not from `LEVEL_WIDTH` —
     saves ~75k buffer objects. (`GameRenderer.cpp:92,589-590` needs
     no change: `renderDistance = 256 >> viewDistance`, far = 256
     blocks, fog START 0.8×256 ≈ 205 / END 256 — fog already hides
     the unloaded edge; the streaming load radius just has to exceed
     the fog end so terrain exists before the player reaches it.)
  10. `src/world/level/MobSpawner.cpp:60` — a REAL BUG found by this
      investigation, independent of infinite: enemy spawning checks
      `cx >= 0 && cx < 16 && cz >= 0 && cz < 16` — ABSOLUTE chunk
      coords 0..15, not 32, and not player-relative. Hostile mobs
      effectively only spawn while the player is in the world's
      south-west quarter (chunks 0..15). Friendly spawning (44-46)
      has the same 0..15 loop (`ChunkPos(i>>4, i&15)`). Both must
      become a player-relative radius in v17. (`Level::tick`'s own
      ±4-chunk poll offsets are already player-relative — fine.)
  11. Multiplayer only — `src/network/ClientSideNetworkHandler.h:100,
      103-104` + `.cpp:61,69,74-78,581,629-630`:
      `chunksLoaded[1024]` bool array, `NumRequestChunks = 1024`,
      `isChunkLoaded()` rejects outside coords,
      `areAllChunksLoaded()` counts to 1024. Single-player is
      unaffected; MP needs a dynamic request set (its
      `arrangeRequestChunkOrder()` already sorts around the player).
      `ChunkDataPacket.h:11` x/z are plain `int` — fine as-is.
  12. Small sites: `CommandServer.cpp:239-241` (fill command clamps
      to `LEVEL_WIDTH/DEPTH` — keep a clamp, just re-derive it),
      `:459,507` (checkpoint treats `CHUNK_CACHE_WIDTH` (32) as a
      BLOCK size: `cx = (int)e->x / 32` — wrong unit, debug feature,
      rework with streaming); `NetherReactorTileEntity.cpp:133,143`
      (item/enemy spawn positions re-rolled while outside [0,512) —
      drop the bounds in v17).
  ### Measured numbers (from the code, not paper)
  - Per chunk in RAM: blocks 32,768 B + data/skyLight/blockLight
    3×16,384 B + heightmap 256 + updateMap 256 ≈ 82,432 B ≈ 80.5 KiB
    (`LevelChunk.h`: `ChunkBlockCount = 256*128 = 32,768`;
    `DataLayer` nibbles are `length>>1`). On disk per chunk:
    82,176 B payload + 4 B length → 21×4,096 B sectors ≈ 84 KiB.
  - Today: 1,024 chunks ≈ 80.5 MiB chunk RAM (+ renderer VBO pool
    101,400 buffers). Walking the whole 512 world already keeps all
    1,024 chunks alive via `chunkMap` (see (3)) — memory is flat
    only because the world is capped.
  - Fallback re-measured (1024×1024 world): 4,096 chunks ≈ 322 MiB
    chunk RAM + VBO pool 129-wide → 129²×8×3 = 399,384 buffers +
    `RegionFile` offset table would need 4,096 entries. Still not
    viable on the user's realme 8 — the v3/v6 rejection stands, now
    with real numbers. Streaming is the only route to "infinite".
  - Streaming budget (design target for v16): resident window =
    render window. Render grid is 33×33 render-chunks (16-block) =
    ±16 chunks around the player → 1,089 level chunks ≈ 85 MiB,
    i.e. THE SAME memory as today — streaming does not save RAM at
    far view distance, it just lets the window MOVE. Eviction:
    Chebyshev radius 16 chunks from the player chunk, LRU; save
    dirty chunks on evict (mechanism already exists in
    `ChunkCache::getChunk` slot reuse + the `tick()` unsaved scan).
  ### Verdict: NO blocker makes infinite infeasible
  Terrain gen, biomes, the renderer grid, fog and `LevelData` are all
  already coordinate-free. The work is bounded: (a) `ChunkCache` →
  hash map + eviction, (b) `RegionFile` format → per-region files,
  (c) remove ~10 clamp sites, (d) fix the `MobSpawner` 0..15 cap,
  (e) dynamic MP chunk-request set. No engine rewrite.
  Known limit to document, not fix: entity positions are `float`
  (`Entity::x/y/z`). Past ~±1M blocks float32 precision visibly
  jitters movement (0.0625-block steps at 2^20); ~±100k blocks is
  ~0.008-block precision, fine. Modern Bedrock has the same float32
  property — "infinite" in practice means tens of thousands of
  blocks from spawn, not billions. No hard cap is needed for v16–v18;
  the limit is stated here so nobody is surprised at 1M blocks.
  ### v15 changed NO gameplay
  Only `ROADMAP.md` (this plan) + version bump 14 → 15. World is
  still 512×512, all v1–v14 features untouched. The APK is identical
  to v14 in play; it is only the stage marker for this plan.

- **v16 — Infinite world, stage 2: streaming core (SHIPPED).** The
  fixed 32×32 chunk cache is now a dynamic map + eviction; the world
  is STILL 512×512 (border stays — `RegionFile` cannot save outside
  0..31 yet, that is v17), but the resident window now moves with the
  player and old worlds still load.
  ### What shipped
  - `ChunkCache.h`: `chunks[1024]` → `std::map<int64_t,
    LevelChunk*>` keyed by the 64-bit `ChunkPos` hash; `fits()` is now
    the WORLD border (0..31), not the cache size; the shared
    `emptyChunk` is never stored in the map (the old slot array stored
    it — a double-delete waiting to happen); `saveAll()` saves the
    RESIDENT set instead of re-loading all 1024 chunks through
    `getChunk()`; `gatherStats()` reports the real resident count.
    Eviction runs from `tick()` (never on a `getChunk()` miss — the
    centre must be the PLAYER chunk, not whatever chunk a spawner or
    tile tick happened to request): every 10 ticks, chunks farther
    than Chebyshev radius 16 from the player chunk are evicted, max
    32 per sweep. Constants in `LevelConstants.h`
    (`CHUNK_STREAM_RADIUS = 16`, `CHUNK_STREAM_SWEEP_INTERVAL_TICKS
    = 10`, `CHUNK_STREAM_MAX_EVICT_PER_SWEEP = 32`,
    `CHUNK_SPAWN_GEN_RADIUS = 9`).
  - `ChunkPos.h`: hash key is now 64-bit (`(int64)x << 32 |
    (uint32)z`); the old 15-bit/axis key collided beyond ±32767
    chunks. This also fixes `Level::_chunksToPoll` (`std::set` ordered
    by the hash) merging distinct chunks.
  - `RandomLevelSource`: `chunkMap` key is `int64_t`; new
    `evictChunk(x, z)` removes a generated chunk from `chunkMap` and
    deletes it (`deleteBlockData()` + `delete`) — this is the ONLY
    place a generated chunk is ever freed (the destructor cleanup is
    still commented out), so without it RAM would grow without bound
    as the player walks. `ChunkSource` gains a default
    `evictChunk()` (returns false = "not mine, caller deletes") so
    disk-loaded chunks are deleted by the cache instead.
  - `Minecraft.cpp:prepareLevel()`: generates only the spawn area
    (radius 9 chunks, 19×19 = 361 chunks, centred on the stored spawn
    for old worlds / the centre for new ones) instead of all 1024 —
    load is ~3× less generation work. The rest generates on demand.
  - `Level.cpp:tickTiles()`: the 0..31 poll filter is dropped — the
    poll offsets are already player-relative (±4 chunks); outside
    coords just resolve to `emptyChunk` and tick nothing.
  - `LevelRenderer.cpp`: VBO pool sized from the render grid
    (33×33×8 render-chunks → 26,136 buffers) instead of
    `LEVEL_WIDTH` (65-wide → 101,400). Exact fit at far distance.
  - Two fixes that HAD to move forward from v17, or eviction would
    be undone: `ExternalFileLevelStorage::tick()` now scans the
    RESIDENT set (`ChunkSource::getResidentChunks()`) instead of
    calling `level->getChunk(x, z)` for all 1024 chunks every 50
    ticks (that re-loaded every evicted chunk + its light recompute);
    `ChunkCache::saveAll()` likewise saves residents only, so saving
    no longer re-generates the whole world for a new level.
  ### How the unload-stub risk was handled
  `LevelChunk::unload()` is still a stub (only `loaded = false`), so
  eviction NEVER deletes a chunk that holds live entities or tile
  entities: `evictChunk()` refuses (pins) any chunk with
  `countEntities() > 0` or a non-empty tile-entity map. Entities live
  in `Level::entities` + the chunk's `entityBlocks` index, tile
  entities in `Level::tileEntities` + the chunk's map — deleting the
  chunk object while either still references it would leave dangling
  pointers, and a reloaded chunk would create a DUPLICATE tile entity
  (empty chest/furnace) when `getTileEntity()` found an empty map.
  Pinning is bounded in v16: the world is still 512×512 (≤ 1024
  chunks, same as before), mob caps are tiny (monster 20 + creature
  15 + water 10 per level), tile entities are player-placed and few.
  Eviction saves UNCONDITIONALLY before deleting (blocks via
  `RegionFile`, entities via the global `entities.dat` snapshot —
  same as the old slot-reuse path): a freshly generated chunk starts
  with `unsaved = false`, so "save only if dirty" would silently drop
  never-saved generated terrain, and regeneration is NOT bit-identical
  (`postProcess` uses shared `Random` state). A second dangling-pointer
  trap was found while doing this: `ExternalFileLevelStorage` keeps raw
  chunk pointers in its pending-unsaved list, so eviction now calls a
  new `ChunkStorage::onChunkEvicted()` hook that drops the entry
  BEFORE the delete (otherwise `savePendingUnsavedChunks()` would
  write through a freed pointer). Eviction also clears the
  single-chunk `last` cache when it evicts that chunk.
  ### What v17 still owes this
  Real entity despawn/relocation on evict: pinning is safe only while
  the world is capped. Once the border lifts, the pinned trail (a
  chunk per wandering mob / placed chest) would grow without bound —
  v17 must despawn (or relocate) non-player entities in evicted
  chunks and detach tile entities properly before removing the
  border. Also still open: the per-chunk light recompute on reload
  (unchanged from before — same `updateLights` loop, just runs less
  often now that the tick scan no longer reloads everything).
  ### Verification (build-only until the realme 8 test)
  Local NDK r17c build (same toolchain as CI) compiles clean for
  arm64-v8a + armeabi-v7a; new symbols confirmed in the built
  `libminecraftpe.so` (`ChunkCache::evictChunk/evictFarChunks/
  getResidentChunks`, `RandomLevelSource::evictChunk`,
  `ExternalFileLevelStorage::onChunkEvicted`, `ChunkCache: %d`
  format string; old `ChunkCache: 1024` string gone). CI
  `./gradlew assembleDebug` SUCCESS (see run in the v16 report).
  Device proof still needed: walk far from spawn, resident count
  stays ≤ 1,089, walk back and the old area looks identical, edited
  chunks survive save/load, no crash on evict/reload cycles.

- **v17 — Infinite world, stage 3: border lifted + dynamic save/load
  (SHIPPED).** The world is no longer 512×512: the player can walk
  past x/z 0 and 512 and into negative coordinates, and terrain
  generates/saves/loads there. The v16 pinning debt is paid (see
  below), so the resident window can move without limit.
  ### What shipped
  - `RegionFile.h/.cpp` — PER-REGION save files. The chunk payload
    format inside a file is UNCHANGED (4 KiB sectors, first sector =
    1024-entry offset table, same bytes), but there is now one file
    per 32×32-chunk region, `r.<rx>.<rz>.dat` in the level directory
    (region = floor-divide chunk coord by 32), instead of one
    `chunks.dat` for the whole world. Negative chunk coords and
    coords ≥ 32 map to exactly one region file + one slot, so saving
    can no longer index before the offset array (heap corruption) or
    collide chunks. Region files open lazily and are capped at 16
    open (LRU — offset tables are written through on every write and
    rebuilt from disk on open, so closing one loses nothing; the cap
    bounds file descriptors on long walks). Migration: on first open
    of region (0,0), a legacy `chunks.dat` (exactly region 0..31) is
    RENAMED to `r.0.0.dat` (same bytes, new name); if the rename
    fails the legacy file is used as-is for region (0,0) so no data
    is lost. Old 512 worlds keep loading with all chunks intact.
  - `ChunkCache.h` — the border is gone: `fits()` (0..31) deleted;
    `hasChunk()` now means exactly "resident in the cache";
    `getChunk()` loads/generates a real chunk at ANY coordinate
    (no more shared `emptyChunk` for outside coords);
    `postProcess()` / `load()` lose their `fits()` guards. The v16
    streaming window (Chebyshev radius 16, sweep every 10 ticks,
    max 32/sweep) is unchanged.
  - Entity + tile-entity eviction (the v16 debt, PAID):
    `LevelChunk::collectEntities()` + `detachTileEntities()` (new).
    `evictChunk()` no longer pins chunks with entities/tile
    entities. ENTITIES: every non-player entity in the evicted chunk
    is despawned via `Level::removeEntity()` (marked removed;
    `Level::tickEntities()` deletes it on the next game tick). Safe
    because `Entity::save()` already refuses removed entities, so a
    despawned mob is never written back to `entities.dat` and can
    never resurrect/duplicate on reload. A player entity in the chunk
    still pins it (checked FIRST, before any despawn). TILE
    ENTITIES: detached, not deleted — `detachTileEntities()` only
    clears the chunk's map (no `setRemoved()`, no delete); the
    objects stay alive in `Level::tileEntities` with their contents,
    the eviction's `saveEntities()` writes them to `entities.dat`,
    and `ChunkCache::reattachTileEntities()` (new, runs in
    `getChunk()` right after a chunk loads, before postProcess or any
    `getTileEntity()` call) puts the ORIGINAL tile entity back into
    the chunk map, so a chest/furnace keeps its contents and is never
    duplicated (guarded by `hasTileEntityAt()`).
  - `MobSpawner.cpp` — the v15 real bug, FIXED: hostile spawning
    dropped the absolute `cx >= 0 && cx < 16 && cz >= 0 && cz < 16`
    check (the loop was already player-relative ±8 chunks, so no
    check is needed); friendly spawning dropped its absolute
    0..255-chunk loop (`ChunkPos(i>>4, i&15)`) for the same ±8-chunk
    player-relative radius. Hostile mobs now spawn anywhere, not
    just in the south-west quarter.
  - Border clamps removed: `Level.cpp` `setInitialSpawn()` /
    `validateSpawn()` (the [4,508] wander clamps), `Level::clip()`
    (the `[0, LEVEL_WIDTH)` gate — `t > 0` already proves a real
    block is there); `Entity::loadFromTag()` (the x/z load clamp
    that teleported entities back inside on load);
    `ExternalFileLevelStorage::readPlayerData()` (the v1 player-pos
    clamp to [0.5, 511.5]); `Minecraft::prepareLevel()` (the
    0..31 spawn-area clamps — for a bed spawn outside 0..31 they
    made cx1 < cx0 and generated NOTHING at load; spawn chunk now
    floor-divided so negative spawns land right);
    `CommandServer.cpp` fill (x/z clamps dropped, y clamp kept);
    `NetherReactorTileEntity.cpp` (the [0,512) spawn re-roll loops).
  - Two fixes found while doing this: `ExternalFileLevelStorage`
    pending-unsaved key was `x + z * 32` (an int) — it COLLIDES for
    negative/large coords (chunk (-1,0) and (31,-1) both give -1),
    so two chunks shared one entry and one would never save; it is
    now the 64-bit `ChunkPos` hash (`UnsavedLevelChunk::pos` is
    `int64_t`). `CommandServer` checkpoint used `CHUNK_CACHE_WIDTH`
    (32) as a BLOCK size — wrong unit, chunk coords off by 2× AND
    `getBlocksAndData` fed a 32-wide range (buffer over-read/write
    past the 16×128×16 chunk); now `CHUNK_WIDTH` (16) with
    `Mth::floor` chunk coords, consistent with the restore path and
    `numChunkBytes`. `deleteLevel()` fallback now also removes
    `r.*.dat` files.
  - `LevelConstants.h`: `CHUNK_CACHE_WIDTH` / `LEVEL_WIDTH` /
    `LEVEL_DEPTH` no longer cap anything — they survive only as the
    default spawn centre (new worlds still spawn at chunk (16,16)
    = block (256,256)).
  ### Deviations from the v17 plan (in "Next" before this version)
  - Region files are flat `r.<rx>.<rz>.dat` in the level directory,
    NOT `region/r.<rx>.<rz>.dat` as the plan sketched — same effect,
    one less directory to create/manage.
  - The storage version was NOT bumped: the chunk payload format is
    byte-identical inside each region file and the migration is a
    rename, so the existing version/converters still apply.
  - `Level::inRange()` + multiplayer `ClientSideNetworkHandler`
    (`chunksLoaded[1024]`, `NumRequestChunks = 1024`) were LEFT
    untouched: MP needs a dynamic request set AND matching
    server-side changes, a separate subsystem — single-player is
    unaffected. MP still plays a 512 world; see "Known limits".
  - `clip()`: the plan said fix the Z check to `LEVEL_DEPTH` then
    remove both bounds; the gate was removed outright instead
    (`t > 0` is the sufficient check).
  ### Known limits / caveats (honest, for v18 to verify on device)
  - Dropped items + XP orbs DESPAWN when their chunk evicts (they
    are despawned like mobs). This tree has no per-chunk entity
    save — `entities.dat` is one global snapshot — so keeping them
    alive detached would leave them ticking in unloaded chunks. An
    item dropped and then left >16 chunks behind is lost (modern MC
    would keep it). Mobs despawning at distance matches modern MC
    (hostiles already self-despawn past 96 blocks; animals never
    self-despawn, which is exactly why eviction must despawn them).
  - Multiplayer still 512×512 (see above).
  - Entity positions are `float` (v15 note stands): past ~±100k
    blocks precision visibly degrades; ~±1M blocks it jitters.
    No hard cap added; "infinite" in practice means tens of
    thousands of blocks from spawn.
  - A mob that walks out of the resident window BETWEEN eviction
    sweeps (not via its chunk evicting) is not despawned by this
    version — it keeps ticking with `inChunk == false` (pre-existing
    pattern, also possible in v16 inside 0..31). Rare; v18 to check.
  ### Verification (build-only until the realme 8 test)
  CI `./gradlew assembleDebug` SUCCESS (see run in the v17 report);
  APK verified locally (versionCode 17, signatures, `unzip -t`,
  sha256). Device proof still needed (v18): walk/teleport past
  x=512 and z=512 and into negative coords — terrain continues, no
  vertical edge, no blue void; edit blocks beyond the old border,
  quit, reload, edits persist; OLD 512 world loads and its
  `chunks.dat` migrates to `r.0.0.dat` (check the world dir);
  chest with items placed far from spawn, walk away >16 chunks,
  walk back, contents intact, no duplicate chest; mobs spawn at
  night far from origin (the old 0..15 bug is gone); 10-minute
  walk, memory flat. Only after v18 passes does any version claim
  "infinite".

- **v18 — Infinite world, stage 4: verification + polish (SHIPPED,
  build-only).** Code audit of the v16/v17 streaming system + three
  targeted fixes. No gameplay change; v1–v17 features untouched.
  ### Audit results (what was checked, what was found)
  1. **Remaining fixed-world assumptions — AUDITED, none left in
     single-player.** Searched for `CHUNK_CACHE_WIDTH` / `LEVEL_WIDTH` /
     `LEVEL_DEPTH` as world bounds, `chunksLoaded[1024]`, `x + z*32`
     indexing, `& 31` / `% 32` slot math and coordinate clamps. What is
     left is DELIBERATE, not a leak: `Level::inRange()` (only caller is
     the MP `ClientSideNetworkHandler` block-update gate) and the whole
     `ClientSideNetworkHandler` chunk-request set (`chunksLoaded[1024]`,
     `NumRequestChunks = 1024`) — MP stays 512×512, needs matching
     server-side changes; `CHUNK_CACHE_WIDTH` / `LEVEL_WIDTH` /
     `LEVEL_DEPTH` survive only as the default spawn centre (new worlds
     spawn at chunk (16,16) = block (256,256)); `x >> 4` / `x & 15`
     block-to-chunk math is floor-correct for negatives on this
     toolchain (arithmetic shift — the pattern is used tree-wide in
     `Region`, `hasChunksAt`, `Level::getTile`) and `Mth::floor` is a
     true floor; `Level::MAX_LEVEL_SIZE` is 32,000,000, not a 512
     clamp. One stale v16 comment fixed (`Level::tickTiles` claimed
     out-of-range coords resolve to the shared `emptyChunk` — wrong
     since v17, `getChunk()` generates/loads a real chunk anywhere).
  2. **Mob walking out of the resident window between sweeps (the v17
     caveat) — TRACED + SAFETY NET ADDED.** The trace: a mob CANNOT
     walk out in one tick — `Level::tick(Entity*)` refuses to tick any
     entity whose ±32-block neighbourhood is not fully resident
     (`hasChunksAt` gate), so a mob freezes at the window edge (chunk
     distance 15–16) before it can leave, and `evictChunk()` despawns
     every entity still indexed in an evicted chunk. The leak class is
     narrower than the caveat feared: only an entity with
     `inChunk == false` is indexed NOWHERE, so no sweep can find it —
     and reaching that state needs the chunk-change to run while the
     new chunk is not resident, which normal movement (≤1 chunk/tick)
     and even the enderman teleport (±16 blocks, ≤1 chunk) cannot do
     after the gate passed. But the class is real if ANY path ever
     produces it, and the cost is not just memory: a leaked entity
     stays in `Level::entities` forever, still counts toward the
     `MobSpawner` caps (`countInstanceOf*` walks the whole list, so
     leaked animals/monsters suppress new spawns) and is still written
     to `entities.dat` on save (resurrecting on the next load). Fix
     (`Level::tick`): when the `hasChunksAt` gate fails, a
     NON-PLAYER entity whose OWN chunk is not resident is despawned
     (`removeEntity`, same call eviction uses) instead of freezing.
     An entity whose own chunk IS resident keeps the old freeze
     behaviour (resumes when the player walks back — no gameplay
     change for in-window mobs); the player is never despawned; and
     the net is single-player only (`!isClientSide` — on a client the
     entities are server-authoritative).
  3. **Not-yet-generated chunks — VERIFIED + PRE-GEN RING (the safe
     option, not a loading pause).** First, the scary claim was
     checked and is FALSE for physics: the player can never walk into
     air — collision (`Level::getCubes` → `getTile` → `getChunk`) and
     the renderer (`Chunk::rebuild` reads through `Region`, whose
     constructor calls `level->getChunk`) both generate a chunk
     synchronously on demand, and `updateDirtyChunks()` rebuilds every
     render chunk within 32 blocks of the player every frame. What
     was real is the COST: walking into new terrain burst several
     chunk generations (Perlin + features + mob spawns) into one
     frame's near-chunk rebuilds, and far render chunks queue behind
     the 3-per-frame rebuild budget. Fix: `ChunkCache::
     pregenerateNearChunks()` runs on the same 10-tick sweep as
     eviction, after it — radius 3 chunks around the player chunk
     (covers the 32-block near-rebuild band + 1 chunk margin), max 4
     missing chunks per sweep, nearest ring first, skipped when there
     is no generator (MP client cache). The two windows are disjoint
     (pre-gen 3, eviction 16), so pre-gen can never resurrect a chunk
     the same sweep just evicted. A loading pause was rejected: it
     would freeze input/movement for a problem that is only a frame
     hitch, and the renderer was not to be touched.
  4. **Negative coordinates + eviction tuning — AUDITED, no tuning
     change.** Floor-division re-checked on every path that touches
     negative chunk coords: `RegionFile::floorDivRegion` (v17),
     `prepareLevel` spawn floor-div (v17), `evictFarChunks` manual
     floor (v17), `reattachTileEntities` / `Level::getTile` /
     `hasChunksAt` / `Region` `>> 4` shifts (floor on NDK),
     `MobSpawner` / `tickTiles` / `LevelChunk::addEntity`
     (`Mth::floor`, true floor), `CommandServer` fill (y clamped
     only) + checkpoint (`Mth::floor`), `NetherReactor` spawn offsets
     (±7 blocks, no bounds). All correct. Eviction tuning VERIFIED,
     not changed: radius 16 matches the render grid exactly (33×33)
     and the fog end (256 blocks); sweep 10 ticks / max 32 per sweep
     drains a teleport in ~34 sweeps (~17 s) — bounded, no blind
     tuning.
  5. **`chunkMap` growth — PROVEN BOUNDED + one teardown fix.**
     Sole inserter is `RandomLevelSource::getChunk`, and the ONLY
     caller of it is `ChunkCache::getChunk` on a cache miss (every
     `Level` / `Region` / spawner / tile path funnels through
     `Level::getChunk` → the cache, so every generated chunk is
     resident in the cache). Sole remover is
     `RandomLevelSource::evictChunk`, called only from
     `ChunkCache::evictChunk` on the sweep. Disk-loaded chunks never
     enter `chunkMap` (created by `storage->load`, deleted by the
     cache on evict). `saveAll` / the storage 50-tick scan iterate
     the RESIDENT set only (`getResidentChunks`), never re-loading
     evicted chunks; `savePendingUnsavedChunks` entries are dropped
     by `onChunkEvicted` before the delete; `deleteLevel` removes
     `r.*.dat`. The one hole found: the `ChunkCache` DESTRUCTOR
     deleted resident chunks directly while `chunkMap` still pointed
     at them (harmless only because the source was deleted right
     after) — it now routes each chunk through
     `source->evictChunk()` (erases from `chunkMap` + deletes),
     falling back to a direct delete for disk-loaded chunks.
  ### Verification (build-only until the realme 8 test)
  Local NDK r17c `ndk-build` (same toolchain as CI) compiles clean
  for arm64-v8a, full `libminecraftpe.so` link OK; CI
  `./gradlew assembleDebug` SUCCESS (see run in the v18 report).
  **"Infinite" is STILL NOT claimable after v18.** v18 proves from
  code that nothing bounds the world, nothing leaks per the audit,
  and generation is spread ahead of the player — but the claim needs
  the device test: 10-minute walk past the old border in every
  direction + negative coords, memory flat, FPS within ~10% of v14
  at spawn, no void hole, no crash, edits + far chest + far night
  mobs surviving reload. Until that test passes, v18 is "infinite in
  code, unverified on device".
  ### Debts for v19 (found, not fixed — not tuning-blind, just scoped)
  - `evictChunk()` rewrites the whole `entities.dat` on EVERY
    evicted chunk (up to 32 rewrites in one sweep while a teleport
    drains) — wasteful I/O burst, correctness is fine; save the
    snapshot once per sweep instead.
  - The 10-minute device walk itself (see "Verification" above).

- **v19 — Music + ambient sounds (SHIPPED, build-only).**
  ### Audit (read from code, `src/client/sound/`)
  All 102 "sound files" are raw-PCM C arrays in
  `src/client/sound/data/*.pcm` (`unsigned char PCM_<name>[N]`), NOT
  ogg/mp3 — and every one is a SHORT effect: stereo 16-bit 44.1 kHz,
  longest is the creeper fuse hiss (~1.4 s), most are 0.3–1.0 s.
  There is NO music file, NO bird/cricket/wind/ambience file, and NO
  audio decoder in the tree (the Android backend,
  `SoundSystemSL`, only enqueues raw PCM through OpenSL; max 4
  simultaneous buffers, one-shot only, no loop, no stop, and it
  ignores the `pitch` parameter entirely). Recorded music was
  therefore NOT feasible in one batch — v19 synthesises its
  music/ambience instead (below). What played before v19: 40
  registered names (`SoundEngine::init`) covering steps (6 surfaces),
  dig/break (via `TileSoundType`), mob ambient/hurt/death for
  pig/cow/sheep/chicken/zombie/skeleton/spider/creeper/zombiepig,
  eat, bow, fuse, explode, splash, click, pop/pop2, door, glass,
  hurt, fall damage. What was SILENT despite a live call site:
  `random.drink` (potion drinking), `random.burp` (after eating),
  `random.fizz` (item in lava/water), `fire.ignite` (flint & steel),
  `random.chestopen`/`random.chestclosed` (chest use) — no file and
  no registration for any of them, so `SoundRepository::get` only
  logged "Couldn't find a sound". Also silent: slime + enderman
  (`Slime`/`Enderman::getAmbientSound` return `mob.slime` /
  `mob.enderman*`, none registered, no files). Three files existed
  but never sounded: `chicken1.pcm` (include + definition commented
  out, so `mob.chicken` had only 2 variants), `chickenplop.pcm`
  (never included at all; its call site in `Chicken::tick` was
  commented out along with the whole egg-lay block), and
  `skeletondeath.pcm` (file + `SoundDesc` existed but the name was
  never registered AND `Skeleton::getDeathSound` returned
  `mob.skeletonhurt`). One quirk found, NOT a bug: `SA_pop2` looks
  undefined in `Sound.cpp`, but `data/pop2.pcm:583` itself contains
  the `SoundDesc SA_pop2(...)` definition at the end of the array.
  ### What shipped
  - Procedural music (`SoundEngine::_generateMusic` /
    `_makeMusicTrack`, `SoundEngine.h/.cpp`): two 18 s calm tracks
    synthesised once at startup as mono 16-bit 22.05 kHz PCM
    (~0.8 MB each) — a quiet 3-note pad (C major / A minor) plus a
    slow random-walk pentatonic melody (sine + octave harmonic,
    attack/decay envelope). Scheduler in `SoundEngine::update`:
    first track ~10 s after entering a world, then a random track
    every 45–120 s (MC-style gap). Only while in a world and not
    paused; nothing plays on the title screen (player == NULL).
  - Procedural ambience (`_makeChirp`, same scheduler): bird-chirp
    bursts (3 descending glissandi) by day, cricket pulses (4.2 kHz,
    45 ms on/off) by night (`Level::isDay()`), every ~20–45 s.
  - Music volume is `options->music` (`_playMusic`); SFX volume
    (`options->sound`) no longer gates the scheduler — before v19,
    `update()` returned early when `sound == 0`, which would have
    killed music too. The Options screen never exposed either
    slider (only sensitivity was live), so MUSIC + SOUND sliders
    were added to the Mojang pane (`OptionsScreen.cpp`); the
    `options.music` / `options.sound` lang strings already existed.
    Caveat: `Options::load()` is entirely commented out tree-wide,
    so the slider is per-session like every other option here.
  - Silent-sound fixes (no new files): `chicken1` compiled in
    (`mob.chicken` now 3 variants); `chickenplop` compiled in +
    registered + the egg-lay block in `Chicken::tick` re-enabled
    (field `eggTime` + `Item::egg` already existed — chickens now
    lay an egg every 5–10 min with the plop sound); `skeletondeath`
    registered + `Skeleton::getDeathSound` now returns it; slime
    ambient/hurt/death now return `random.pop` (placeholder squelch
    — closest existing sound); `random.chestopen/closed` →
    door open/close, `random.drink`/`random.burp` → eat sounds,
    `random.fizz` → splash, `fire.ignite` → fuse hiss.
  ### Known limits / debts
  - The music is synthesised, not recorded Minecraft music (no
    C418 tracks in the tree, no decoder) — it sounds like calm
    generative chimes, not the real soundtrack. A real track would
    need a licensed/added audio file + a decoder or a Java-side
    MediaPlayer path — out of scope for this batch.
  - Enderman is STILL silent (`mob.enderman`, `.hit`, `.death` —
    no file exists; mapping it to zombie sounds would misattribute
    the mob, so it was left alone, not faked).
  - A track already playing cannot be stopped: the OpenSL backend
    has no stop call, so muting music mid-track (or pausing) lets
    that ≤18 s track play out. `pitch` is ignored by the backend
    for ALL sounds (pre-existing, not v19).
  - If 4 SFX buffers are already playing, a starting music/ambience
    buffer is dropped silently (`MAX_BUFFERS_PLAYING = 4`).
  - All of it is build-only until a live test on device (first
    track ~10 s in, day chirps vs night crickets, old sounds still
    audible, no crash/loop).

- **v20 — Brewing (SHIPPED, build-only).**
  ### Audit (read from code before writing anything)
  - NO brewing existed: no BrewingStand tile/tile entity, no potion
    items, no brewing UI, no recipe table. Residue only: commented-out
    brewing-stand + cauldron recipes in `Recipes.cpp`, `ContainerType::
    BREWING_STAND = 5` in `Container.h`, and a commented-out
    `setId(BrewingStandTileEntity.class, "Cauldron")` in
    `TileEntity.cpp`.
  - NO status-effect system existed (FoodItem::setEatEffect is
    commented out; v7/v9 noted the same gap for rotten flesh / golden
    carrot). Potion effects needed a small new system (see below).
  - Nether Reactor existed with a 8-way loot table
    (`NetherReactorTileEntity::getSpawnItem`): glowstone dust x3,
    melon seeds, brown/red mushroom, reeds, cactus, nether quartz x4,
    low-odds junk. No nether wart / blaze rod / blaze powder / ghast
    tear anywhere in the tree. No soul sand tile either (so nether
    wart is an item, not a farmable crop — see caveats).
  - Everything brewing needed already existed as a pattern: the
    furnace tile entity + screen + menu (container, tick, save/load,
    touch UX), DyePowderItem's per-aux icon/name, FoodItem's
    hold-to-drink flow (`use` -> `startUsingItem` -> `useTimeDepleted`,
    `UseAnim::drink` already handled in `Player.cpp:274`), the
    `random.drink` sound (mapped in v19), and lang strings for every
    brewing item + every potion name (`item.potion.*.name`,
    `potion.*.postfix`) already in `en_US.lang`.
  ### What shipped
  - Potion effects (new, on `Mob`): 7 tick-down timers
    (`speed/slowness/strength/weakness/fireResistance/regeneration/
    poisonEffectTicks`, decremented at the top of `Mob::tick` before
    the fire damage in `Entity::baseTick`). Speed x1.2 / slowness
    x0.85 multiply `LocalPlayer::getWalkingSpeedModifier` (stacks with
    the v6/v14 sprint 1.4x); strength +3 / weakness -1 damage in
    `Player::attack`; fire resistance overrides `Mob::burn` /
    `Mob::lavaHurt` and extinguishes `onFire`; regeneration heals 1 HP
    / 50 ticks, poison hurts 1 HP / 25 ticks never below 1 HP (modern
    MC cadences).
  - `PotionItem` (id 373, one item, 11 aux types): water, awkward,
    healing (+4 HP), swiftness, strength, fire resistance (all 3:00),
    regeneration / poison (0:45), weakness / slowness (1:30),
    harming (6 HP). Drink = 32-tick hold, `UseAnim::drink`, becomes
    an empty glass bottle after drinking. Per-aux bottle icon
    (items.png row 12) + per-aux name (`item.potion.<type>.name`).
  - `GlassBottleItem` (id 374): crafted from 3 glass; fill from a
    water block (tap it) or while standing in water (tap with no block
    hit) — both paths, because either tap path may not hit water on
    this old client. Water is not consumed (modern MC).
  - Brewing stand (tile 117, modern MC id): `BrewingStandTile`
    (EntityTile, cube + drawn stand texture in terrain.png cell
    (4,11)) + `BrewingStandTileEntity` (5 slots: 3 bottles, ingredient,
    fuel; 400-tick brew, 1 blaze powder = 20 brews, saves/loads like
    the furnace) + `BrewingMenu` + `BrewingScreen` (furnace touch UX:
    ingredient top-left, fuel below it, 3 bottle slots in a column on
    the right, brew arrow + fuel flame from the furnace spritesheet,
    ghost-icon + name preview of what the first brewable bottle
    becomes). `Player::openBrewingStand` virtual + LocalPlayer screen
    + ServerPlayer packet + `ClientSideNetworkHandler` BREWING_STAND
    case, mirroring the furnace exactly.
  - `BrewingRecipes`: water + nether wart -> awkward; awkward +
    glistering melon / sugar / blaze powder / magma cream / ghast tear
    / spider eye / fermented spider eye -> healing / swiftness /
    strength / fire resistance / regeneration / poison / weakness;
    fermented spider eye corrupts healing/poison -> harming,
    swiftness -> slowness.
  - Crafting (Recipes.cpp): brewing stand (blaze rod + stone — no
    cobblestone tile exists in this tree; the original commented
    recipe used stoneBrick), glass bottle (3 glass -> 3), gold nugget
    (1 ingot -> 9), glistering melon (melon + 8 nuggets), fermented
    spider eye (eye + sugar + brown mushroom, shapeless), magma cream
    (blaze powder + slimeball, shapeless), blaze powder (1 rod -> 2,
    shapeless).
  - Nether-ingredient source (the ROADMAP's "Nether-reactor loot
    change" alternative to waiting for v21): the reactor loot table is
    now 12-way — the old 7 items plus nether wart x3, blaze powder x2,
    blaze rod x1, ghast tear x1 per spawn slot (~200 items per full
    run, so ~16 of each). Nothing else about the reactor changed.
  - Spider eye drop enabled (was commented out in `Spider.cpp`):
    1/3 chance of 1 eye, string 0-2 unchanged.
  - v18 debt PAID (safe): `evictChunk()` no longer rewrites the whole
    `entities.dat` per evicted chunk — the sweep writes the snapshot
    once afterwards. Safe because the snapshot is global
    (`ExternalFileLevelStorage::saveEntities` ignores its chunk
    argument) and it is already only written once per 60 s by the
    storage tick anyway; the post-sweep write holds the same
    post-eviction state the per-chunk writes had.
  - Drive-by fix: `MAX_ITEMS` 512 -> 1024. v12's ender pearl (ctor id
    368 -> full id 624) already writes/reads past the 512-entry
    `Item::items` array — an out-of-bounds access that only "works"
    because the same slot is read back. No ids change; the array just
    covers it now.
  ### Known limits / debts
  - The brewing stand renders as a full cube with the drawn stand
    texture, not the modern 3D stand model (no custom non-cube tile
    renderer exists in this tree — same precedent as the v10
    enchanting table).
  - No nether wart farming: no soul sand tile exists, so wart only
    comes from the reactor (and is consumed on brew). No redstone
    (extend) / glowstone (level II) brewing — no potion-level system
    to upgrade yet. No splash / lingering potions (no throwable use
    for the item yet), no cauldron, no witch yet (witch stays queued
    after brewing, as planned).
  - Brewing-stand base is stone, not cobblestone (no cobblestone tile
    in this tree). Weakness is -1 damage (approximation, not the exact
    modern value).
  - All of it is build-only until a live test on device (craft the
    stand, run the reactor, brew awkward -> healing/swiftness, drink,
    check the effects, check the stand UI).

- **v21 — Nether dimension (SHIPPED, build-only).**
  ### Audit (read from code before writing anything)
  - A dimension system EXISTED but was never used for the Nether: `Dimension` base (`NORMAL=0`, `NORMAL_DAYCYCLE=10`) + `NormalDayCycleDimension`; `Dimension::getNew(id)` only handled those two; `DimensionFactory` picks by game type; `Level` accepts `fixedDimension`; `LevelData` has a `dimension` field the factory ignores. `Minecraft::toggleDimension(){}` was an empty stub (`Minecraft.h:68`).
  - NO portal tile existed: `Tile::portalTile` declared in `Tile.h:219` but never defined/registered. NO soul sand tile either: `Tile::hellSand` declared but never defined, registration commented out in `Tile.cpp:269`, no `HellSandTile` class.
  - Portal ingredients existed: obsidian tile 49, flint & steel item (id 3) — but `FlintAndSteelItem::useOn` was COMMENTED OUT, so flint & steel did nothing at all.
  - Nether blocks partly existed: netherrack 87, nether brick 112, quartz block 155, glowstone = `LightGemTile` 89 (drops yellow dust). Quartz ore (id 153) and nether wart crop (id 115) were FREE ids; portal id 90 was FREE.
  - Mobs: PigZombie existed (neutral, `fireImmune=true`, gold sword, `mob.zombiepig.*` sounds). NO ghast, NO blaze, NO magma cube. NO fireball projectile (`Arrow`/`Throwable` existed; skeleton ranged pattern is old-AI `checkHurtTarget`, every 3 s). `FlyingMob` existed (no gravity in air).
  - The Nether Reactor is NOT a dimension: it converts the structure to glowing obsidian and spawns items/mobs in the OVERWORLD (~200 items/run incl. v20 wart/blaze/rod/tear). Left unchanged as a legacy placeholder.
  - Storage: `ExternalFileLevelStorage` per level dir; per-region files `r.<rx>.<rz>.dat`; the player tag lives INSIDE `level.dat`. Consequence: a second storage dir for the Nether would get its own player tag — handled by the omit flag (below).
  - Spawning: `MobSpawner::tick` -> `Level::getRandomMobSpawnAt` -> `ChunkSource::getMobsAt` (virtual) — the Nether spawn list lives in the new source. `isSpawnPositionOk` (solid below + 2 air) works on the Nether floor; `Monster::canSpawn` darkness check passes because `hasCeiling` leaves sky light at 0.
  - Rendering/sky: `dimension->foggy` -> no sky plane/sun/moon/clouds + fog starts at camera; `hasCeiling` skips sky light; fog colour via `Dimension::getFogColor`; `ultraWarm` already branched in `LiquidTileDynamic.h:23`.
  - Textures: `terrain.png` 256x256; NO uniform-magenta cells left, but light-purple placeholder cells (214,127,255) were free and verified unused by any tile tex: 170, 171, 172, 173, 216, 217, 231.
  - Lang strings already existed for soul sand, glowstone, portal, nether wart; `tile.quartzOre.name` was missing and was added.
  ### What shipped
  - **Second dimension, real:** `NetherDimension` (`id = -1`, `foggy`, `hasCeiling`, `ultraWarm`, no day cycle, red fog 0.2/0.03/0.03, `NetherLevelSource` as its chunk source) registered in `Dimension::getNew`. The world keeps TWO `Level` objects alive in `Minecraft` (`overworldLevel`/`netherLevel`); the Nether storage lives at `<world>/DIM-1` and is created on first entry with a pre-seeded `level.dat` (same seed/gameType, spawn at the arrival portal) so generation centres on the portal.
  - **Portal:** obsidian frame (any size 4–23 wide, 5–23 high, either plane) lit by flint & steel — `FlintAndSteelItem::useOn` restored (it was commented out) and extended with a frame search; otherwise it places fire as before. `PortalTile` (id 90, tex 170, `SHAPE_CROSS_TEXTURE`, non-solid, light 11/16, collapses when its frame is gone). Entering a portal requests a switch; the switch is deferred to `Minecraft::tick` (never inside a level tick), fires only on ENTERING (no bounce-back), 5 s cooldown. Coordinate scale 1:8 like modern MC. The destination portal is found within 16 blocks or built fresh (4x5 obsidian frame on solid ground) in `_levelGenerated()`; return position is stored at entry.
  - **Player-tag safety:** the Nether storage sets `omitPlayerFromLevelDat`, so its `level.dat` never carries a Player compound (re-entering can never restore a stale inventory/position over the live player). The live player is detached with a new `Level::detachEntity` (does NOT set `removed=true` — `removeEntity` would freeze the player forever) and re-added by `setLevel`. Quitting while in the Nether merges the live player (pos/inventory/XP, incl. Nether loot) into the overworld tag at the return position (`leaveGame`); both dimension levels + storages are deleted on quit/app exit.
  - **Nether terrain** (`NetherLevelSource`): bedrock floor y=0 (+scatter y1–4) and ceiling y=127 (+scatter y126), rolling netherrack floor height 28–62 (value noise), lava sea y<=31 where the floor dips below 32, soul sand patches (noise>0.78, top 3 layers), quartz ore ~1.2% of netherrack y10–110, glowstone clusters under the ceiling y120–125. No structures.
  - **Nether mobs:** Ghast (4x4x4, 10 HP, flies — `FlyingMob`-style travel, no gravity; fireball ≤28 blocks every 3 s; drops 0–1 ghast tear + 0–2 gunpowder; cluster size 1) and Blaze (20 HP, fireball ≤16 blocks every 3 s + melee fallback; drops 0–1 blaze rod). Both fire-immune. New `Fireball` (`Throwable`, no gravity, explodes radius 1 with fire on hit). Spawn list: pig zombie 10, blaze 6, ghast 4 (weights; pig zombie already existed). New models/renderers + 64x32 textures (`mob/ghast.png`, `mob/blaze.png`); fireball renders as billboarded orange/yellow quads (no texture).
  - **Nether wart farming:** `HellSandTile` (soul sand, id 88, tex 171) + `NetherWartTile` (crop, id 115, 4 stages, tex 173/216/217/231, grows without light, only on soul sand, mature drops 2–4) + wart item is now a `SeedItem` (id 116, plants on soul sand). Quartz ore tile (id 153, tex 172, drops quartz). Soul sand/quartz ore/glowstone added to the creative inventory; `tile.quartzOre.name=Nether Quartz Ore` added to `en_US.lang`.
  - **Sounds are placeholders** (see debts): ghast ambient silent, hurt `random.pop`, death `random.explode`; blaze hurt `random.fizz`, death `random.explode`; fireball launch `random.bow` at pitch 0.6.
  - Version bump 20 -> 21 (`app/build.gradle`, `AndroidManifest.xml`, `build-apk.sh`). Local `ndk-build` (r17c, both ABIs) passes: `libminecraftpe.so` arm64 10,072,904 B / armv7 8,695,684 B, zero errors.
  ### Known limits / debts
  - No Nether fortress / nether brick structures, no magma cube, no wither skeleton. No beds-explode behaviour in the Nether (beds behave as in the Overworld). Portal renders as crossed quads, not a flat plane (same `SHAPE_CROSS_TEXTURE` precedent as the web tile).
  - Ghast/blaze/fireball sounds are placeholders (above); no `mob.ghast.*` / `mob.blaze.*` sounds exist in the sound table yet.
  - Crash while in the Nether (not a clean quit) may lose Nether loot: the live player state is only merged into the overworld tag by `leaveGame()`; the Nether level itself never writes a player tag by design. A crash-safe variant (write the player into the overworld tag on every dimension switch, or on autosave) is a v22+ hardening item.
  - Multiplayer dimension switching is NOT implemented (single-player only: `requestDimensionChange` returns early on `level->isClientSide`).
  - All of it is build-only until a live test on device (build an obsidian frame, light it with flint & steel, enter, check terrain/lava/glowstone/fog, ghast/blaze spawns, return portal to the Overworld, no crash, Overworld world intact after return; quit while in the Nether and reload — loot must survive).

- **v22 — The End (SHIPPED, build-only).**
  ### Audit (read from code before writing anything)
  - The v21 dimension system reused cleanly: `Dimension` base +
    `Dimension::getNew` (only NORMAL/NORMAL_DAYCYCLE/NETHER handled),
    `Minecraft::toggleDimension` (was the Nether toggle),
    `requestDimensionChange` / `changeDimension` (Nether-only guards),
    `getOrCreateNetherLevel` (storage `<world>/DIM-1`, pre-seeded
    `level.dat`, `omitPlayerFromLevelDat`), `Level::detachEntity`,
    `findOrCreatePortal`, and the Nether `LevelSource` pattern (no
    chunkMap, deterministic hashes). A third Level object + a third
    storage dir was the natural extension — no new framework needed.
  - Enderman already existed (v12: 20 HP, teleport on hurt, drops
    0–2 ender pearls, item id 368). Ender pearl existed as an item
    but had NO use (no throwing). Eye of ender did NOT exist as an
    item (only a commented-out recipe in `Recipes.cpp:534`); the
    lang string `item.eyeOfEnder.name` already existed.
  - NO End content existed: no end stone / portal frame / portal /
    dragon egg / chorus tile (ids 119–122 and 240 were all FREE —
    modern MC ids, used as-is), no End dimension (id 1 free), no
    dragon (no `MobTypes` entry, no model/renderer), no chorus.
    `tile.dragonEgg.name` and `tile.whiteStone.name` ("End Stone")
    already existed in `en_US.lang`.
  - Stronghold generation does NOT exist in this tree (no structure
    generation beyond dungeons + the Nether reactor) and is not
    feasible in one batch. DECISION (recorded, not faked): there is
    NO stronghold. The end portal frame is craftable instead
    (1 obsidian + 1 eye of ender -> 4 frames); the player builds the
    5x5 ring anywhere, taps each frame with an eye, and the 3x3
    interior fills with the end portal tile. The eye is crafted
    shapeless from ender pearl + blaze powder (both already in the
    game since v12/v20).
  - Textures: NO uniform-magenta cells were left except 30 and 153
    (v21 used 170–173/216/217/231). Cells 14 (solid blue) and 181
    (solid black) were verified unused as tile textures by grep
    (no constructor / getTexture returns them) and were drawn over:
    30 end stone, 153 frame-with-eye, 14 chorus plant (RGBA, so the
    cross texture has a transparent background), 181 end portal
    starfield. Empty frames reuse the end-stone texture (30);
    the dragon egg reuses obsidian (37). Item icons: eye (0,11),
    chorus fruit (1,11), both blank black cells before.
  - Void: `Entity::baseTick` calls `outOfWorld()` below y=-64 and
    that just `remove()`s the entity — for the player that would
    delete the player object with no death screen. The End needs
    its own void-damage path (see below).
  - A v21 latent bug found by this audit (FIXED here, not just
    noted): `NetherWartTile::getTexture` returned `tex + data`
    (173, 174, 175, 176), but wart stages 1–3 were drawn into
    cells 216/217/231 — cells 174–176 hold unrelated textures (174
    is half magenta placeholder). Stages now map through a table
    {173, 216, 217, 231}, the mapping the v21 comment intended.
  ### What shipped
  - **End dimension, real:** `EndDimension` (id 1, `foggy`, no
    ceiling, no day cycle — `getTimeOfDay` fixed 0.75, near-black
    purple fog) registered in `Dimension::getNew`, with
    `EndLevelSource` as its chunk source. The world keeps THREE
    `Level` objects alive in `Minecraft` (`overworldLevel` /
    `netherLevel` / `endLevel`); the End storage lives at
    `<world>/DIM1`, pre-seeded `level.dat` (same seed/gameType,
    spawn (0,67,0)), player tag omitted like the Nether.
  - **End terrain** (`EndLevelSource`): one floating end-stone
    island centred (0,0), radius ~48, top y 62–66, ~14 thick, void
    outside and below; 10 obsidian pillars (3x3, ring radius
    20–32, heights 9–24 above y=64, bedrock caps); spawn platform
    at the centre (5x5 obsidian y=64, 8-block bedrock ring y=65,
    centre (0,65,0) air = the exit portal, filled on dragon death);
    sparse chorus plants on the island top. Endermen are the only
    spawner mob in the End.
  - **End portal (overworld side):** `EndPortalFrameTile` (id 120,
    data bit 0 = eye inserted; empty = end-stone look, eye = drawn
    frame texture) + `EyeOfEnderItem` (id 381; tap a frame to set
    its eye, consumes 1 eye; when all 12 frames of a 5x5 ring have
    eyes the 3x3 interior fills with `EndPortalTile` id 119).
    Standing in an end portal tile switches dimension (enter-only,
    5 s cooldown, same deferred pattern as the Nether portal):
    overworld -> End (arrival always the island centre, no 1:8
    scale), End exit portal -> overworld return position. Death in
    the End respawns in the overworld (modern MC): `respawnPlayer`
    switches dimension first, the respawn itself runs in
    `_levelGenerated` once the overworld is live.
  - **Ender dragon (boss):** `EnderDragon` (`MobTypes` 41, 200 HP,
    6 melee, fire-immune, 4x2) — Monster old-AI like the ghast,
    flying a steady orbit (radius 22, y=82, clears every pillar
    top inside that radius), fireballs up to 40 blocks every 3 s
    + melee fallback, `DragonModel` (body/head/2 wings/tail, wing
    flap) + `DragonRenderer` + drawn `mob/dragon.png`. Spawned
    exactly once by `Minecraft::ensureEndDragon` on first End
    entry (never by the spawner, never again once the exit portal
    exists). Death activates the exit portal (centre becomes the
    end portal tile) + places the dragon egg on the platform rim +
    drops 120 XP (modern MC gives 12,000, but
    `Mob::dropExperience` splits into 1–3 XP orbs, so 12,000 would
    spawn ~6,000 orb entities). No perch/landing AI (none exists
    in this tree — the dragon circles until shot down).
  - **Chorus:** `ChorusPlantTile` (id 240, 5 stages on end stone,
    grows in the dark like nether wart, one cross texture for all
    stages) + `ChorusFruitItem` (id 432, +4 hunger, plants on end
    stone, random ±8-block teleport on finishing eating — up to 8
    tries for 2 air above solid ground). Mature plant drops 1–2
    fruit; immature drops nothing.
  - **Crash safety (pays the v21 debt, for the Nether too):**
    `Minecraft::persistPlayerToOverworld()` writes the live player
    (pos/inventory/XP) into the overworld tag at the return
    position — on EVERY dimension entry, every 60 s while in the
    Nether/End, and on quit (`leaveGame` reuses it; the v21
    Nether-only inline block is replaced). A crash in the End or
    Nether now loses at most a minute of dimension loot instead of
    everything since entry. `requestDimensionChange` /
    `changeDimension` are generalised from Nether-only to any
    dimension (End returns land exactly on the stored return
    position — no Nether portal is built next to the end portal
    frame).
  - **Void death:** `LocalPlayer::tick` hurts the player for 1000
    below y=-32 in the End (normal death flow: death screen,
    respawn in the overworld). In creative `hurt()` refuses, so the
    player is bounced back to the spawn platform instead of falling
    to y=-64 and being `remove()`d.
  - Small: end stone / frame / dragon egg / eye / chorus fruit in
    the creative inventory; eye + frame recipes; lang strings for
    end portal / frame / chorus plant / chorus fruit (end stone and
    dragon egg names already existed).
  - Version bump 21 -> 22 (`app/build.gradle`,
    `AndroidManifest.xml`, `build-apk.sh`). Local `ndk-build`
    (r17c, both ABIs) passes: (see the v22 report).
  ### Known limits / debts
  - No stronghold (by decision, see audit — the frame is
    craftable instead). No eye throwing / stronghold locating.
  - No dragon perch/landing, no end crystals, no dragon respawn
    (the dragon spawns once per world; after its death the exit
    portal stays and no new dragon comes). A dragon killed by a
    fall into the void or by another mob's fireball... the dragon
    only dies to player damage in practice; if it dies without a
    player killing blow it drops no XP (v8 rule) but the exit
    portal + egg still activate (dropDeathLoot always runs).
  - Dragon XP is 120, not modern MC's 12,000 (orb-splitting limit,
    see above). Dragon/chorus/enderman sounds are placeholders or
    silent (no audio files exist; v19 left the enderman silent
    rather than faking it — same call here).
  - Breaking an end portal frame leaves a floating end portal
    behind (no collapse logic — the exit portal's bedrock ring
    makes a shared frame check wrong for it). The chorus plant
    uses one texture for all 5 stages, grows flat like a crop
    (modern MC grows it upward as a stalk with a chorus flower —
    no vertical growth, no chorus flower in this batch).
  - A dragon mid-fight when the player quits respawns at full HP
    on re-entry (the live level keeps it only within a session;
    defeat is what persists, via the exit portal tile). The
    dragon may clip a tall pillar if it is knocked off its orbit
    (its orbit clears the pillars; knockback is the only way in).
  - Multiplayer dimension switching is NOT implemented
    (single-player only, same as v21).
  - All of it is build-only until a live test on device (craft 12
    frames + 12 eyes, build the ring, fill it, enter, check island
    + pillars + void + endermen, fight the dragon, exit portal back
    to the overworld, quit while in the End and reload — inventory
    + overworld world intact, no crash).

## Next (v22)
- **Device verification for the infinite-world claim (v18's open
  item, before any version calls the world "infinite").** 10-minute
  walk session on the realme 8, memory flat, FPS within ~10% of v14
  at spawn, no void holes, no crash; walk/teleport past x=512 and
  z=512 and into negative coords — terrain continues, no vertical
  edge, no blue void; edit blocks beyond the old border, quit,
  reload, edits persist; OLD 512 world loads and its `chunks.dat`
  migrates to `r.0.0.dat` (check the world dir); chest with items
  placed far from spawn, walk away >16 chunks, walk back, contents
  intact, no duplicate chest; mobs spawn at night far from origin
  (the old 0..15 bug is gone); a mob left behind >16 chunks despawns
  instead of freezing forever (v18 net). The v17 APK stays the
  fallback until this stage passes.
  Also queued: the v18 debt — save `entities.dat` once per
  eviction sweep instead of once per evicted chunk. (PAID in v20.)
- **v23 — Villages + trading** (next version after the device
  verification above). Village generation + villagers + trading
  UI. Big; last among gameplay features.
- **Device verification for the End (v22's open item).** See the
  v22 test list above (portal ring, island, dragon fight, exit
  portal, quit-in-End reload). The v21 APK stays the fallback
  until this stage passes.
- **v22 — The End.** SHIPPED (see "Done").

## Queued (priority order — re-evaluate after each version)

- **Witch — AFTER brewing.** Re-prioritised 2026-11-08: a
  witch without potions (brewing does not exist yet) would just be
  another melee mob, so its value is small; graphics polish (v13)
  is visible in every screenshot the user takes and is low risk, so
  it shipped first. The witch fits the AI framework (GoalSelector
  exists) and ships once brewing gives it potions to throw.
- **v22 — The End.** SHIPPED (see "Done").
- **v23 — Villages + trading.** Village generation + villagers + trading
  UI. Big; last among gameplay features.
- **Night spawning live test (from old v8 scope).** Drops are done (v7);
  code chain verified intact in v8, but hostile mobs spawning at night
  still needs a live night test on device. Optional: spider eye drop
  (its `dropDeathLoot` is commented out).

## Skipped / blocked

- **Boats (v7 scope, investigated 2026-11-08, NOT feasible in one batch).**
  Only residue exists in the tree: commented-out `Item::boat` declaration
  + registration (`Item.cpp`, id 77, `BoatItem` class) and a commented-out
  crafting recipe (`Recipes.cpp`). There is NO `Boat` entity class, NO
  `BoatItem` class, NO vehicle directory, NO boat model/renderer
  (`EntityRendererId` has no boat entry), and NO riding system at all in
  `Entity` — only a vestigial `Model::riding` bool that `MobRenderer`
  hardcodes to `false` (`//mob.isRiding()`). The minecart is commented
  out the same way. Shipping a usable boat would mean writing a vehicle
  system from scratch (entity + water physics + model/renderer + riding
  + player controls + item + recipe) — a multi-version effort, not a
  v-batch. v7 shipped the v8 fallback (hostile mob drops) instead.
- **Minecarts + rails (v12 scope, investigated 2026-11-08, NOT feasible
  in one batch).** The residue is even thinner than the boat's: only
  commented-out `Item::minecart` / `minecart_chest` / `minecart_furnace`
  declarations + registration (`Item.cpp`, ids 72/86/87, `MinecartItem`
  class) and commented-out crafting recipes (`Recipes.cpp`, incl. rail
  x16 / golden rail / detector rail), a `SHAPE_RAIL` constant + a
  commented-out `tesselateRailInWorld` call (`TileRenderer.cpp`), and
  `Tile::rail` declared in `Tile.h` but never defined. There is NO
  `Minecart` entity class, NO `MinecartItem` class, NO `RailTile` /
  `DetectorRail` / `GoldenRail` class, NO rail tile registration, and
  NO riding system at all in `Entity` — only a vestigial `Model::riding`
  bool that `MobRenderer` hardcodes to `false` (`//mob.isRiding()`).
  Shipping a usable minecart would mean writing a vehicle system from
  scratch (rail tile + shape/connection logic + minecart entity +
  rail physics + model/renderer + riding + player controls + item +
  recipe) — a multi-version effort, not a v-batch. v12 shipped the
  slime + enderman fallback instead.
- (none yet — fill in as versions prove things out)
- **Infinite world (pre-v15 note, SUPERSEDED by the v15 investigation
  above).** Was listed as "not feasible: fixed `CHUNK_CACHE_WIDTH`
  chunk cache; max sane size is 512×512 today (1024 pending
  measurement)". v15 measured 1024 (4,096 chunks ≈ 322 MiB + 399,384
  VBO buffers — still not viable, rejection stands) AND proved
  infinite itself IS feasible via streaming: terrain gen, biomes,
  renderer grid and fog are already coordinate-free; the blockers
  are the fixed 32×32 `RegionFile` offset table and ~10 clamp sites.
  Stages v16–v18 in "Next" are the plan; nothing here blocks them.
