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

- **v23 — Villages + trading (SHIPPED, build-only).**
  ### Audit (read from code before writing anything)
  - NO village/villager/trading existed: zero hits for "villag" in the
    whole tree. The only residue was `TownFeature.h` — an uncompilable
    Java-style stub (`extends LargeFeature`, `byte[]`, empty body,
    included nowhere) — and `DungeonFeature.h`, entirely `#if 0`'d
    out. No structure generation beyond dungeons (disabled) + trees /
    ores / flowers in `RandomLevelSource::postProcess`, so the village
    generator was written from scratch on the `Feature::place`
    pattern (per-chunk, deterministic, clipped to the chunk).
  - NO real emerald existed: `Item::emerald` (id 264) IS the diamond
    gem in this tree's old naming (`item.emerald.name=Diamond`, dropped
    by the diamond ore tile `Tile::emeraldOre`, smelted by the furnace
    into the same gem, used for diamond tools + the v10 enchanting
    recipe). A new item was needed for the currency — see below. The
    ore drop was deliberately NOT changed (re-splitting diamond vs
    emerald would break diamond tools + enchanting).
  - Everything else needed already existed: passive-animal AI
    (`PathfinderMob::updateAi` strolls on its own — pigs/cows set no
    goals, so a villager gets wandering for free), tap-to-interact
    (`GameMode::interact` -> `Player::interact` -> `Entity::interact`,
    the Cow/Pig pattern), the EnchantScreen pattern (a screen with
    buttons, no container — the trading UI copies it), all house
    materials (cobblestone `Tile::stoneBrick`, planks, logs, glass,
    doors, fence, glowstone `Tile::lightGem`, farmland, wheat/carrot
    crops), `MobTypes` 42 + a renderer id free, `Item` ctor id 433
    free (full id 689 < `MAX_ITEMS` 1024), and a fully blank
    `gui/items.png` row 13 for the emerald icon.
  ### What shipped
  - **Village generation** (`VillageFeature`, new): the world is
    divided into a grid of 24x24-chunk cells; each cell holds exactly
    one village at a jittered chunk inside it (splitmix64 hash of
    world seed + grid cell — pure function of seed + chunk, so every
    chunk overlapping a village computes the SAME layout and places
    only the parts inside its own 16x16 area: no cross-chunk writes,
    no generation recursion, saved with the chunks like any other
    terrain). ~1 village per 384 blocks, plains/desert only, dry land
    (centre heightmap >= 65). Each village: a well (centre, 3x3 water
    + cobblestone rim + corner pillars), gravel paths (cross + plaza
    ring), 5 houses (cobblestone or plank walls, log corners, plank
    roof, open doorway facing the centre, glass windows, glowstone
    floor lamp; one has a workbench), 2 farm plots (wheat + carrots,
    mature, with a water channel), 4 lamp posts (fence + glowstone).
    Ground inside radius 26 is flattened to the centre height first.
    Placement runs LAST in `RandomLevelSource::postProcess`, so houses
    overwrite any trees/flowers inside the footprint.
  - **Villager** (`MobTypes` 42, `ER_VILLAGER_RENDERER`): passive
    `Animal` (10 HP, never self-despawns, strolls with the built-in
    old-AI wander), humanoid model + drawn `mob/villager.png`
    (robe + face, same precedent as the slime/enderman textures).
    No death loot, no XP on kill. Sounds are `mob.villager` —
    unregistered, so silent, same as the enderman precedent.
    Registered in `MobFactory` (save/load works via
    `EntityFactory::loadEntity`, type id 42 < 64) and in
    `EntityRenderDispatcher`.
  - **Villager persistence** (`VillageFeature::ensureVillagers`,
    hooked in `ChunkCache::getChunk` + `ChunkCache::postProcess`):
    entities are NOT chunk data (global `entities.dat`), and chunk
    eviction despawns them like every other mob — so a village's
    villagers are topped back up to 4 whenever the centre chunk
    (re-)enters the resident set (generation, disk load, or deferred
    postProcess). Overworld single-player only (`dimension == NORMAL`,
    `!isClientSide`). Spawning refuses until the chunk's terrain is
    populated, so a villager can never be buried by the flattening
    that runs later. Safe to call from `getChunk`: `Level::addEntity`
    only touches the resident chunk, and `Level::tickEntities`
    iterates by index, so the append cannot invalidate an iterator
    (same as mob spawning during chunk generation).
  - **Emerald currency** (new `Item::emeraldGem`, ctor id 433, full
    id 689): icon drawn into blank `items.png` cell (2,13),
    `item.emeraldGem.name=Emerald` in `en_US.lang`, in the creative
    inventory. Sources: trading only (see below) — mining the
    diamond ore still gives the diamond gem, unchanged.
  - **Trading UI** (`TradingScreen`, EnchantScreen pattern, opened
    by tapping a villager -> `Player::openTrading` ->
    `LocalPlayer::openTrading`): 6 fixed trades, same for every
    villager, no stock limits, no trade XP: 20 wheat -> 1 emerald,
    16 coal -> 1 emerald, 1 emerald -> 3 bread, 1 emerald -> 2 cooked
    beef, 2 emeralds -> 1 iron pickaxe, 3 emeralds -> 1 diamond
    sword. Cost is removed from the inventory, the result is added,
    and the cost is refunded if the inventory has no room. Labels
    use the items' real names; the screen shows the emerald balance.
    Multiplayer: `Player::openTrading` is a no-op on the server
    (same as enchanting) — single-player only in this batch.
  - Version bump 22 -> 23 (`app/build.gradle`,
    `AndroidManifest.xml`, `build-apk.sh`). Local `ndk-build`
    (r17c, both ABIs) passes: `libminecraftpe.so` arm64 10,146,632 B
    / armv7 8,732,548 B, new symbols confirmed
    (`mob/villager.png`, `TradingScreen`, `emeraldGem`,
    `VillageFeature`). One real bug caught by the local build:
    `Inventory::removeItem(const ItemInstance*)` hides the
    `FillingContainer::removeItem(slot, count)` overload, so the
    trade-removal call is qualified
    (`FillingContainer::removeItem`).
  ### Known limits / debts
  - No village chest with loot (skipped deliberately: placing a
    tile entity during postProcess has no precedent in this tree —
    the dungeon spawner/chest feature is `#if 0`'d out — so the
    first emerald comes from selling wheat/coal, and the village
    farms grow mature wheat/carrots for exactly that loop).
    (PAID in v33 — the precedent is the normal block-place path:
    `LevelChunk::setTile` -> `EntityTile::onPlace` ->
    `setTileEntity`; see v33.)
  - No per-villager professions or randomised offers (all villagers
    share the 6 fixed trades); no trade XP, no restocking limits.
  - Villagers do NOT go home at night (no such goal exists in this
    tree) and wander the village by random stroll — they can drift
    away from the houses over time. Killed villagers are replaced
    when the village chunk reloads (deliberate top-up, not modern
    MC behaviour). No "go home" / door AI.
  - Houses have open doorways, not door blocks (door data is fiddly
    and villagers have no door AI); flat plank roofs, not gabled;
    no church / blacksmith variants, no village paths beyond the
    cross + plaza ring.
  - Trading does not open in creative mode (same guard as the
    enchanting screen) and does not work in multiplayer.
  - Villager ambient/hurt/death sounds are silent (no audio files;
    same call as the enderman in v19).
  - All of it is build-only until a live test on device (find a
    village in plains/desert, check well/houses/paths/farms/lamps,
    villagers wandering, tap one -> trading UI, trade wheat/coal
    for emeralds and back for bread/tools, quit/reload — village
    and villagers still there, no crash).

- **v24 — Witch + splash potions + swamp hut (SHIPPED, build-only).**
  ### Audit (read from code before writing anything)
  - NO witch existed: zero hits for "witch" in the whole tree. NO
    splash potion existed either: v20 shipped drinkable potions only
    (no throwable use for the item, no splash/area effect).
  - Everything the witch needed already existed: the v20 potion
    effects on `Mob` (poison/slowness/weakness/harming timers,
    `PotionItem::applyEffect` — but it took a `Player*`, so the
    splash path needed a `Mob*` version, see below); the v21
    ghast/blaze ranged pattern (old-AI `checkHurtTarget`, one aimed
    projectile every 3 s — the skeleton's arrow pattern is the same
    shape); `Throwable` (block + entity hit detection, gravity,
    save/load hooks — `Fireball` is the precedent); the v23
    `VillageFeature` structure pattern (deterministic grid-cell
    placement + clipped per-chunk writes + `ensureVillagers` top-up
    from `ChunkCache`, reused for the hut); a swamp biome
    (`Biome::swampland` exists and is reachable from the biome map);
    the `MobTypes` 43 + `EntityTypes` 86 ids free; `random.glass`
    registered (splash sound); `Level::getEntitiesOfType` for the
    hut-witch top-up.
  - Two audit findings that shaped the design: (1) the witch's
    "keep your distance" AI cannot touch `PathfinderMob::path`
    (private) — the public `isPathFinding()` / `setPath()` pair is
    the way in (caught by the local build, not by review); (2) there
    is NO cauldron tile in this tree (v20 debt), so the hut ships
    without a cauldron.
  ### What shipped
  - **Witch** (`MobTypes` 43, `ER_WITCH_RENDERER`): `Monster`,
    0.6x1.95, 26 HP (modern MC), no melee (attackDamage 0), does
    NOT burn in daylight (modern MC), humanoid model + drawn
    `mob/witch.png` (64x32, purple robe / green skin / dark hat,
    recolored from the villager texture). Night spawn via `Biome`
    enemies (weight 1, always alone, like the enderman). Drops
    0-2 sticks + 1/3 spider eye, 1/4 sugar, 1/4 glowstone dust,
    1/8 glass bottle; 5 XP on kill. Sounds are `mob.witch` —
    unregistered, so silent, same as the villager/enderman
    precedent. Registered in `MobFactory` (save/load works via
    `EntityFactory::loadEntity`, type id 43 < 64) and in
    `EntityRenderDispatcher`.
  - **Witch AI** (`Witch::checkHurtTarget`, skeleton/ghasts
    pattern): within 16 blocks it throws one potion every 3 s and
    faces the target; at 6-16 blocks it holds ground and throws;
    closer than 6 blocks it plots an away-path (only when not
    already pathfinding) and backs off, so it keeps ~10 blocks
    like the modern witch. Potion choice: harming if the target
    has <= 8 HP, otherwise poison / slowness / weakness / harming
    at random, skipping an effect the target already has.
  - **Self-heal:** when hurt to <= 12 HP it drinks a healing
    potion (heal 4, `random.drink` sound) with a 10 s cooldown, so
    it cannot out-heal the player forever.
  - **Splash potions, real** (`ThrownPotion`, `EntityTypes::
    IdThrownPotion` 86, `ER_THROWN_POTION_RENDERER`): a `Throwable`
    with a slight arc (gravity 0.05) that SPLASHES on hitting a
    block or an entity — every mob within 3 blocks of the impact
    gets the effect via `PotionItem::applyEffect` (now taking a
    `Mob*` instead of a `Player*`; drinking is unchanged, a
    `Player*` converts implicitly). A potion landing at the
    player's feet still poisons them; the thrower is never
    splashed by its own potion. Glass-break sound + splash
    particles on impact. The potion aux is saved/loaded
    (`PotionAux` byte), and the renderer shows the bottle icon
    for the actual aux (poison looks like poison, harming like
    harming — `gui/items.png` row 12, one cell per aux).
  - **Swamp hut** (`SwampHutFeature`, new): one hut per 16x16-chunk
    grid cell at a jittered chunk (same deterministic splitmix64
    pattern as villages, different salt), swampland only, ground
    height >= 62. 7x5 planks on log stilts (floor = ground + 3,
    per-corner stilts reach the local lakebed), log corners +
    plank walls, doorway on the south wall, glass window on the
    north, plank roof with 1-block overhang, workbench inside.
    Placed LAST in `RandomLevelSource::postProcess` after villages
    (different biomes, so they never overlap). One witch lives in
    each hut (`ensureWitch`, same top-up pattern as
    `ensureVillagers`, hooked at both `ChunkCache` call sites).
  - Version bump 23 -> 24 (`app/build.gradle`,
    `AndroidManifest.xml`, `build-apk.sh`). Local `ndk-build`
    (r17c, both ABIs) passes: `libminecraftpe.so` arm64
    10,158,920 B / armv7 8,744,836 B, new symbols confirmed
    (`Witch`, `ThrownPotion`, `SwampHutFeature`, `mob/witch.png`).
    Two real bugs caught by the local build: `PathfinderMob::path`
    is private (fixed via `isPathFinding`/`setPath`), and
    `ThrownPotion.cpp` included `CompoundTag.h` at the wrong
    relative depth.
  ### Known limits / debts
  - NO cauldron in the hut (no cauldron tile exists in this tree —
    the v20 debt stands; the hut has a workbench instead).
  - No player-throwable splash potions: the witch throws them, but
    the player cannot throw a potion yet (no throwable use for
    `PotionItem` — that is the natural follow-up, see "Next").
  - The hut witch tops up to 1 when the hut chunk reloads
    (deliberate, same as villagers) and can wander away from the
    hut by day (no "stay home" goal exists in this tree).
  - Witch sounds are silent (no audio files; same call as the
    enderman in v19). The witch has no nose/wart model detail
    (humanoid model only — the hat is painted into the texture).
  - All of it is build-only until a live test on device (night:
    find a witch, check it keeps its distance and throws potions
    — poison/slowness/harming must be felt — check its drops and
    its self-heal; swamp: find a hut, check stilts/floor/roof/
    workbench and the witch inside; no crash).

- **v25 — Player splash potions + cauldron (SHIPPED, build-only).**
  Pays the v20 cauldron debt and the v24 player-throw debt.
  ### Audit (read from code before writing anything)
  - `ThrownPotion` (v24) was directly reusable: the (Level, Mob)
    `Throwable` ctor already shoots along the player's aim (the
    EggItem/SnowballItem precedent — `new ThrownEgg(level, player)`
    with no shoot call), the splash radius-3 `onHit` already
    existed, and `PotionItem::applyEffect` already took a `Mob*`.
    The ONLY witch-specific behaviour was the thrower skip
    (`ownerId`), which modern MC does NOT do for the player (a
    splash healing potion at your own feet heals you).
  - `PotionItem` (v20) needed a splash encoding: aux is saved as a
    short "Damage" tag (`ItemInstance.cpp:281`), so a flag bit fits
    with room to spare. Gunpowder existed since v7 (`Item::sulphur`,
    id 33, creeper drop 0–2) and `BrewingRecipes::getResult` /
    `isIngredient` were the only two brewing entry points (the
    tile entity + the screen preview both call them).
  - NO cauldron tile existed (v20/v24 debt, confirmed by grep —
    only a commented-out recipe + a commented-out TileEntity
    registration). NO buckets exist either (`Item::bucket_*` all
    commented out), so bottle-only in/out is the honest scope.
  - `GameMode::useItemOn` calls the tile's `use()` BEFORE the held
    item's `useOn`, so the cauldron's bottle logic lives in the
    tile, not in `GlassBottleItem` (which stays untouched).
  - Textures: NO uniform-magenta cells were left in terrain.png.
    Cell 250 (10,15) was verified unused (not a tile tex, not in
    the ItemRenderer item->tex mapper, not a destroy stage
    240..249, not a dynamic-texture target — only Water/WaterSide
    are dynamic, both on the water tex) and was ~92% transparent.
    `gui/items.png` row 14 was fully blank (row 13 col 2 is the
    v23 emerald, so row 14 avoids it).
  - Tile id 118 was FREE (modern MC cauldron id); lang
    `tile.cauldron.name=Cauldron` already existed. The cauldron
    recipe was commented out against `Item::cauldron`, which never
    existed as an item — the live form uses the tile.
  ### What shipped
  - **Splash potions, player-throwable.** A splash potion is the
    same item id 373 with aux = base type + 16 (`SPLASH_FLAG`,
    `PotionItem::isSplash` / `baseAux`). Brewing: any potion +
    gunpowder in the brewing stand -> its splash variant
    (water and awkward included — splash water bottle / splash
    awkward potion are real in modern MC; an already-splash
    potion refuses gunpowder AND every other ingredient, since no
    redstone/glowstone level system exists yet). Throwing:
    `PotionItem::use` throws a `ThrownPotion` on tap (EggItem
    pattern: count-- unless creative, `random.bow` sound, server
    side only) — splash potions are never drunk. The thrown
    potion's aux is normalised to its base type in the ctor, so
    `onHit` applies the base effect in the v24 radius-3 splash,
    and the renderer shows the right bottle. A player-thrown
    potion CAN splash its thrower (new `splashOwner=true` ctor +
    `SplashOwner` save byte; the witch keeps `false` = v24
    behaviour). All 9 effect potions (healing, swiftness,
    strength, fire resistance, regeneration, poison, weakness,
    harming, slowness) can be splashed; water/awkward splash too
    (no effect, like modern MC). Splash icons are tinted copies
    of the row-12 icons on `gui/items.png` row 14 + a droplet
    mark, and names are `item.splashPotion.<type>.name`
    ("Splash Potion of Healing", etc.), so splash vs drinkable is
    visible in the hotbar and in the name. `applyEffect` /
    `getTypeDesc` / `useTimeDepleted` all mask the flag, so no
    call site can drink or apply a wrong effect from a splash aux.
  - **Cauldron** (tile 118, modern MC id; `CauldronTile`, plain
    `Tile` — no tile entity, the water level 0..3 lives in the
    data nibble, so it survives save/load and eviction like any
    tile data). Cube + drawn iron-pot texture (terrain.png cell
    250); the top face shows the water texture (cell 205) while
    any water is held. Interactions: empty glass bottle + level
    > 0 -> take one level out (bottle becomes a water bottle);
    water bottle (drinkable or splash) + level < 3 -> pour one
    level in (bottle becomes empty); anything else / empty hand /
    full / empty -> falls through (no-op). `random.splash` sound
    on both. Crafting: 7 iron ingots (the v20 commented recipe,
    now live against the tile); TileItems are auto-created for
    registered tiles, so it is placeable + in the creative
    inventory. Breaking it drops itself (base `getResource`
    returns the id).
  - **Swamp hut now has a cauldron** (v24 debt paid): a full
    cauldron (data 3) against the east wall, opposite the
    workbench (`SwampHutFeature::placeHut` + new `setHD` helper
    for tile+data placement). Existing huts in old worlds do NOT
    gain one retroactively (generation only runs on new chunks).
  - Version bump 24 -> 25 (`app/build.gradle`,
    `AndroidManifest.xml`, `build-apk.sh`). Local `ndk-build`
    (r17c, both ABIs) passes: `libminecraftpe.so` arm64
    10,158,920 B / armv7 8,748,932 B, new symbols confirmed
    (`CauldronTile`, `ThrownPotion(Level,Mob,int,bool)` ctor,
    `Tile::cauldron`, `SplashOwner` tag string). CI
    `./gradlew assembleDebug` SUCCESS (run 42526785778, ~5 min);
    APK `minecraft-mcpe-android-0.6.1-debug-v25.apk`
    12,897,571 B, sha256 819594df…5818f18, versionCode 25,
    `apksigner verify` v1+v2 true, `unzip -t` clean; the APK
    itself was re-checked to contain the new cauldron texture
    (terrain.png cell 250), the row-14 splash icons, all 11
    `item.splashPotion.*` lang keys, and the new symbols in its
    `libminecraftpe.so`.
  ### Known limits / debts
  - No buckets (none exist in this tree), so bottles are the only
    way to fill/empty a cauldron. No dyeing / washing in the
    cauldron, no lingering potions, no redstone-extend /
    glowstone-level-II brewing (no potion-level system yet).
  - Splash potions are not in the creative inventory (only the
    drinkable aux-0 bottle shows there) — brew them in survival,
    or in creative via the brewing stand like everything else.
  - The cauldron is a full cube with one drawn texture, not the
    modern 3D pot model (same precedent as the brewing stand /
    enchanting table). Its water top is the flat water texture,
    not a level-scaled surface (the level is only visible as
    water/no-water, not 1/3 vs 2/3 vs full).
  - A splash potion thrown at a block is used like an egg/snowball
    (tap with no block hit); tapping a block with one does nothing
    (same as eggs — `use()` is not called on a block hit).
  - All of it is build-only until a live test on device (brew a
    splash potion with gunpowder, throw it at a mob — the radius-3
    effect must be felt — throw a splash healing at your own feet
    and check YOU heal, fill/empty the cauldron with bottles,
    check the swamp hut now has a full cauldron, no crash).

- **v26 — Brewing upgrades: redstone + glowstone (SHIPPED, build-only).**
  Pays the v20 caveat "belum ada redstone/glowstone upgrade" for
  potions.
  ### Audit (read from code before writing anything)
  - Potion encoding had room: aux = base type 0–10 (bits 0–3) +
    SPLASH_FLAG 16 (bit 4), saved as a short "Damage" tag
    (`ItemInstance.cpp:281`), so bits 5 (32) and 6 (64) were free
    for the two new variants — no save-format change, max aux
    10+16+32+64 = 122. `baseAux()` (& 15) already stripped all
    flags, so every existing call site kept working.
  - Durations and magnitudes were HARDCODED, with no level/duration
    stored anywhere: durations in `PotionItem::applyEffect`
    (DURATION_LONG 3600 / MEDIUM 1800 / SHORT 900), magnitudes at
    the consumers — speed x1.2 (`LocalPlayer.cpp:381`), strength +3
    (`Player.cpp:790`), regen 1 HP / 50 ticks, poison 1 HP / 25
    ticks (`Mob.cpp:288-294`). The 7 timers live on `Mob` and are
    NOT saved (only zeroed on spawn, `Mob.cpp:603-609`), so the new
    amplifier fields are session-only too — no save/load change.
  - Both ingredients already existed: redstone dust
    (`Item::redStone`, id 331, redstone ore v11) and glowstone dust
    (`Item::yellowDust`, id 348, witch drop 1/4 in v24).
  - One trap found by the audit: `ThrownPotion`'s ctor normalised
    the aux with `baseAux()`, which would have thrown the variant
    flags away the moment a splash potion was thrown — fixed to
    strip only the splash bit (see below). A second trap in the
    same area: the v25 gunpowder branch rebuilt the result as
    `baseAux + SPLASH_FLAG`, which would also have stripped an
    extended / level II variant when splashing it — now OR's the
    flag on instead.
  ### What shipped
  - **Redstone = extended.** Any timed-effect potion + redstone
    dust -> extended variant (flag 32): swiftness / strength /
    fire resistance 3:00 -> 8:00 (9600 ticks), regeneration /
    poison 0:45 -> 1:30 (1800), weakness / slowness 1:30 -> 4:00
    (4800) — the modern MC table. Instant healing / harming have
    no duration to extend, water / awkward have no effect — both
    refuse redstone.
  - **Glowstone = level II.** Any upgradable potion + glowstone
    dust -> level II variant (flag 64): healing +8 HP (was +4),
    harming 12 damage (was 6), swiftness x1.4 (was x1.2),
    strength +6 damage (was +3), regeneration 1 HP / 25 ticks
    (was / 50), poison 1 HP / 12 ticks (was / 25). Level II
    durations are SHORTER, modern MC: swiftness / strength II
    1:30 (1800), regen / poison II 0:22 (440). Fire resistance
    is binary (no level II), weakness / slowness have no brewable
    level II in modern MC, water / awkward have no effect — all
    refuse glowstone.
  - **Mutual exclusivity.** An extended potion refuses glowstone
    and a level II potion refuses redstone (both in
    `BrewingRecipes::getResult`). Fermented spider eye also
    refuses upgraded potions (no corrupt table for variants in
    this tree — it refuses rather than silently downgrading).
  - **Drinkable AND splash.** Both upgrades work on splash
    potions too (the variant flag is OR'd on, the splash bit
    survives), and an extended / level II potion can still be
    splashed with gunpowder (flag preserved). The thrown potion
    keeps its variant through `ThrownPotion` (strip-splash only)
    and applies it in the radius-3 splash via the same
    `PotionItem::applyEffect`.
  - **Names differ in UI.** `getTypeDesc` now appends a suffix
    for upgraded potions, with 26 new lang keys in `en_US.lang`:
    extended shows the duration ("Potion of Swiftness (8:00)",
    "Potion of Regeneration (1:30)", ...), level II shows "II"
    ("Potion of Swiftness II", "Potion of Healing II", ...),
    each with a "Splash Potion of ..." twin. Icons are
    intentionally the base ones — modern MC uses the same bottle
    colour for a potion and its extended / level II variants.
  - Amplifier storage: 4 new `Mob` fields (`speed / strength /
    regeneration / poisonEffectLevel`, 1 = normal, 2 = level II),
    set together with the timer in `applyEffect` and read by the
    existing consumers (`LocalPlayer::getWalkingSpeedModifier`,
    `Player::attack`, `Mob::tick` regen/poison cadence).
  - Version bump 25 -> 26 (`app/build.gradle`,
    `AndroidManifest.xml`, `build-apk.sh`). Local `ndk-build`
    (r17c, both ABIs) passes: `libminecraftpe.so` arm64
    10,158,920 B / armv7 8,748,932 B, new symbols confirmed
    (`PotionItem::applyEffect/getTypeDesc`, `BrewingRecipes::
    isIngredient`, extended/level II handling). CI
    `./gradlew assembleDebug` SUCCESS (run 42528844420, ~4m41s);
    APK `minecraft-mcpe-android-0.6.1-debug-v26.apk`
    12,898,917 B, sha256 2df5e2b3…5847a2, versionCode 26,
    `apksigner verify` v1+v2 true, `unzip -t` clean; the APK
    itself was re-checked to contain all 26 new lang keys
    (`item.potion.*Long` / `*2` + `item.splashPotion.*` twins)
    and the new symbols in its `libminecraftpe.so` (arm64
    10,163,016 B / armv7 8,748,932 B).
  ### Known limits / debts
  - No lingering potions, no buckets (v25 debts stand). Splash
    potions are still not in the creative inventory (brew them).
  - Corrupting an extended / level II potion with fermented
    spider eye is refused (modern MC carries the variant over —
    e.g. extended swiftness -> extended slowness — but this tree
    has no corrupt table for variants; a refusal is honest, a
    silent downgrade would not be).
  - A splash potion thrown at a block is still used like an
    egg/snowball (v25 caveat stands).
  - All of it is build-only until a live test on device (brew an
    effect potion + redstone -> longer duration felt in play,
    + glowstone -> stronger effect felt, both also as splash,
    redstone + glowstone on one potion refused, no crash).

- **v27 — End crystals + dragon respawn (SHIPPED, build-only).**
  Pays the v22 debt "no end crystals, no dragon respawn (the dragon
  spawns once per world)".
  ### Audit (read from code before writing anything)
  - `EnderDragon` (v22): 200 HP, orbit r22/y82, fire-immune,
    `Minecraft::ensureEndDragon` (`Minecraft.cpp:576`) spawns it
    exactly once on first End entry; the EXIT PORTAL tile at
    (0,65,0) is the "already defeated" flag — `ensureEndDragon`
    returns early once it exists, so the dragon can never come back.
    Death runs `dropDeathLoot` (always, even without a player kill):
    centre becomes the end portal tile + dragon egg at (2,65,0) +
    120 XP. No perch/landing AI exists in this tree.
  - Pillars (`EndLevelSource.cpp:77-133`): 10 obsidian 3x3 shafts,
    ring radius 20–32, tops y=73–88, each capped with a SINGLE
    bedrock block; the list (`PILLARS[10]`) was private to the
    header. Spawn platform: 5x5 obsidian y=64 + 8-block bedrock
    ring y=65, centre (0,65,0) air until the dragon dies.
  - Ingredients: ALL present. Glass (`Tile::glass`), eye of ender
    (v22, ctor 381), ghast tear (`Item::ghastTear`, ctor 114, ghast
    drops 0–1 in v21). No new mob/loot needed.
  - Entity precedent: `ExperienceOrb` (v8, `Entity` subclass, type
    id 84, save/load via `EntityFactory`, billboard-quad renderer
    with no texture) is the reusable static-entity pattern;
    `Fireball` (v21) is the explosion precedent
    (`Level::explode(NULL, x, y, z, 1.0f, true)`). `Entity::hurt`
    returns false by default and `Entity::isPickable` returns false
    by default — a crystal MUST override both, or neither punches
    (`Player::attack` -> `entity->hurt`) nor arrows
    (`Arrow.cpp:215` -> `hurt`) nor the crosshair target
    (`GameRenderer.cpp:696` skips non-pickable) can touch it.
    `Painting`/`HangingEntity` is the breakable-non-mob precedent.
  - Two traps found by the audit: (1) `Mob::heal` caps health at
    20 (`Mob.cpp:417`) — a 200 HP dragon could never be healed by
    its crystals, so the dragon needs its own `heal` override.
    (2) `Entity` has PURE VIRTUAL `addAdditonalSaveData` /
    `readAdditionalSaveData` (`Entity.h:147-148`) — a crystal with
    no extra state still needs empty overrides, or it is abstract
    and cannot be constructed. (3) The toolchain is C++98 —
    delegating constructors do not compile (found by the local
    build, fixed by duplicating the 3-line init).
  - Item ids: ctor id + 256 = full id. Modern MC end crystal is
    426, so ctor 170 (free — nothing used it). Icon cell (2,11) in
    `gui/items.png` was fully transparent (verified by pixel count:
    0 non-transparent before, 99 after drawing).
  - Explosion safety: `Explosion` respects `getExplosionResistance`
    and obsidian is `setExplodeable(2000)`, so a power-6 crystal
    blast cannot eat the pillars or the bedrock — verified from
    code, not assumed.
  ### What shipped
  - **End crystal entity** (`EndCrystal`, type id 87, renderer id
    `ER_END_CRYSTAL_RENDERER`): floats exactly where placed (its
    `tick()` never calls `baseTick`, so no gravity/fire/water —
    only the tickCount bookkeeping the renderer spins/bobs from);
    2x2, fire-immune, `isPickable`, save/load works through the
    base `Entity` save + a new `EntityFactory` case. Renderer:
    spinning crossed pink quads + near-white core (no texture, the
    XP-orb precedent), bobbing ±0.12 blocks.
  - **Crystal explodes when destroyed.** `hurt()` (punch or arrow)
    detonates once: `Level::explode(NULL, x, y, z, 6.0f, false)` —
    modern MC's power-6 crystal blast — then removes itself. The
    `exploding` flag is set BEFORE the blast, because the blast's
    own entity scan calls `hurt()` on everything in radius
    including the crystal; without the flag that recurses until
    the stack dies. No item drop (modern MC drops nothing).
  - **Dragon heal.** Every 10 ticks a crystal heals the nearest
    live dragon within 32 blocks by 1 HP (2 HP/s), via `Mob::heal`
    — which now routes to the new `EnderDragon::heal` override
    (cap 200, not `Mob`'s 20). This also heals during the FIRST
    fight if the player places crystals on the pillars early.
  - **Dragon respawn.** Every 20 ticks a crystal checks: exit
    portal present at (0,65,0) (a dragon was defeated before) AND
    no live dragon AND >= 4 crystals standing on respawn spots —
    the 10 pillar bedrock caps or the 8 exit-portal bedrock ring
    cells (`EndLevelSource::isPillarTop` / `isExitRingCell`, new
    public statics). Then it consumes those crystals, clears the
    exit portal back to air, and spawns a fresh dragon at full HP
    (same (0.5, 82, 0.5) spawn as `ensureEndDragon`). Modern MC
    wants the 4 crystals on the exit-portal rim only; this build
    ALSO accepts pillar tops, per the v27 brief — the rim alone
    would strand the crystals a first fight leaves on pillars.
    Placing 4 crystals during the FIRST fight does nothing (no
    portal yet), so no premature second dragon; `ensureEndDragon`
    still refuses a second dragon while one is alive.
  - **Second death.** The respawned dragon's death runs the same
    `dropDeathLoot`: exit portal reactivates + the egg is placed
    on the rim again (same (2,65,0) spot, so still at most one egg
    unless the player took the first — then a second egg appears,
    which is generous but harmless) + 120 XP again. The respawn
    loop is repeatable: after every death, 4 fresh crystals
    resummon the dragon.
  - **End crystal item** (ctor 170, full id 426 = modern MC):
    shaped recipe 7 glass + eye of ender + ghast tear
    (`"GGG" / "GEG" / "GTG"`), in the creative inventory, icon
    drawn into `gui/items.png` cell (2,11), lang
    `item.endCrystal.name=End Crystal`. Placement (useOn, End-only,
    top face only, on bedrock or obsidian, 2 air above, one crystal
    per spot within 1 block) — modern MC also allows obsidian, so
    the platform obsidian accepts crystals too; only pillar caps
    and the exit ring count toward respawn.
  - Version bump 26 -> 27 (`app/build.gradle`,
    `AndroidManifest.xml`, `build-apk.sh`). Local `ndk-build`
    (r17c, both ABIs) passes: `libminecraftpe.so` arm64
    10,187,592 B / armv7 8,753,028 B, new symbols confirmed
    (`EndCrystal::tick/hurt/tryRespawnDragon/tryHealDragon/
    isOnRespawnSpot`, `EnderDragon::heal`,
    `EndLevelSource::isPillarTop/isExitRingCell`,
    `EndCrystalItem::useOn`, `EndCrystalRenderer::render`). CI
    `./gradlew assembleDebug` SUCCESS (run 42530904633, ~5m40s);
    APK `minecraft-mcpe-android-0.6.1-debug-v27.apk`
    12,907,575 B, sha256 70d866a2…5e2e9, versionCode 27,
    `apksigner verify` v1+v2 true, `unzip -t` clean; the APK
    itself was re-checked to contain the new end-crystal icon
    (items.png cell (2,11), 99 non-transparent px), the
    `item.endCrystal.name` lang key, and the new symbols in its
    `libminecraftpe.so` (arm64 10,187,592 B — byte-identical size
    to the local build).
  ### Known limits / debts
  - No dragon perch/landing (v22 debt stands — the dragon still
    circles at y=82 through every fight, respawned or not). No
    heal BEAM visual (the heal is a silent +1 HP/10 ticks; modern
    MC draws a beam from crystal to dragon — no beam renderer
    exists in this tree). No initial crystals on the pillars at
    world gen (modern MC spawns one per pillar; here the first
    fight has no crystals to shoot down unless the player places
    them — the pillars are unchanged from v22).
  - Crystals persist across quit like every other entity: the
    global `entities.dat` snapshot (`saveEntities` writes ALL of
    `level->entities`, and `EntityFactory` rebuilds a crystal from
    its type id 87 on load). The only loss case is chunk-evict
    despawn (v17 rule) — but the End island is only ~6 chunks
    wide, so the player cannot get 16 chunks away from a placed
    crystal while staying in the End, and the End level stops
    ticking entirely once the player leaves the dimension.
  - The explosion destroys nearby non-resistant blocks (modern MC
    does the same) — a crystal detonated on the exit ring can
    break the dragon egg sitting next to it if the egg is still
    there; obsidian/bedrock are safe.
  - All of it is build-only until a live test on device (kill the
    dragon once, craft 4 end crystals, place them on 4 pillar tops
    or the exit ring, check the dragon respawns at full HP and
    fights again, shoot a crystal and check the power-6 blast,
    check the respawned dragon heals near a surviving crystal,
    check the exit portal + egg are right after the second death,
    no crash, old features intact).

- **v28 — Dragon perch + initial end crystals (SHIPPED, build-only).**
  Pays the v22 debt "no dragon perch/landing" and the v27 debt
  "no initial crystals on the pillars at world gen".
  ### Audit (read from code before writing anything)
  - `EnderDragon` (v22, `EnderDragon.h/.cpp`): NO state machine, NO
    phases — `tick()` steers toward a moving orbit point every tick
    (radius 22, y=82, angle = tickCount * 0.012 ≈ 0.7°/tick, a full
    orbit in ~30 s; ease 0.06, speed capped 0.35/tick, velocity
    smoothed 0.2). `travel()` is Ghast-style: NO gravity in air, so
    the dragon holds whatever altitude it is steered to. It faces
    its travel direction. That is the whole movement system — and
    it is exactly what a perch needs: steering to a FIXED point
    instead of a moving one makes the dragon descend, sit, and take
    off again with no new physics.
  - Attack (`PathfinderMob::updateAi` -> `checkHurtTarget`, reached
    through the virtual `updateAi()` inside `Mob::tick`, which the
    dragon calls directly to dodge `Monster::tick`'s peaceful
    despawn): target acquisition is `Monster::findAttackTarget` —
    nearest player within **16 blocks**. At orbit y=82 vs a player
    at y~66 the dragon usually has NO target at all, so its
    fireballs (d<40, one per 3 s) rarely fire in practice — a
    pre-existing v22 limitation, NOT fixed here. Perch fixes it
    incidentally: on the portal the player is within a few blocks,
    so the dragon finally acquires a target and its melee (d<4,
    6 damage via `Monster::checkHurtTarget`) engages. Melee reach
    for the PLAYER: survival entity pick ray is capped at 3 blocks
    (`GameRenderer::pick`), `Mob::isPickable` is true, and the
    perched dragon (4 wide, 2 tall, feet y=66) sits right on the
    ring the player stands on — sword hits land.
  - Exit portal geometry (`EndLevelSource`): 5x5 obsidian platform
    y=64, 8-cell bedrock ring y=65 (top surface y=66), centre
    (0,65,0) air until the dragon dies. The perch point (0.5, 66,
    0.5) puts the dragon's feet exactly on the ring top. The
    descend/ascend diagonal (orbit r22/y82 <-> centre/y66) clears
    every pillar top by 3+ blocks (closest tops near the path are
    y=77 at r=20; the path is at y~80.5 there), and the ring is the
    only thing below the perch point — so the dragon cannot get
    stuck on a pillar or fall to the void. Knockback
    (`Mob::hurt` -> `knockback`, 0.4 + yd +0.4) can shove it off
    the ring, but the perch steering pulls it straight back.
  - No "sit still at a fixed point" pattern existed in any mob —
    the closest reusable pieces are the orbit steering itself and
    the crystal's static float. The perch reuses the steering, so
    no new movement code was needed.
  - Crystal trap found by the audit (would have been a real
    regression): v27 `tryRespawnDragon` counts pillar-top crystals
    toward the 4 needed for a respawn. If initial crystals simply
    spawned on the pillars, 4+ survivors would resummon the dragon
    ~1 s after the FIRST death and clear the exit portal before the
    player can use it. The fix is a flag, not a rule change (below).
  ### What shipped
  - **Dragon perch** (`EnderDragon`, state machine ORBIT ->
    DESCEND -> PERCHED -> ASCEND -> ORBIT): orbits ~20 s (400
    ticks), descends to (0.5, 66, 0.5), sits ~8 s (160 ticks),
    takes off back to the orbit point. All steering is the old
    orbit ease via a factored-out `steerTo()` — same cap, same
    smoothing. Safety, all in code: every non-orbit state has a
    timeout that forces takeoff (descend 300 ticks, perch is
    fixed-length, ascend 300 ticks — it can never wedge in a
    perch state); a y<50 watchdog abandons the perch and climbs
    to orbit; the steer targets are fixed safe points, so the
    dragon is never steered toward the void. While landing or
    perched it does NOT shoot fireballs (modern MC does not
    either, and a point-blank fireball would hit the ring); its
    melee bite still works up close, and `PathfinderMob`'s
    `lookAt()` faces the player while perched. Perch state is
    transient (not saved): a reloaded or respawned dragon starts
    in orbit. No animation change — the dragon sits with its
    normal wing-flap model (a perched pose would need a new model
    state; the sit itself is the behaviour, honestly noted).
  - **Initial end crystals** (`Minecraft::ensureEndDragon`): one
    crystal on each of the 10 pillar tops when the FIRST dragon
    spawns (new public `EndLevelSource::getPillarTop` accessor).
    Only on the very first spawn — re-entry mid-fight (dragon
    alive) and post-death (portal exists) both return before it,
    so they are never re-added and a destroyed crystal stays
    destroyed. Worlds whose first fight already started in
    v22–v27 get no retroactive crystals (deliberate, not a bug).
  - **Initial-crystal respawn exclusion** (`EndCrystal::initial`,
    saved as the "Initial" boolean; old v27 crystals load as
    non-initial): initial crystals heal the dragon and explode
    exactly like player-placed ones, but never count toward the
    v27 respawn — so the first death leaves the exit portal
    standing. Player-placed crystals still count as in v27, and
    the respawned dragon perches too (same class, same state
    machine).
  - Version bump 27 -> 28 (`app/build.gradle`,
    `AndroidManifest.xml`, `build-apk.sh`). Local `ndk-build`
    (r17c, both ABIs) passes: `libminecraftpe.so` arm64
    10,191,688 B / armv7 8,757,124 B, new symbols confirmed
    (`EnderDragon::steerTo`, `EndLevelSource::getPillarTop`,
    "Initial" tag string). CI `./gradlew assembleDebug` SUCCESS
    (run 42532281341, ~4m42s); APK
    `minecraft-mcpe-android-0.6.1-debug-v28.apk`
    12,909,822 B, sha256 978a2206…20a3aa, versionCode 28,
    `apksigner verify` v1+v2 true, `unzip -t` clean; the APK
    itself was re-checked to contain the new symbols in its
    `libminecraftpe.so` (arm64 10,191,688 B — byte-identical size
    to the local build) and the "Initial" tag string.
  ### Known limits / debts
  - No heal BEAM visual (v27 debt stands — the heal is still a
    silent +1 HP/10 ticks; no beam renderer exists in this tree).
  - No perched-pose animation: the dragon sits with its normal
    flying model/wing flap, not a folded-wing landing pose.
  - The dragon still rarely acquires a target while orbiting
    (16-block acquisition vs the y=82 orbit — the v22 limitation
    above); it only fights properly once it perches or the player
    climbs a pillar. Widening the acquisition range was out of
    scope for this batch. (PAID in v30: dragon-only 48-block
    acquisition — see v30.)
  - `PathfinderMob::updateAi` can set `jumping` while the dragon
    follows a path; `Mob::tick` would then call `jumpFromGround`
    (yd 0.42) if the dragon is onGround on the ring. In practice
    the path target is the player at the same height (yd = 0) and
    a perched dragon has ~0 horizontal velocity, so no jump is
    expected — but a small hop while perched is possible if a
    path ever targets above it; steering re-lands it immediately.
  - All of it is build-only until a live test on device (enter
    the End, check 10 crystals on the pillar tops, watch a full
    cycle orbit -> descend -> sit ~8 s -> takeoff, hit the
    dragon with a sword while perched and check damage lands,
    check it never wedges on a perch or falls to the void, kill
    it and check the exit portal STAYS (no instant auto-respawn
    from the surviving pillar crystals), then place 4 fresh
    crystals and check the v27 respawn still works and the new
    dragon perches too, no crash, old features intact).

- **v29 — Heal-beam visual + perch-hop fix (SHIPPED, build-only).**
  Pays the v27 debt "no heal BEAM visual" and the v28 cosmetic
  risk "the dragon can hop while perched".
  ### Audit (read from code before writing anything)
  - Heal today: `EndCrystal::tick` (server-side only — it returns
    early when `level->isClientSide`) calls `tryHealDragon()` every
    10 ticks: the nearest live dragon within 32 blocks, +1 HP via
    `Mob::heal` → `EnderDragon::heal` (cap 200, not `Mob`'s 20).
    There is NO "currently healing" state anywhere — the heal is a
    silent, stateless +1/10 ticks, and the nearest-dragon scan is
    the only definition of "healing". The crystal knows which
    dragon it heals only for the instant of that scan.
  - Rendering: `LevelRenderer::renderEntities`
    (`LevelRenderer.cpp:924`) → `EntityRenderDispatcher::render`
    interpolates the entity position and subtracts the camera
    offsets (`xOff`/`yOff`/`zOff`, `LevelRenderer.cpp:939-941`),
    then `EndCrystalRenderer::render` draws the spinning crossed
    pink quads + near-white core with plain Tesselator colours
    (the v8 XP-orb precedent). NO line/beam renderer exists in
    this tree. Nearest precedents: the selection box
    (`LevelRenderer.cpp:527`, `t.begin(GL_LINES)`); translucency
    (`TntRenderer`, enable `GL_BLEND`, draw, disable it again);
    camera-facing quads (`ExperienceOrbRenderer`, rotate by
    `playerRotY`/`playerRotX`). The beam belongs in the CRYSTAL
    renderer, not the dragon renderer or `LevelRenderer`: the
    crystal is the per-beam owner (up to 10 beams, one per healing
    crystal), its render call already runs exactly when the crystal
    is visible, and a removed crystal stops being rendered — so
    the beam dies with the crystal for free.
  - Client-side trap (found by the audit, shaped the design): the
    client crystal never runs `tryHealDragon`, so no server heal
    state exists for the renderer to read — and syncing one would
    need a new network field. The renderer instead RE-DERIVES the
    same in-range condition from `crystal->level->entities` (the
    same `Level` object in single-player; in MP the client level
    carries the same entity list), so the beam needs no sync and
    works in both. A 1-frame beam on the exact heal tick was
    rejected: the heal fires once per 10 ticks, a single-frame
    flash would be invisible — the beam is continuous while the
    in-range condition holds.
  - Perch hop (v28 risk, re-checked): `PathfinderMob::updateAi`
    sets `jumping = true` when the next path node is above the
    feet (`yd > 0`) or on horizontal collision, and `Mob::tick`
    then calls `jumpFromGround()` (yd 0.42) if the dragon is
    `onGround` on the ring. The dragon uses the OLD AI
    (`Mob::useNewAi()` returns false by default; only
    Zombie/PigZombie override it), and `updateAi()` is virtual
    (`Mob.h:146`), so a dragon-only override is possible with
    zero effect on any other mob — and the clear lands inside
    `updateAi()`, which `Mob::tick` calls BEFORE its jumping
    check in the same tick, so it takes effect immediately.
  ### What shipped
  - **Heal beam** (`EndCrystalRenderer`, new static
    `findHealedDragon()` + beam drawing in `render()`): when a
    live dragon (`health > 0`, not removed) is within 32 blocks,
    a thin ribbon runs from the crystal centre (feet y + 1 +
    bob) to the dragon's body centre (feet y + 1 — the dragon is
    2 tall, `setSize(4, 2)`), interpolated with `a` exactly like
    the dispatcher does for the crystal itself. No new render
    pipeline: two crossed quads along the beam axis (the crystal's
    own crossed-quad precedent, stretched into a line) — outer
    ribbon pink (1.0, 0.45, 0.85, the crystal's hue), half-width
    0.16, alpha 0.55; core ribbon near-white (1.0, 0.9, 1.0),
    half-width 0.06, alpha 0.85, crossed against the outer one so
    the beam reads from every angle even with face culling on.
    Translucency via `GL_BLEND` (`SRC_ALPHA`,
    `ONE_MINUS_SRC_ALPHA` — the `TntRenderer` enable/draw/disable
    pattern, so no other renderer inherits the state); vertex
    alpha reaches the shader (Tesselator packs 4-component
    colours, `Tesselator.cpp:229`). Depth testing stays ON, so
    pillars and the island occlude the beam like any other
    geometry. Gentle alpha pulse (0.75 + 0.25·sin(tick·0.3)) so
    the beam reads as an active heal, not a static decoration.
    Guards: beam skipped when the beam is degenerate (< 0.5
    blocks, dragon on top of the crystal); the side vector
    (axis × up) falls back to the X axis when the beam is
    near-vertical. The beam vanishes when the dragon dies
    (`health <= 0`), flies out of range, or the crystal is
    destroyed — no explicit cleanup needed.
  - **Perch-hop fix** (`EnderDragon::updateAi()` override, new):
    calls `super::updateAi()` (resolves to
    `PathfinderMob::updateAi` — `Monster` does not override it),
    then clears `jumping` while `perchState == PERCH_PERCHED`.
    Dragon-only, perch-only: orbit/descent/ascent are untouched
    (the dragon is airborne there, so `jumpFromGround` could not
    fire anyway), and the dragon never needs to jump — it flies
    by steering (Ghast-style travel, no gravity).
  - Version bump 28 -> 29 (`app/build.gradle`,
    `AndroidManifest.xml`, `build-apk.sh`). Local `ndk-build`
    (r17c, both ABIs) passes: `libminecraftpe.so` arm64
    10,183,496 B / armv7 8,757,124 B. The `.so` is stripped, so
    symbols were confirmed in the fresh object files instead:
    `EnderDragon::updateAi()` defined (T) in both ABIs, calling
    `PathfinderMob::updateAi()` (U); `EndCrystalRenderer.o`
    references `EntityRenderDispatcher::xOff`, `glBlendFunc` and
    `Mth::sqrt` — all three only exist in the new beam code.
    CI `./gradlew assembleDebug` SUCCESS (run 42533524632, ~3m20s);
    APK `minecraft-mcpe-android-0.6.1-debug-v29.apk`
    12,910,194 B, sha256 b77f6aea…96f7d7a3cb, versionCode 29,
    `apksigner verify` v1+v2 true, `unzip -t` clean; the APK
    itself was re-checked to contain the new symbols in its
    `libminecraftpe.so` (arm64 10,183,496 B / armv7 8,757,124 B —
    byte-identical sizes to the local build).
  ### Known limits / debts
  - The beam is continuous while the dragon is in range, NOT
    one flash per heal tick (a single-frame beam would be
    invisible — see the audit). At full HP `heal()` is a no-op
    but the beam still shows: the link is live, and this matches
    `tryHealDragon`, which heals unconditionally in range.
  - The beam is a flat ribbon pair, not modern MC's textured,
    scrolling beam; constant width (no taper with distance), no
    impact particles where it meets the dragon.
  - A crystal whose bounding box is frustum-culled draws no beam
    even if the dragon is on screen — the beam is owned by the
    crystal's render call (same culling as the crystal itself).
  - All of it is build-only until a live test on device (enter
    the End, let the dragon fly near a crystal and check the
    pink/white beam is visible crystal -> dragon, check it
    vanishes when the dragon flies away / dies or the crystal is
    destroyed, check the perched dragon never hops, no render
    glitch or crash, old features intact).

- **v30 — Dragon fireball targeting (SHIPPED, build-only).**
  Pays the v22/v28 debt "the dragon rarely acquires a target
  while orbiting" — the last named End boss-fight gap.
  ### Audit (read from code before writing anything)
  - Acquisition: `Monster::findAttackTarget` (`Monster.cpp:57-63`)
    is `level->getNearestPlayer(this, 16)` + `canSee(player)` —
    16 blocks, 3D (`Level::getNearestPlayer` uses
    `distanceToSqr`). The dragon orbits at r=22, y=82 while the
    player stands on the island at y~66, so the typical 3D
    distance is sqrt(22^2 + 16^2) ~= 27 blocks — outside 16 —
    and the dragon almost never had a target while orbiting.
    (At the island edge, r~40-48, the nearest orbit point is
    ~18-26 blocks away horizontally, d ~= 24-30 — still > 16.)
  - The fireball code itself was ALREADY orbit-capable:
    `EnderDragon::checkHurtTarget` fires one aimed fireball
    every 3 s for d<40 (`attackTime = 60`, `fb->shoot(xd, yd,
    zd, 1.0f, 8)` — the v21 ghast pattern), blocked only while
    DESCEND/PERCHED (deliberate, v28: no point-blank shots on
    the portal ring). `Fireball` explodes radius 1 + fire
    (`Fireball.cpp:13-16`). `attackTime` ticks down in
    `Mob::tick` (`Mob.cpp:186`). So the ONLY blocker was
    acquisition — the fire logic needed no change at all.
  - Retention: `PathfinderMob::updateAi` calls
    `findAttackTarget` only when it has no target yet; once
    acquired it calls `checkHurtTarget` every tick while
    `canSee` holds, dropping the target only if unseen AND
    d>32 (`checkCantSeeTarget`). So widening acquisition alone
    is sufficient — no per-tick re-scan, no new state.
  - `canSee` (`Mob.cpp:105`) is a `Level::clip` from the
    dragon's head (y+1.7) to the player's head — a pillar can
    block it on some orbit angles, but the ~30 s orbit cycles
    the line of sight open regularly, so no workaround was
    needed. Ranged AI at long range already works in this tree
    (ghast 28 blocks, blaze 16, witch 16 — all the same
    `checkHurtTarget` shape), so no new ranged pattern was
    invented.
  ### What shipped
  - **Dragon-only wide acquisition** (`EnderDragon::
    findAttackTarget`, new override; `Monster::
    findAttackTarget` untouched, so no other mob changes):
    nearest player within **48 blocks** + `canSee`. 48 covers
    any player on the island (radius ~48) as the orbit cycles
    past them, while the existing 40-block fire gate in
    `checkHurtTarget` still decides actual shots — acquisition
    alone never makes the dragon shoot from across the void.
    Cooldown (3 s), speed (1.0), inaccuracy (8) and explosion
    (radius 1 + fire) are the existing ghast values, unchanged:
    the shots are dodgeable and periodic, not spam — the fight
    stays winnable (and the perch phase is still the sword
    phase, v28). No fireballs while landing/perched (v28 rule
    kept); melee bite up close unchanged.
  - Version bump 29 -> 30 (`app/build.gradle`,
    `AndroidManifest.xml`, `build-apk.sh`). Local `ndk-build`
    (r17c, both ABIs) passes: `libminecraftpe.so` arm64
    10,175,304 B / armv7 8,757,124 B, new symbol confirmed
    (`EnderDragon::findAttackTarget()` defined (T) in both
    ABIs). CI `./gradlew assembleDebug` SUCCESS (run
    42534573320, ~5m40s); APK
    `minecraft-mcpe-android-0.6.1-debug-v30.apk`
    12,910,000 B, sha256
    df4fe483775975a33599a00d8b0efaa80cb1b3ac8278f42a380d41c59a4056eb,
    versionCode 30, `apksigner verify` v1+v2 true, `unzip -t`
    clean; the APK itself was re-checked to contain the new
    symbol in its `libminecraftpe.so`
    (`EnderDragon::findAttackTarget()` defined (T) in both
    ABIs; arm64 10,175,304 B / armv7 8,757,124 B — byte-identical
    sizes to the local build).
  ### Known limits / debts
  - The dragon still cannot be hit by a sword while orbiting
    (melee reach is the 3-block pick ray, v28 note) — the
    perch is still the sword phase; v30 only makes the AIR
    phase fight back. The player still needs a bow (or a
    pillar climb) to damage it mid-orbit, as in modern MC.
  - A pillar between dragon and player blocks `canSee` for a
    few seconds of each orbit — the dragon simply holds its
    fire until the line clears; no lead prediction (the
    fireball aims at the player's current position, ghast
    pattern — a strafing player can dodge it, which is the
    intended balance).
  - All of it is build-only until a live test on device (enter
    the End, stand on the island while the dragon orbits — do
    NOT climb a pillar, do NOT wait for the perch — check the
    dragon fires fireballs toward the player every few seconds,
    the fireballs are visible and explode near the player,
    the rate/damage feel fair (not spam, dodgeable), then
    check perch + heal-beam + respawn v27-v29 still work, no
    crash, old features intact).

- **v34 — Wither boss, stage 1 (SHIPPED, build-only).**
  Pays the v33 "Next" candidate: the skull (v32) + soul sand (v21)
  already existed, so the ritual had all its ingredients — the boss
  itself is what was missing. Stage 1 ships the whole loop
  (ritual → boss → projectiles → boss bar → nether star); stage 2
  is the fight-feel polish (see debts).
  ### Audit (read from code before writing anything)
  - **Wither skull item (v32)**: `Item::witherSkull`, a plain
    `Item(141)` → id 397, icon (3,11) — NOT placeable
    (`Item::useOn` defaults to false). A ritual needs placeable
    skulls, so the item needed a `useOn` (BedItem precedent:
    face-UP check, `isEmptyTile` + `isSolidBlockingTile`,
    `setTileAndData`, `count--`).
  - **Soul sand**: `Tile::hellSand` id 88, tex 171 (v21). Exists.
  - **Flying-boss patterns**: Ghast (v21 — Monster + gravity-free
    `travel`, fireball every 3 s within 28) and EnderDragon (v22 —
    Monster + `steerTo` orbit, `Mob::tick` instead of
    `Monster::tick` so PEACEFUL never deletes the boss, 48-block
    `findAttackTarget` override, fireball within 40, `heal`
    override because `Mob::heal` caps at 20). The wither is the
    dragon's shape with player-chase steering instead of an orbit.
  - **Projectile**: Fireball (v21 — Throwable, no gravity,
    explode r1 + fire on hit) is reusable as a pattern but NOT as
    the projectile: a wither skull must not set fire, and must
    apply the wither effect. No wither-skull projectile existed.
  - **Wither effect (v32)**: `Mob::witherEffectTicks`, 1 HP / 40
    ticks, no 1-HP floor, black hearts in the HUD — reusable
    directly from `WitherSkull::onHit` (AABB query like
    `ThrownPotion::onHit`).
  - **Nether star**: did NOT exist (zero hits). New item: ctor
    143 → modern id 399, icon drawn into the free
    `gui/items.png` cell (4,11), lang `item.netherStar.name`.
  - **Boss bar**: did NOT exist. But the pattern did — the XP bar
    (`Gui::renderExperience`: `GuiComponent::fill` + centred
    `Font::drawShadow`, `hudLayout` for the hotbar width), so a
    boss bar is ~25 lines scanning `Level::entities` for a live
    wither. It fits in this batch — no stage-2 deferral needed.
  - **Skull tile**: did NOT exist. Tile id 144 is free (modern MC
    skull id). Texture was the one real constraint: `terrain.png`
    has NO free cells left (v9/v11 used the last magenta ones).
    Cells 252/253 are referenced ONLY by the never-generated
    placeholder tiles `info_updateGame1/2` (248/249, not in the
    creative inventory, never placed by world gen) — so the skull
    face is drawn over cell 252, same spirit as v9/v11 reusing
    placeholder cells. The only casualty: those two placeholder
    tiles would render as skull cubes if ever placed via the fill
    command.
  - **Ritual-detection hook**: no precedent for structure
    detection on placement. The hook is `Tile::setPlacedBy`,
    which BOTH placement paths call (`TileItem::useOn` and the
    new `WitherSkullItem::useOn`) — so the check lives in
    `SkullTile::setPlacedBy` → `Wither::trySummonFromSkull` and
    covers both items. It runs ONLY on a successful skull
    placement, and only matches a complete T + 3 skulls, so no
    other building can false-positive. Server-side only
    (`!level->isClientSide` — in MP the client also runs the
    place path; the server summons).
  - **Save/load**: `Mob` health saves as a short (300 fits);
    `MobTypes::Wither` 46 < 64; `MobFactory::CreateMob` is the
    single load path (`EntityFactory` delegates to it).
  - **Sounds**: no wither audio files exist; the boss reuses the
    skeleton's registered sounds + `random.explode` for the
    summon/death (same choice as the dragon's placeholder sounds).
  ### What shipped
  - **Skull tile** (`SkullTile`, id 144, tex 252): full cube with
    the drawn skull face on all sides; breaking it drops the
    skull item back (`getResource` override — the default would
    drop the tile itself, id 144). Placed by the skull item or by
    the auto-registered TileItem (id 144); both run the ritual
    check via `setPlacedBy`.
  - **Placeable skull item** (`WitherSkullItem`, BedItem pattern):
    the v32 item (id 397, same icon/lang) now places the skull
    tile on the top face of a block. Old worlds keep working: the
    item id is unchanged (397), only its class changed.
  - **Ritual summon** (`Wither::trySummonFromSkull`): soul sand
    in a T (3 in a row + 1 below the middle) with a skull on top
    of each of the 3, along X or Z, with the fresh skull as the
    left/middle/right head. The whole structure (4 soul sand + 3
    skulls) is consumed, an explosion goes off (radius 3, no
    fire), and the wither appears above it.
  - **Wither boss** (`MobTypes::Wither` 46): 300 HP, fire-immune,
    1.0×3.5, hovers ~3 blocks above the nearest player
    (dragon `steerTo`, capped 0.35/tick), regenerates 1 HP / 20
    ticks via a direct health increment (NOT `heal()` — that
    sets `invulnerableTime` and would make the boss unhittable
    half the time), `heal` override caps at 300, survives
    PEACEFUL (`Mob::tick`, dragon precedent), never jumps
    (`updateAi` clears the flag — it flies), 48-block target
    acquisition (dragon precedent), melee within 4 blocks, one
    aimed wither skull every 2 s within 40 blocks. Drops 1
    nether star + 50 XP on a player kill (modern MC values).
    Model: `WitherModel` (header-only, natural scale like the
    dragon — 3 heads + ribcage body + tapering tail, 56 units =
    3.5 blocks; 64×64 texture, `mob/wither.png`, drawn) +
    `WitherRenderer` (plain `MobRenderer`, dragon precedent).
  - **Wither-skull projectile** (`EntityTypes::IdWitherSkull`
    88): Throwable, no gravity, Fireball-shaped; on impact
    applies Wither I (10 s) to every mob within 2 blocks (the
    wither itself is immune to its own skulls) + a small
    explosion (radius 1, no fire). Renderer is a billboarded
    near-black ball with a purple core (`WitherSkullRenderer`,
    FireballRenderer pattern, no texture).
  - **Nether star** (`Item::netherStar`, id 399): icon cell
    (4,11) drawn, lang added, in the creative inventory.
  - **Boss health bar** (`Gui::renderBossBar`): "Wither" name +
    purple bar, hotbar-wide, top-centred (XP-bar pattern), shown
    whenever a live wither is in the level — in EVERY game mode,
    so a creative summon test still sees it. Found by scanning
    `Level::entities`, so a reloaded wither shows its bar again
    for free.
  - Version bump 33 -> 34 (`app/build.gradle`,
    `AndroidManifest.xml`, `build-apk.sh`). Local `ndk-build`
    (r17c, both ABIs) passes: `libminecraftpe.so` arm64
    10,257,224 B / armv7 8,785,796 B, new symbols confirmed
    (`Wither::trySummonFromSkull`, `SkullTile::getResource/
    setPlacedBy`, `Gui::renderBossBar`, `WitherModel`,
    `WitherSkull::onHit`, `WitherRenderer`, `WitherSkullItem`,
    `WitherSkullRenderer`, `Item::netherStar`). CI
    `./gradlew assembleDebug` SUCCESS (run 42540797864, ~5m35s,
    00:33:08–00:38:43 UTC 2026-11-09); APK
    `minecraft-mcpe-android-0.6.1-debug-v34.apk`
    12,940,395 B, sha256
    8ae74e9eec1ef95f5d5441b73c86a1e3635e26ad685c63c7f5324faa8ba9153a,
    versionCode 34, `apksigner verify` v1+v2 true, `unzip -t`
    clean; the APK itself was re-checked to contain the new
    symbols in its `libminecraftpe.so` (arm64 10,257,224 B /
    armv7 8,785,796 B — byte-identical sizes to the local build)
    and the new `mob/wither.png` (64×64) + the skull face in
    terrain.png cell 252 + the nether-star icon in items.png
    cell (4,11).
  ### Known limits / debts (stage 2)
  - **One skull per volley, not three.** Modern MC fires from all
    three heads (a 3-skull spread). Stage 1 fires a single aimed
    skull every 2 s — the fight is fair but visibly thinner than
    modern MC.
  - **Skull tile is a full cube, top-face placement only.** Modern
    MC skulls are small heads and mount on walls too. The full
    cube reads as a black skull block; wall mounting needs a data
    value + a render shape.
  - **No wither rose.** Modern MC drops a wither rose where the
    wither kills a mob — needs a new flower tile + item, not just
    a drop.
  - **No armoured phase.** Modern MC gives the wither armour and
    arrow immunity below half HP. Stage 1 has flat 300 HP with
    regen all the way down.
  - **No wither spawn protection / spawn animation.** Modern MC
    grows the wither over ~2 s with an invulnerable charge-up and
    a bigger (power 7) summon blast; stage 1 summons instantly
    with a radius-3 blast.
  - The boss bar shows the FIRST live wither only (multiple
    withers share one bar); the bar has no wither icon, just the
    name + fill.
  - All of it is build-only until a live test on device (see the
    "Next" v34 test list: ritual, skulls, boss bar, star drop,
    no crash, old features intact).
- **v33 — Village chest loot (SHIPPED, build-only).**
  Pays the v23 debt "no village chest with loot" and the v31/v32
  fortress-chest debt — the oldest standing structure debt.
  ### Audit (read from code before writing anything)
  - The v23 worry ("placing a tile entity during postProcess has no
    precedent") turned out to be unfounded:
    `LevelChunk::setTile` / `setTileAndData` (server side) call
    `Tile::onPlace` for the placed tile (`LevelChunk.cpp:96,136`),
    and `ChestTile` extends `EntityTile`, whose `onPlace`
    (`EntityTile.cpp:6-10`) calls `level->setTileEntity(x, y, z,
    newTileEntity())` — so placing a chest block via
    `level->setTile` during postProcess creates the
    `ChestTileEntity` synchronously and registers it in
    `Level::tileEntities` + the chunk map. The mechanism is the
    normal block-place path, not a special tile-entity constructor
    call. `ChestTile::newTileEntity()` returns a `ChestTileEntity`
    via the factory; `Tile::chest` is id 54.
  - Save/load + eviction come free from the v17 machinery:
    `ChestTileEntity::shouldSave()` is true while any slot is
    non-empty, so a filled chest is written to the global
    `entities.dat` (`ExternalFileLevelStorage::saveEntities`);
    `TileEntity::loadStatic` rebuilds it on load (`"Chest"` id
    string); `ChunkCache::reattachTileEntities` re-attaches the
    ORIGINAL entity before postProcess (guarded by
    `hasTileEntityAt`, no duplicate); disk-loaded chunks have
    `terrainPopulated=true`, so postProcess never re-runs and never
    places a second chest. Eviction detaches (chunk map cleared,
    object lives on in `Level::tileEntities`) and saves. An emptied
    chest reports `shouldSave()==false` and is simply re-created
    empty on reload — empty is empty, nothing is lost.
  - One real trap found by the audit (shaped the design):
    `Level::getTileEntity` goes through
    `LevelChunk::getTileEntity`, which LAZY-CREATES a fresh empty
    tile entity when the chunk map has none — and during a game
    tick the onPlace-created entity may still sit in
    `Level::pendingTileEntities` (`updatingTileEntities`), so the
    lazy twin would be created, filled with the loot, and then
    DELETED by the pending-flush dedup (which keeps the first,
    empty, entity). The fill helper therefore NEVER calls
    `getTileEntity`: it scans `Level::tileEntities` +
    `Level::pendingTileEntities` directly (both public,
    `Level.h:306-307`) for the entity at (x,y,z) and fills only if
    found; if neither list has it the chest stays empty rather than
    risking a duplicate.
  - Determinism: loot Randoms are re-seeded per village/fortress
    hash + house index on every `tryPlace*` call, so every chunk
    overlapping a structure computes the same loot; the chest block
    lands in exactly one chunk (clipped setters + an `inChunk`
    guard, so no cross-chunk `getTile` during postProcess), and
    postProcess runs once per chunk (`terrainPopulated` guard).
  - Loot items all already existed: bread/wheat/carrot/apple/coal/
    iron ingot/string/emeraldGem (v23 currency) for villages;
    iron/gold ingot/coal/bread/obsidian/diamond (`Item::emerald` IS
    the diamond gem in this tree's old naming)/flint & steel for
    the fortress. `FillingContainer::setItem` COPIES the instance,
    so stack temporaries are safe.
  ### What shipped
  - **Village loot chests** (2 per village): the two 7x5 houses —
    N (i=0) and the workbench house S (i=1) — each hold a chest in
    the corner opposite the floor lamp (x0+1, groundY+1, z0+d-2),
    on the wood floor with air above (opens normally). 3–5 stacks
    from the village table: bread 2–4, wheat 3–6, carrot 2–4,
    apple 1–3, coal 2–4, iron ingot 1–3, string 1–3, emerald 1–2
    (rare — seeds the trading loop).
  - **Fortress loot chest** (1 per fortress): in the blaze room,
    east wall middle (wx+22, D+1, wz) — clear of the glowstone
    corners, the west doorway and the blaze spawn spots. 3–4
    stacks: iron ingot 1–3, gold ingot 1–3, coal 1–3, bread 1–3,
    obsidian 1–2, diamond 1 (rare), flint & steel 1 (rare). No blaze
    rods / nether wart: the fortress already has blazes and a wart
    room next door.
  - **Shared helper** (`ChestLoot`, new
    `src/world/level/levelgen/ChestLoot.{h,cpp}`, added to
    `Android.mk`): `fillChest` (weighted table, random empty
    slots) + `fillVillageChest` / `fillFortressChest` tables.
    `VillageFeature::buildHouse` gains a chest flag + per-house
    loot Random; `NetherFortressFeature::placeFortress` gains the
    fortress hash.
  - Version bump 32 -> 33 (`app/build.gradle`,
    `AndroidManifest.xml`, `build-apk.sh`). Local `ndk-build`
    (r17c, both ABIs) passes: `libminecraftpe.so` arm64
    10,224,456 B / armv7 8,773,508 B, new symbols confirmed
    (`ChestLoot::fillChest/fillVillageChest/fillFortressChest`).
    CI `./gradlew assembleDebug` SUCCESS (run 42538696263,
    ~4m08s, 00:10:03–00:14:11 UTC 2026-11-09); APK
    `minecraft-mcpe-android-0.6.1-debug-v33.apk`
    12,928,993 B, sha256
    f24f41cdfc547f0ecb58301fb51e593a62c77028641038d2c2d00cebacde13b4,
    versionCode 33, `apksigner verify` v1+v2 true, `unzip -t`
    clean; the APK itself was re-checked to contain the new
    symbols in its `libminecraftpe.so` (arm64 10,224,456 B /
    armv7 8,773,508 B — byte-identical sizes to the local build).
  ### Known limits / debts
  - Chests only exist in villages/fortresses generated AFTER v33:
    chunks already populated in v23–v32 worlds never re-run
    postProcess, so old villages/fortresses do not gain chests
    retroactively (same rule as the v25 swamp-hut cauldron).
  - No per-village loot variety beyond the two tables (same table
    for every village/fortress, only the rolls differ); no double
    chests (adjacent chests are refused by
    `ChestTile::mayPlace`, and the layout keeps them apart).
  - All of it is build-only until a live test on device (find a
    village in plains/desert — NEW chunks only, walk ~384+ blocks
    from spawn if the old area is already generated — check the N
    house and the workbench house each have a chest with 3–5
    sensible stacks, open it, take some loot, quit, reload — the
    taken loot stays gone and the rest stays; walk away >16 chunks
    and back — the chest is still there, contents intact, no
    duplicate chest; enter the Nether, find a fortress, check the
    blaze-room chest; no crash, old features intact).
- **v32 — Wither skeleton (SHIPPED, build-only).**
  Pays the v31 debt "no wither skeleton".
  ### Audit (read from code before writing anything)
  - `Skeleton` (`Skeleton.cpp`): a bow mob — `checkHurtTarget`
    shoots an `Arrow` every 3 s within 10 blocks and sets
    `holdGround = true` (stands still and shoots); `aiStep` burns
    it in daylight; `getCarriedItem` returns its bow; `getMaxHealth`
    10 (comment says 15); drops 0–2 arrows + 0–2 bones.
  - Melee precedent, NO new AI needed: the OLD AI in
    `PathfinderMob::updateAi` finds the target, paths toward it
    while `!holdGround`, and calls the virtual `checkHurtTarget`
    when it can see it. `Monster::checkHurtTarget` is the melee
    version (hit within 2 blocks, never sets `holdGround`), so a
    skeleton subclass that overrides `checkHurtTarget` with
    Monster's version walks up and swings — the witch (v24) proves
    the same old-AI + `checkHurtTarget`-override pattern. The
    zombie's new-AI `MeleeAttackGoal` was NOT reused (it needs a
    goal/target selector setup the skeleton never had; the old-AI
    path is the skeleton's own path). `PigZombie` proves the
    carried-weapon pattern (`ItemInstance` member + `getCarriedItem`
    override, here a stone sword, modern MC).
  - Effect system (v20): 7 tick-down timers on `Mob`
    (`Mob::tick`, poison 1 HP / 25 ticks with a `health > 1` guard
    so it never kills, regeneration 1 HP / 50 ticks), all
    session-only (never saved). Wither is timer #8 on the exact
    same pattern, with the one deliberate difference: NO health
    guard — modern MC wither drains 1 HP / 40 ticks (Wither I)
    and CAN kill. No HUD effect display existed (hearts are plain
    red); the fix is black-heart sprites + a `withered` branch in
    `Gui::renderHearts`.
  - No skull item existed (zero hits for "skull"/"wither" as an
    item); modern MC id 397 = ctor 141 + 256 (ctor 141 was free),
    icon into the fully transparent `gui/items.png` cell (3, 11),
    lang key added (`item.skullWither.name`). No wither boss
    exists — the skull is a drop/creative item only in v32.
  - Fortress spawn: `ensureBlazes` (v31, `ChunkCache::getChunk` +
    `postProcess` hooks, Nether-only + centre-chunk + terrain-
    populated guards, count-and-top-up to 3 inside the blaze
    room) reuses verbatim for wither skeletons (top-up to 2,
    corridor/bridge spots — the blaze room stays the blazes').
    `MobTypes` 45 was free (44 = magma cube); `EntityFactory`
    loads via `MobFactory::CreateMob`, so save/load works with
    the new case added. `Android.mk` lists monster sources
    explicitly — the new `.cpp` must be added there or the link
    fails (caught before CI).
  ### What shipped
  - **Wither skeleton** (`WitherSkeleton`, `MobTypes` 45,
    `MobFactory` case, save/load via `EntityFactory`): a
    `Skeleton` subclass, but melee — `checkHurtTarget` is
    `Monster`'s melee version (hit within 2 blocks, never sets
    `holdGround`, so the old AI keeps walking it to the target),
    `aiStep` is `Monster::aiStep` (no sunlight burn), stone
    sword carried (`getCarriedItem`, modern MC), 20 HP, 5 melee
    damage, fire-immune, black skeleton texture
    (`mob/witherskeleton.png`, the skeleton darkened ×0.22 with
    pale eyes — same tint precedent as the v31 magma cube).
    The plain skeleton is untouched (still shoots arrows).
    `Android.mk` needed the new `.cpp` added (explicit source
    list — caught by the local build before CI).
  - **Wither effect** (timer #8 on `Mob`, same pattern as v20
    poison): `witherEffectTicks`, 1 HP / 40 ticks (modern MC
    Wither I), applied for 10 s by `WitherSkeleton::doHurtTarget`
    on any mob it hits — with the one deliberate difference
    from poison: NO `health > 1` guard, so wither CAN kill.
    Session-only like all v20 timers (never saved). Visuals: a
    black smoke puff on the afflicted mob every 20 ticks, and
    black hearts in the HUD (`Gui::renderHearts` switches to
    the black sprites at u 88 full / 97 half in `icons.png`
    row 0, drawn from the red hearts) while the player is
    withered.
  - **Wither skull item** (modern MC id 397 = ctor 141 + 256;
    icon drawn into the blank `gui/items.png` cell (3, 11);
    lang `item.skullWither.name=Wither Skeleton Skull`; in the
    creative inventory). Drop: 2.5% from a wither skeleton
    (modern MC rate), alongside 0–2 bones and 0–1 coal (1/3).
  - **Fortress spawn** (`ensureWitherSkeletons`, same top-up
    pattern as `ensureBlazes`, hooked in `ChunkCache::getChunk`
    + `postProcess`): 2 per fortress, on the corridor
    (wx, wz−7) and the bridge (wx+8, wz) — the blaze room stays
    the blazes'. Nether-only, centre-chunk, terrain-populated
    guards, same as the blazes.
  - Version bump 31 → 32 (`app/build.gradle`,
    `AndroidManifest.xml`, `build-apk.sh`). Local `ndk-build`
    (r17c, both ABIs) passes: `libminecraftpe.so` arm64
    10,212,168 B / armv7 8,769,412 B, new symbols confirmed
    (`WitherSkeleton` ctor / `doHurtTarget` / `checkHurtTarget`
    / `dropDeathLoot`, `NetherFortressFeature::
    ensureWitherSkeletons`, `skullWither`, texture string). CI
    `./gradlew assembleDebug` SUCCESS (run 42537306964, ~4m50s,
    23:53:12–23:58:02 UTC 2026-11-08); APK
    `minecraft-mcpe-android-0.6.1-debug-v32.apk`
    12,927,400 B, sha256
    bfe1c76bc078f00eef65475856f9afe67d00a04bbdcd9e2147d479816d40c3bd,
    versionCode 32, `apksigner verify` v1+v2 true, `unzip -t`
    clean; the APK itself was re-checked to contain the new
    symbols in its `libminecraftpe.so` (arm64 10,216,264 B /
    armv7 8,769,412 B) and the new `mob/witherskeleton.png`
    texture (64×32, dark bones + pale eyes), the black-heart
    sprites in `icons.png` and the skull icon in `items.png`.
  ### Known limits / debts
  - No wither boss: the skull is a drop/creative item only.
    Summoning the boss (3 skulls + 4 soul sand in a T, boss
    entity + health bar + AI) is a multi-version project of its
    own — the next big Nether boss candidate, not a v-batch.
  - No fortress loot chest (the v23 village-chest debt stands —
    placing a tile entity during postProcess still has no
    precedent); no blaze spawner tile (v31 debt stands).
    (PAID in v33 for the chest — see v33; the blaze spawner tile
    still stands.)
  - The wither skeleton has no ranged attack at all (modern MC
    neither — melee only) and no skull-helmet rendering (it
    carries the sword; no head-slot item exists in this tree).
    Wither skeletons spawn ONLY in fortresses (top-up), never
    from the general Nether spawn list — modern MC also ties
    them to fortresses. A wither skeleton killed by another
    mob's damage still drops loot (v8 rule: XP only for player
    kills, loot always).
  - All of it is build-only until a live test on device (enter
    the Nether, find a fortress, check 2 black sword-carrying
    skeletons in/near the corridor/bridge, check they walk up
    and swing melee — never shoot arrows — check the wither
    effect drains HP, turns the hearts black and can kill if
    left alone, check a skull drops rarely, check the plain
    skeleton still shoots, no crash, old features intact).
- **v31 — Nether fortress + magma cube (SHIPPED, build-only).**
  Pays the v21 debt "no Nether fortress / nether brick structures,
  no magma cube, no wither skeleton" — the fortress and the magma
  cube. The wither skeleton and the blaze spawner stay debts (below).
  ### Audit (read from code before writing anything)
  - Structure pattern: `VillageFeature` (v23) + `SwampHutFeature`
    (v24) — deterministic grid-cell placement (splitmix64 hash of
    seed + cell) + clipped per-chunk writes in
    `RandomLevelSource::postProcess` + entity top-up from
    `ChunkCache` (`ensureVillagers` / `ensureWitch`). The Nether
    reuses it directly: `ChunkCache::postProcess` calls
    `source->postProcess` for ANY source, and `NetherLevelSource::
    postProcess` was an empty no-op — the fortress hooks in there,
    no ChunkCache change needed for placement.
  - Nether terrain (`NetherLevelSource::getChunk`): bedrock floor
    y=0 (+scatter y1-4) and ceiling y=127, rolling netherrack floor
    height 28-62 (value noise), lava sea y<=31 where the floor dips
    below 32, soul sand patches, quartz ore, glowstone under the
    ceiling. The Nether has NO usable heightmap (the bedrock
    ceiling at y=127 would read as the "top"), so the fortress deck
    Y is derived from the terrain noise itself (see below).
  - Nether brick: ALREADY existed — tile 112 (`Tile::netherBrick`,
    tex 14x16 cell), stairs 114, smelt recipe netherrack ->
    netherbrick item, stairs recipe, in the creative inventory
    (`Inventory.cpp:150`). No new tile was needed for the fortress.
  - Blaze: ALREADY existed (v21, `MobTypes::Blaze` 40, 20 HP,
    fireball <=16 blocks every 3 s + melee, drops 0-1 blaze rod,
    fire-immune). It only spawned randomly across the Nether via
    `NetherLevelSource::getMobsAt` (weight 6) — nothing tied it to
    a fortress, because there was no fortress.
  - Blaze spawner tile: DOES NOT exist. `Tile::mobSpawner` is only
    DECLARED (`Tile.h:169`), never defined or registered, and no
    spawner tile-entity class exists anywhere in the tree (the
    dungeon spawner/chest feature is `#if 0`'d out, v23 note). A
    spawner tile would need a new tile + tile entity + tick
    spawning — out of scope; blazes come from the top-up instead.
  - Wither skeleton / magma cube: NEITHER existed (zero hits for
    "wither"/"magma" as mobs; `Item::magmaCream` existed since v20
    as a brewing ingredient). Magma cube is a `Slime` subclass away
    (hop movement, squash-and-stretch via `SlimeRenderer`'s
    `(Slime*)` cast, zombie goals — all inherited). Wither skeleton
    is NOT: `Skeleton` is a bow mob (`Skeleton::checkHurtTarget`
    shoots an `Arrow` every 3 s within 10 blocks), so a wither
    skeleton needs melee AI + a sword + the wither effect (no
    wither effect exists — v20's system has 7 timers, wither is
    not one) + a wither-skull item (does not exist). Deferred.
  - Nether wart: farmable since v21 (`NetherWartTile` id 115, 4
    stages data 0-3, grows on soul sand in the dark, mature drops
    2-4). A wart room plants it on soul sand the fortress places
    itself, so it needs no terrain soul-sand patch underneath.
  ### What shipped
  - **Fortress structure** (`NetherFortressFeature`, new): one
    fortress per 20x20-chunk grid cell (320 blocks, splitmix64
    hash of seed + cell, different salt from villages), centre
    chunk jittered inside the cell. ~49x33 blocks of nether
    brick: bridge E-W (deck 33 long x 5 wide + 1-high curbs,
    walkable middle 3), corridor N-S (3 wide, 29 long, side
    walls + flat roof, open at both ends), blaze room east (7x7,
    walls + roof, doorway facing the bridge, glowstone in the 4
    inner ceiling corners), wart room west (7x5, soul-sand floor,
    10 mature wart in two rows, doorway facing the bridge, 2
    glowstone). Nether-brick support pillars run from the deck
    down to solid ground under the bridge (every 4 blocks), room
    corners/midpoints and corridor ends — through air AND lava,
    so nothing floats over the lava sea or the void.
  - **Deck height, deterministic:** `NetherLevelSource::
    getFloorHeight` (new public static — the exact
    `28 + (int)(noise2D * 34)` formula `getChunk` uses) gives the
    floor height per column; the deck Y is the MAXIMUM over the
    fortress footprint, so the deck is never buried where the
    terrain rises. A fortress is skipped when that deck would sit
    in/just above the lava sea (deckY < 36; the sea tops out at
    y=31). Pure noise — no chunk reads — so every chunk
    overlapping a fortress agrees on the same deck Y, and the
    fortress is saved with the chunks like any other terrain.
  - **Blazes live in the fortress** (`ensureBlazes`, same top-up
    pattern as villagers/witch, hooked in `ChunkCache::getChunk`
    + `postProcess`, Nether-only + centre-chunk + terrain-
    populated guards): blazes within the fortress box are counted
    and topped up to 3, spawned inside the blaze room. Random
    Nether blaze spawning (`getMobsAt` weight 6) is unchanged.
  - **Magma cube** (`MobTypes` 44, `ER_SLIME_RENDERER` reused):
    `Slime` subclass, 8 HP, hop movement + squash-and-stretch
    inherited, fire-immune, drops 0-2 magma cream
    (`Item::magmaCream`, v20 ingredient), orange magma texture
    (`mob/magmacube.png`, a luminance-preserving tint of
    `mob/slime.png`). Spawns in the Nether via `getMobsAt`
    (weight 6, clusters 1-2, same as blazes). Registered in
    `MobFactory` (save/load works via `EntityFactory::loadEntity`,
    type id 44 < 64).
  - Version bump 30 -> 31 (`app/build.gradle`,
    `AndroidManifest.xml`, `build-apk.sh`). Local `ndk-build`
    (r17c, both ABIs) passes: `libminecraftpe.so` arm64
    10,216,264 B / armv7 8,765,316 B, new symbols confirmed in
    the object files (`NetherFortressFeature::tryPlaceFortress/
    ensureBlazes/getDeckY`, `NetherLevelSource::postProcess/
    getFloorHeight`, `MagmaCube` ctor + `getDeathLoot`). CI
    `./gradlew assembleDebug` SUCCESS (run 42536116776, ~3m10s);
    APK `minecraft-mcpe-android-0.6.1-debug-v31.apk`
    12,922,872 B, sha256
    deba056b71e63e1b202f2cddb7d2597480b6d42fec088b0650f008c932315f2c,
    versionCode 31, `apksigner verify` v1+v2 true, `unzip -t`
    clean; the APK itself was re-checked to contain the new
    symbols in its `libminecraftpe.so` (arm64 10,216,264 B /
    armv7 8,765,316 B — byte-identical sizes to the local build)
    and the new `mob/magmacube.png` texture (64x32, orange tint).
  ### Known limits / debts
  - No wither skeleton (see audit: needs melee AI + wither
    effect + wither-skull item — the fortress's other signature
    mob, next version's candidate).
  - No blaze spawner tile (see audit: `Tile::mobSpawner` is
    declared but never defined; no spawner tile entity exists).
    The blaze room's blazes come from the top-up, not a spawner
    block — a room cleared of blazes refills when its centre
    chunk reloads (same deliberate top-up as villagers/witch).
  - No fortress loot chest (placing a tile entity during
    postProcess has no precedent in this tree — the v23 village
    chest debt stands, same reason). (PAID in v33 — see v33.)
  - No nether-brick fence (no such tile exists — the bridge
    curbs are full nether-brick blocks, not fences); flat roofs,
    not gabled; one fixed fortress layout (no rotation/variants);
    magma cubes are single-size only (no splitting on death,
    same as the v12 slime); the magma texture is a tint of the
    slime sprite, not the modern magma-cube sprite; magma cube
    sounds are the slime's (silent — `mob.slime` unregistered).
  - All of it is build-only until a live test on device (enter
    the Nether, walk ~320+ blocks and find a fortress — bridge/
    corridor/rooms in nether brick on pillars, never floating
    over lava — check 3 blazes in/near the blaze room, check the
    wart room's 10 mature wart, check magma cubes hopping on the
    Nether floor and dropping magma cream, quit/reload — the
    fortress is still there at the same place, no crash, old
    features intact).

## Next (v24)
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
- **Device verification for the End (v22's open item).** See the
  v22 test list above (portal ring, island, dragon fight, exit
  portal, quit-in-End reload). The v21 APK stays the fallback
  until this stage passes.
- **Device verification for villages + trading (v23's open item).**
  See the v23 test list above. The v22 APK stays the fallback
  until this stage passes.
- **Device verification for the witch + swamp hut (v24's open item).**
  See the v24 test list above. The v23 APK stays the fallback
  until this stage passes.
- **Device verification for player splash potions + cauldron (v25's
  open item).** See the v25 test list above. The v24 APK stays the
  fallback until this stage passes.
- **Device verification for brewing upgrades (v26's open item).**
  Brew an effect potion + redstone dust -> the name shows the
  extended duration and the effect lasts noticeably longer;
  + glowstone dust -> the name shows "II" and the effect is
  noticeably stronger (faster / more damage / more healing);
  both also as splash potions (throw an extended / level II splash
  at a mob — the variant effect must be felt); brew glowstone into
  an extended potion (and redstone into a level II one) -> refused,
  the potion is unchanged; no crash. The v25 APK stays the fallback
  until this stage passes.
- **Device verification for end crystals + dragon respawn (v27's
  open item).** Kill the dragon once (or use a world where it is
  already dead), craft 4 end crystals (7 glass + eye of ender +
  ghast tear each), place them on 4 pillar tops or the exit-portal
  ring, check the dragon respawns at full HP and fights again;
  shoot/punch a crystal and check the power-6 blast; check the
  respawned dragon heals near a surviving crystal; check the exit
  portal + egg are right after the second death; no crash. The
  v26 APK stays the fallback until this stage passes.
- **v25 — Player splash potions + cauldron.** SHIPPED (see "Done").
- **v26 — Brewing upgrades: redstone + glowstone.** SHIPPED (see "Done").
- **v27 — End crystals + dragon respawn.** SHIPPED (see "Done").
- **Device verification for dragon perch + initial crystals (v28's
  open item).** Enter the End, check 10 crystals on the pillar
  tops; watch a full cycle orbit -> descend -> sit ~8 s -> takeoff;
  hit the dragon with a sword while perched and check damage lands;
  check it never wedges on a perch or falls to the void; kill it
  and check the exit portal STAYS (no instant auto-respawn from
  the surviving pillar crystals); then place 4 fresh crystals and
  check the v27 respawn still works and the new dragon perches
  too; no crash. The v27 APK stays the fallback until this stage
  passes.
- **v28 — Dragon perch + initial end crystals.** SHIPPED (see "Done").
- **Device verification for the heal beam + perch-hop fix (v29's
  open item).** Enter the End, let the dragon fly near a crystal
  and check the pink/white beam is visible from the crystal to
  the dragon; check it vanishes when the dragon flies out of
  range / dies or the crystal is destroyed; check the beam does
  not cover the screen or glitch when several crystals heal at
  once; watch a full perch cycle and check the dragon never hops
  on the ring; no crash. The v28 APK stays the fallback until
  this stage passes.
- **v29 — Heal-beam visual + perch-hop fix.** SHIPPED (see "Done").
- **Device verification for dragon fireball targeting (v30's
  open item).** Enter the End, stand on the island while the
  dragon orbits — do NOT climb a pillar, do NOT wait for the
  perch — check the dragon fires fireballs toward the player
  every few seconds, the fireballs are visible and explode near
  the player, the rate/damage feel fair (not spam, dodgeable);
  then check perch + heal-beam + respawn v27-v29 still work, no
  crash. The v29 APK stays the fallback until this stage
  passes.
- **Device verification for the Nether fortress (v31's open
  item).** Enter the Nether, walk ~320+ blocks and find a
  fortress — bridge/corridor/rooms in nether brick on pillars,
  never floating over lava; check 3 blazes in/near the blaze
  room (they shoot fireballs, drop blaze rods), check the wart
  room's 10 mature wart (harvest gives 2-4 each), check magma
  cubes hopping on the Nether floor and dropping magma cream,
  quit/reload — the fortress is still there at the same place;
  no crash. The v30 APK stays the fallback until this stage
  passes.
- **v30 — Dragon fireball targeting.** SHIPPED (see "Done").
- **v31 — Nether fortress.** SHIPPED (see "Done").
- **Device verification for the wither skeleton (v32's open
  item).** Enter the Nether, find a fortress, check 2 black
  sword-carrying skeletons in/near the corridor/bridge (they walk
  up and swing melee — never shoot arrows), check the wither
  effect drains HP, turns the hearts black and can kill if left
  alone, check a skull drops rarely (2.5% — kill several), check
  the plain skeleton still shoots arrows, no crash. The v31 APK
  stays the fallback until this stage passes.
- **Device verification for village + fortress chest loot (v33's
  open item).** Find a village in plains/desert — NEW chunks only,
  walk ~384+ blocks from spawn if the old area is already generated
  — check the N house and the workbench house each have a chest
  with 3–5 sensible stacks (bread/wheat/carrot/apple/coal/iron/
  string, rarely an emerald); open a chest, take some loot, quit,
  reload — the taken loot stays gone and the rest stays; walk away
  >16 chunks and back — the chest is still there, contents intact,
  no duplicate chest; enter the Nether, find a fortress, check the
  blaze-room chest (iron/gold/coal/bread/obsidian, rarely diamond
  or flint & steel); no crash. The v32 APK stays the fallback
  until this stage passes.
- **v32 — Wither skeleton.** SHIPPED (see "Done").
- **v33 — Village chest loot.** SHIPPED (see "Done").
- **Device verification for the wither boss, stage 1 (v34's open
  item).** Uninstall the old APK first (fresh CI debug keystore).
  Collect 3 wither skulls (2.5% drop from wither skeletons in a
  fortress — or take them from the creative inventory), build the
  soul-sand T (3 in a row + 1 below the middle) and place a skull
  on top of each of the 3 — check the structure is consumed, an
  explosion goes off and the wither appears above it, flying;
  check the purple "Wither" boss bar at the top of the screen and
  that it drains as the boss takes damage; stand under it and
  check it shoots dark skull projectiles that explode and give the
  wither effect (black hearts, HP draining, can kill); kill it
  and check it drops exactly 1 nether star (+ 50 XP); break a
  placed skull before completing the ritual and check the skull
  item drops back; check an incomplete T (missing stem, or only 2
  skulls) does NOT summon; check the boss stays in peaceful
  (it must not vanish) and the old features still work, no crash.
  The v33 APK stays the fallback until this stage passes.
- **Next version (v35 candidate): wither boss, stage 2.** Stage 1
  shipped the whole loop, so stage 2 is the fight-feel polish
  from the v34 debts, in this order: (1) three-head volley — one
  aimed skull per head per volley, the most visible gap versus
  modern MC; (2) small-head skull shape + wall mounting, so the
  ritual pieces look like skulls instead of black cubes; (3)
  wither rose (needs a new flower tile + item). The armoured
  phase (arrow immunity below half HP) stays after those — it is
  a balance change that needs device feel first, not more code.
- **v34 — Wither boss, stage 1.** SHIPPED (see "Done").

## Queued (priority order — re-evaluate after each version)

- **Witch — AFTER brewing.** SHIPPED as v24 (see "Done").
- **Player-throwable splash potions (new, v24 follow-up).** SHIPPED
  as v25 (see "Done").
- **Cauldron tile (v20/v24 debt).** SHIPPED as v25 (see "Done").
- **v22 — The End.** SHIPPED (see "Done").
- **v23 — Villages + trading.** SHIPPED (see "Done").
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
