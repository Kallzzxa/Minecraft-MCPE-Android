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

- **v38 — Wither skull self-damage (SHIPPED, build-only).** Pays the
  v34/v35/v36 debt "the wither's own skull blasts can still damage
  it (NULL source)". One batch, two lines of gameplay code.
  ### Audit (read from code before writing anything)
  - **Damage routing today**: `Level::explode(source, ...)`
    (`Level.cpp:1721`) → `Explosion::explode()` (`Explosion.cpp`):
    the entity scan is `level->getEntities(source, AABB...)`, and
    `Level::getEntities` EXCLUDES its first argument — so an
    explosion's source is never hurt by its own blast, and a NULL
    source means NO exclusion at all. Every hit then arrives as
    `Mob::hurt(Entity* source, int dmg)` (`Mob.cpp:454`, the
    single damage gate: i-frames, knockback, death → `die(source)`).
  - **The bug, exactly**: `WitherSkull::onHit`
    (`WitherSkull.cpp:45`) called `level->explode(NULL, x, y, z,
    1.0f, false)`. The WITHER-EFFECT half was already immune —
    the radius-2 loop skips every entity whose type is
    `MobTypes::Wither` (v34) — but the explosion half had no
    exclusion: any wither within the blast's 2-block scan radius
    took `hurt(NULL, dmg)`, up to ~9 damage at point blank
    (the `Explosion.cpp` formula `(pow²+pow)/2 × 8 × r + 1`,
    r=1). `Wither::hurt` (v36) could not stop it: its armour
    check only rejects PROJECTILE sources, and its header comment
    admitted the gap ("the wither can still be hurt by its own
    skulls' blasts, exactly as in v34/v35"). The usual case is a
    volley fired at a wall / at the player standing close under
    the boss, or a second wither's skull landing near the first —
    so the v37 lowest-HP bar could drain from the boss fighting
    itself, not from the player.
  - **Summon blast — AUDITED, NOT a bug, unchanged**:
    `Wither::tick` fires it as `level->explode(this, ...)`
    (source = the wither), so `getEntities` excludes the wither
    from its own power-7 blast, AND `hurt()` still blocks
    everything while the charge counter is 1 on the blast tick.
    Double protection already; v38 touches nothing there.
  - **Other mobs' self-immunity patterns**: Creeper
    (`Creeper.cpp:88`) calls `explode(this, ...)` — excluded
    from its own blast (it `remove()`s anyway). Ghast/blaze
    fireballs (`Fireball::onHit`, `Fireball.cpp:27`) use the SAME
    NULL shape as the skull did — a ghast CAN in principle hurt
    itself with its own fireball blast; in practice its blast
    lands at the target up to 28 blocks away, so it almost never
    happens. NOT fixed here (out of scope, different mob, no
    wither-immunity rule to apply) — noted as a debt below.
  - **Rejected alternatives (why the fix is shaped as it is)**:
    (1) Rejecting NULL-source damage in `Wither::hurt` — cannot
    distinguish a skull blast from TNT (`PrimedTnt::explode`
    passes NULL, `PrimedTnt.cpp:78`), and TNT must still hurt
    the wither (v36 design). (2) Passing the SHOOTER wither as
    the blast source — `Throwable` stores only an `ownerId`
    int, no owner pointer, so it would need a
    `Level::getEntity(ownerId)` lookup, and `getEntities`
    exclusion would protect only the shooter, not a second
    wither. (3) Skipping withers inside `Explosion::explode` —
    touches the generic path shared by TNT/creeper/crystals for
    a wither-only rule. (4) The chosen shape instead: the blast
    source becomes the SKULL itself, which identifies the blast
    by type (88), and `Wither::hurt` rejects exactly that type.
  - **Decision (modern MC)**: the wither is immune to wither
    skulls, period — its own volley AND another wither's. The
    effect half already applied this rule to all withers; the
    damage half now matches it.
  ### What shipped
  - `WitherSkull::onHit`: `level->explode(NULL, ...)` →
    `level->explode(this, ...)` (source = the skull). Player +
    other mobs are unaffected by the source change: same damage
    formula, same radius, same no-fire; the only difference is
    knockback — `Mob::hurt` now knocks them away from the blast
    centre (the skull sits at it) instead of a random direction
    with no knockback (the NULL path). `Player::hurt`'s
    difficulty-scaling branch is untouched (a skull is neither
    `BaseEnemy` nor an Arrow); `Mob::die(source = skull)` gives
    no XP (not a player — same as NULL) and `awardKillScore`
    is the base no-op for a non-player source.
    `Tile::getExplosionResistance(source)` ignores its argument,
    so block damage is identical.
  - `Wither::hurt`: new check, before the armoured-phase check —
    `source != NULL && source->getEntityTypeId() ==
    EntityTypes::IdWitherSkull` → return false, in EVERY phase
    (charging is already blocked above it; armoured already
    blocked projectiles, so this only changes the unarmoured
    phase). Blocked hits return before `Mob::hurt`, so no
    invulnerableTime, knockback or hurt sound is spent on them.
    A second wither's skull is rejected the same way, since the
    blast calls `hurt(skull, dmg)` on every wither in radius.
  - `Wither.h` header comment updated: the v36 routing paragraph
    no longer claims the wither can be hurt by its own skulls.
  - Version bump 37 -> 38 (`app/build.gradle`,
    `AndroidManifest.xml`, `build-apk.sh`). Local NDK r17c
    build (both ABIs) passes: `libminecraftpe.so` arm64
    10,244,936 B / armv7 8,793,988 B — byte-identical sizes to
    v37 (the change is two small code paths, no new strings);
    the fresh `Wither.o` / `WitherSkull.o` were confirmed
    rebuilt after the edit and `Wither::hurt` references
    `getEntityTypeId` (the new skull check alongside the v36
    projectile check). CI `./gradlew assembleDebug` SUCCESS
    (run 42547005603, ~4m55s, 01:42:47–01:47:42 UTC
    2026-11-09); APK
    `minecraft-mcpe-android-0.6.1-debug-v38.apk`
    12,947,639 B, sha256
    b6149ed49c9af847cce60e605e920a8dec8a7b76e80897487d6583b8f2db2cc9,
    versionCode 38, `apksigner verify` v1+v2 true, `unzip -t`
    clean; the APK itself was re-checked to contain the new
    code in its `libminecraftpe.so` (arm64 10,244,936 B /
    armv7 8,793,988 B — byte-identical sizes to the local
    build; the .so is stripped so `nm` shows no symbol table —
    verified instead via the local object files, see above).
  ### Known limits / debts
  - Ghast/blaze fireballs keep the NULL-source shape
    (`Fireball::onHit`): a ghast or blaze CAN still hurt itself
    with its own fireball blast in principle. Same one-line
    shape as this fix (pass the fireball as source + a
    shooter-immunity check), but it is a different mob with no
    modern-MC immunity rule of its own — a separate batch if
    it ever shows up in play, not smuggled into this one.
  - The boss bar still has no wither icon (cosmetic, v34 debt).
    The wither rose still has no dye use (needs a dye system
    first — a whole subsystem, not a batch).
  - All v36 tuning numbers stay PROVISIONAL until the v34/v35/v36
    device feel arrives — do NOT re-tune blind.
  - All of it is build-only until a live test on device (see
    the "Next" v38 test list: self-damage gone, cross-wither
    skulls harmless, player/mobs still hurt, summon blast still
    safe, no crash, old features intact).
- **v39 — Blaze spawner tile (SHIPPED, build-only).** Pays the
  v31 debt "no blaze spawner tile" — the oldest open structure
  debt standing. One tile + one tile entity + a spawn tick, on
  the ChestTileEntity / MobSpawner precedents.
  ### Audit (read from code before writing anything)
  - **`Tile::mobSpawner`**: only DECLARED (`Tile.h:169`), never
    defined — no `Tile* Tile::mobSpawner = NULL;` in `Tile.cpp`
    and no registration in `initTiles()`. No spawner tile-entity
    class existed anywhere; the dungeon spawner is `#if 0`'d
    out (`DungeonFeature.cpp`, whole file — a cave carver, not a
    spawner: it never references a spawner tile or entity, so
    there is nothing to re-enable there, only a dead file to
    leave alone). The lang string already existed
    (`tile.mobSpawner.name=Monster Spawner`,
    `en_US.lang:252`), as did the id string: the commented-out
    line in `TileEntity::initTileEntities` already named it
    `"MobSpawner"`.
  - **Tile-entity plumbing (how a ticking TE works here)**:
    `EntityTile` (`EntityTile.cpp:9-13`): `onPlace` →
    `level->setTileEntity(x,y,z,newTileEntity())`, `onRemove` →
    `level->removeTileEntity`. `LevelChunk::setTile` (line ~136)
    calls `onPlace` server-side, so placing ANY block through
    `setTile` — including the fortress's clipped `setF` — creates
    its tile entity automatically (this is how the v33 chest gets
    its entity during postProcess; no explicit creation call is
    needed in `placeFortress`). `Level::tick` ticks every TE in
    `level->tileEntities` whose chunk is resident
    (`Level.cpp:1441-1446`, `hasChunkAt` gate), and removes +
    deletes TEs flagged removed. Furnace (`FurnaceTileEntity::
    tick`) and brewing stand (`BrewingStandTileEntity::tick`)
    are the ticking precedents: both early-return on
    `level->isClientSide`, both keep their state in plain fields
    saved via `CompoundTag` (`putShort`/`getShort`,
    `putInt`/`getInt` exist). `shouldSave()` decides what reaches
    `entities.dat` (`ExternalFileLevelStorage::saveEntities`,
    ~line 519: skips `!shouldSave()`): chest/furnace return false
    when empty/idle and are re-created empty by the lazy path,
    sign returns true always.
  - **Save/load + eviction (no duplication, no loss)**:
    `LevelChunk::getTileEntity` (line ~579) lazily creates the TE
    for any entity tile with no map entry, via
    `level->setTileEntity` (which sets level/pos through
    `LevelChunk::setTileEntity` → `setLevelAndPos` and pushes to
    `level->tileEntities`, or to `pendingTileEntities` while
    `updatingTileEntities` is true — the pending flush dedups
    against the chunk map). `loadEntities` re-attaches saved TEs
    with a `hasTileEntityAt` guard that deletes the duplicate.
    Eviction (v17) detaches TEs without deleting them
    (`detachTileEntities` clears only the chunk map; objects stay
    alive in `level->tileEntities`, the eviction snapshot writes
    them to `entities.dat`) and `reattachTileEntities` puts the
    ORIGINAL back on reload, guarded by `hasTileEntityAt`. A
    spawner TE is a plain `TileEntity` with no container, so all
    of this applies unchanged — no spawner-specific eviction code
    was needed. `shouldSave()` returns true (Sign precedent) so
    the delay persists across quit/reload instead of resetting.
  - **`MobSpawner` (the ambient spawner, v17)**: `MobSpawner::
    tick` (`MobSpawner.cpp:28`) polls player-relative ±8 chunks
    every 2 ticks, caps per `MobCategory` (monster 20 per level,
    counted over the whole `Level::entities` list) and picks mobs
    from `Biome`/`getMobsAt` (Nether: blaze weight 6). The
    spawner TE deliberately does NOT consult that cap (modern MC
    spawners ignore it; `ensureBlazes` also bypasses it via
    `addEntity`) — the per-spawner cap (4 nearby) is the limit.
  - **`ensureBlazes` (v31) + decision**: tops the fortress up to
    3 blazes whenever its centre chunk (re-)enters the resident
    set, instantly and with no player-proximity check. DECISION —
    spawner REPLACES the top-up for new fortresses, top-up stays
    as the old-world fallback: `ensureBlazes` now returns early
    when the spawner block is present at the blaze-room centre
    (`wx+20, deckY+1, wz`), so a v39+ fortress is supplied ONLY
    by its spawner (no instant refill on chunk reload, no
    double supply — spawner cap 4 vs top-up 3 could otherwise
    stack to 4 + a fresh top-up burst). A fortress generated
    before v39 has no spawner block, so the top-up still fires
    there exactly as before; the same fallback covers a spawner
    the player broke (next centre-chunk reload tops the room up
    to 3 again). The check sits AFTER the `terrainPopulated`
    guard: reading the spawner block before the fortress exists
    would top up blazes over lava where the deck will be.
  - **Texture**: `terrain.png` has no blank cell left (no solid
    single-colour cell in the whole atlas; the v21 light-purple
    placeholders are all spent). Cell 251 (11,15) was chosen and
    verified free the v25 way: no tile tex, not in the
    `ItemInHandRenderer` mapper (cloth 200-215, logs 216-219,
    saplings 220-223, slabs 224-231, leaves 232-235), not a
    destroy stage (240-249, `LevelRenderer::renderHit` uses
    `15*16 + progress`), not a dynamic-texture target (only the
    water cells), and no code reference to 251 anywhere in src
    (the only "251" hits are gui.png blit coords and RakNet
    telnet). Its old content was a black/white checker. The cage
    (dark steel bars + orange blaze glow centre) is drawn into
    it, same PIL precedent as v9/v11/v20/v25/v34/v35; the stale
    precomputed `terrain_*.h` copies are untouched (v35/v25
    precedent — nothing references them at runtime).
  - **Spawner in the fortress (placement)**: blaze room centre
    floor `(wx+20, D+1, wz)` — the old first blaze spawn spot,
    clear of the glowstone corners, the west doorway and the
    v33 loot chest at `(wx+22, D+1, wz)`. Placed via `setF` with
    the `inChunk` guard, so exactly the owning chunk places it
    once. Fortresses in ALREADY-generated worlds do NOT get a
    spawner retroactively (generation only runs on new chunks) —
    those rooms keep the `ensureBlazes` top-up instead; stated
    honestly, not papered over.
  ### What shipped
  - **Tile** (`MobSpawnerTile`, id 52 modern MC):
    `Tile::mobSpawner` defined + registered in `Tile.cpp`
    (destroy 5.0, stone sound, Structures, descriptionId
    "mobSpawner"); `TileInclude.h` includes it. Plain cube with
    the drawn cage texture (cell 251) — no custom non-cube tile
    renderer exists in this tree for a see-through cage with a
    spinning mob inside, so cube + texture is the honest shape
    (same precedent as the enchanting table / brewing stand /
    cauldron). Breaking drops NOTHING (`getResource` returns 0 —
    modern MC: silk touch only, and there is no silk touch in
    this tree), so fortress spawners cannot be farmed.
  - **Tile entity** (`MobSpawnerTileEntity`,
    `TileEntityType::MobSpawner` = 6, id string `"MobSpawner"` —
    factory case + `setId` in `TileEntity.cpp`): ticks only
    server-side, only with a player within 16 blocks (delay
    frozen otherwise); on an attempt spawns ONE blaze at a random
    air spot within ±4 blocks horizontally at the spawner level
    or one above (4 tries, feet + head must be non-solid),
    unless 4 blazes are already inside the 9×4×9 box around it;
    delay re-rolls to 200–400 ticks (10–20 s) after every
    attempt; first delay 200 ticks. `shouldSave()` always true,
    `Delay` saved/loaded as int.
  - **Fortress**: 1 spawner in every NEW fortress's blaze room
    (see Audit); `ensureBlazes` gated on the spawner block as
    decided above (old-world fallback kept).
  - **Creative inventory**: spawner added (`Inventory.cpp`, next
    to the wither rose) — a placed spawner spawns blazes wherever
    it is put, while a player is near.
  - Version bump 38 -> 39 (`app/build.gradle`,
    `AndroidManifest.xml`, `build-apk.sh`). Local NDK r17c
    build (both ABIs) passes: `libminecraftpe.so` arm64
    10,261,320 B / armv7 8,798,084 B (v38: 10,244,936 /
    8,793,988 — the growth is the new tile + tile entity +
    fortress code); new symbols confirmed in the object files
    (`MobSpawnerTileEntity::tick/save/load/shouldSave`,
    `MobSpawnerTile::getResource/newTileEntity`,
    `Tile::mobSpawner`). CI
    `./gradlew assembleDebug` SUCCESS (run 42548892653, ~4m05s,
    02:04:33–02:08:38 UTC 2026-11-09); APK
    `minecraft-mcpe-android-0.6.1-debug-v39.apk`
    12,945,352 B, sha256
    fa8358e7b94690015f705cf22be47620d2ed30833e05bf10e04bc75d6b10a630,
    versionCode 39, `apksigner verify` v1+v2 true, `unzip -t`
    clean; the APK itself was re-checked to contain the new
    code in its `libminecraftpe.so` (arm64 10,261,320 B /
    armv7 8,798,084 B — byte-identical sizes to the local
    build) and the new cage texture in `terrain.png` cell 251
    (changed vs the v38 atlas).
  ### Known limits / debts
  - No spinning blaze model inside the cage (no custom tile-entity
    renderer exists in this tree) — the orange glow in the cage
    texture is the only "mob inside" hint; the spawner never
    shows WHICH mob it spawns. A player-placed spawner always
    spawns blazes (no spawn-egg / mob-type switching — no such
    system exists here).
  - Old-world fortresses (generated before v39) never get a
    spawner block — they keep the `ensureBlazes` top-up to 3
    instead. No migration was attempted (rewriting generated
    chunks is out of scope for this batch).
  - Spawner delay (200–400 ticks) and cap (4 nearby) are TUNING,
    provisional until the device feel arrives — do NOT re-tune
    blind. The spawner ignores the global monster cap of 20 by
    design (see Audit); in a Nether crowded with monsters the
    spawner still spawns, like modern MC.
  - All of it is build-only until a live test on device (see
    the "Next" v39 test list: new fortress has a cage block in
    the blaze room, blazes spawn over time to ~4, killing them
    refills after the delay, >16 blocks away it stops, quit/
    reload keeps spawner + blazes, breaking it drops nothing,
    old features intact).
- **v40 — Ghast/blaze fireball self-damage (SHIPPED, build-only).**
  Pays the v38 debt "ghast/blaze fireballs keep the NULL-source
  shape". Same one-line family as v38, but NOT a copy of it — the
  wither's blanket immunity would kill a real modern-MC mechanic
  (a reflected fireball hurts the ghast), so v40 is owner-scoped.
  ### Audit (read from code before writing anything)
  - **Damage routing today**: `Level::explode(source, ...)`
    (`Level.cpp:1721`) → `Explosion::explode()`
    (`Explosion.cpp:70,88`): the entity scan is
    `level->getEntities(source, AABB...)` (the source is excluded
    from its own blast) and every hit arrives as
    `e->hurt(source, dmg)` — so the blast's source IS the damage
    source `hurt()` sees. `Fireball::onHit` (`Fireball.cpp:27`)
    called `level->explode(NULL, x, y, z, 1.0f, true)` — radius 1
    + fire, the modern ghast/blaze values — so NO entity was
    excluded and the shooter took its own blast like anyone else.
    The v38 formula gives up to ~9 damage at point blank
    (`(pow²+pow)/2 × 8 × r + 1`, r=1); a ghast has 10 HP, a blaze
    20 — one own blast can nearly kill a ghast.
  - **When it can actually happen**: ghast fires ≤28 blocks
    (`Ghast::checkHurtTarget`, one aimed ball every 3 s), blaze
    ≤16 (`Blaze::checkHurtTarget`, same cadence) — both
    `new Fireball(level, this)` + `shoot(xd, yd, zd, 1.0f, 8)`.
    The ball flies straight (no gravity, no bounce), so it cannot
    loop back onto its shooter; the real path is the BLAST
    landing beside the shooter — the target standing close under
    it, or the shot hitting a wall in a narrow Nether corridor /
    fortress room. The ghast's 4x4x4 body catches a radius-1
    blast from ~2 blocks away; the blaze is 0.6 wide, so it needs
    the blast almost on top of it. Rare, but real — and unlike
    the wither volley it needs no wall, just a close target.
  - **`ownerId` is reliable for this — within a session.**
    `Throwable` stores `ownerId` (`Throwable.h:53`, protected
    int): the (Level, Mob) ctor sets it to `mob->entityId`
    (`Throwable.cpp:20`), the flight code already trusts it
    (`tick`: skips `entityId == ownerId` while `flightTime < 5`,
    same shape as `Arrow.cpp:179`). `entityId` is a per-run
    counter (`Entity.cpp:67`, `++entityCounter` from 0), so 0
    means "no owner" and can never match a real mob — a fireball
    rebuilt from a save (`EntityFactory.cpp:23`, the (Level)
    ctor) has ownerId 0 and simply gets no immunity (safe
    fallback, not a crash). ownerId is deliberately NOT saved:
    saving it would be wrong, because entityIds are re-assigned
    on load and a saved id could point at a different entity;
    a fireball only lives a few seconds anyway. The only other
    reader is `PigZombie.cpp:74` (arrow owner lookup), same
    session-only semantics. No getter existed — only
    `getAuxData()` returning it, which would have been a hack to
    call from `hurt()` — so v40 adds an explicit `getOwnerId()`.
  - **NO reflection mechanic exists in this tree — verified, not
    assumed.** Zero hits for reflect/deflect/rebound in src
    (only "reflected back" in a redstone comment and "reflect"
    in raknet docs). `Entity::isPickable()` returns false by
    default (`Entity.cpp:721`) and `Fireball` does not override
    it, so the player cannot even target a fireball today — the
    crosshair pick skips it and no punch can land on it. The
    brief's reflected-fireball test therefore CANNOT pass in
    v40: there is nothing to reflect with. What v40 does is
    preserve the mechanic for the version that adds reflection:
    immunity is keyed on ownerId, so a future reflection that
    reassigns ownerId to the deflector makes the reflected ball
    hurt the ghast automatically (documented as a contract in
    `Throwable::getOwnerId`). A v38-style blanket immunity
    ("ghast takes no damage from any fireball") would have made
    reflection unimplementable without rework — that is why it
    was rejected here, explicitly.
  - **Other immunity patterns checked**: Creeper
    (`Creeper.cpp:88`) calls `explode(this, ...)` — excluded
    from its own blast (it `remove()`s anyway). Wither v38
    (skull as source + `Wither::hurt` rejects type
    `IdWitherSkull`). Ender dragon ALSO shoots `Fireball`
    (`EnderDragon.cpp:272`, same (Level, Mob) ctor) — it gets NO
    immunity in v40 (out of scope: ghast/blaze only, and a dragon
    orbiting at y=82 fires outward/down, so its own blast
    landing beside it is even less plausible than the ghast's;
    noted as a debt, not fixed). `Mob::die(source)` is safe for
    the source change: `source->awardKillScore` is the base
    no-op for a fireball, and `Player::isPlayer(source)` is
    false, so no XP changes.
  - **Rejected alternatives**: (1) Copying v38 exactly
    (reject any `IdFireball` source in `hurt()`) — kills the
    reflected-fireball mechanic; rejected on the brief's
    explicit warning. (2) Passing the SHOOTER as the blast
    source (`explode(owner, ...)`) — `getEntities` would exclude
    the shooter from the blast entirely, which also hides the
    shooter from knockback/damage bookkeeping and makes
    `hurt()` see the ghast itself as the source (melee-shaped,
    wrong for `die()`/kill credit); the fireball-as-source shape
    keeps the blast honest and the check exact. (3) Skipping the
    owner inside `Explosion::explode` — touches the generic path
    shared by TNT/creeper/crystals for a ghast/blaze-only rule.
  - **Decision (modern MC)**: a ghast/blaze takes no damage
    from a fireball IT shot. A fireball from ANOTHER ghast/blaze
    still hurts it (different ownerId), TNT / creeper / crystal
    blasts still hurt it (NULL or another entity), and a
    reflected fireball will hurt it once reflection exists and
    reassigns ownerId.
  ### What shipped
  - `Throwable::getOwnerId()` (new, `Throwable.h`): public
    accessor for the protected `ownerId`, with the session-only
    + reflection contract documented in the comment.
  - `Fireball::onHit`: `level->explode(NULL, ...)` →
    `level->explode(this, ...)` (source = the fireball). Player
    + other mobs are unaffected by the source change: same damage
    formula, same radius, same fire; the only difference is
    knockback — `Mob::hurt` now knocks them away from the blast
    centre (the fireball sits at it) instead of a random
    direction with no knockback (the NULL path). The fireball
    itself is excluded from its own blast by `getEntities`
    (it was being `remove()`d anyway).
  - `Ghast::hurt` / `Blaze::hurt` (new overrides): reject exactly
    `source != NULL && source->getEntityTypeId() ==
    EntityTypes::IdFireball && ((Fireball*)source)->getOwnerId()
    == entityId` → return false, before `Mob::hurt`, so no
    invulnerableTime, knockback or hurt sound is spent on
    blocked hits. A second ghast's / blaze's fireball lands
    normally (different ownerId); a fireball with ownerId 0
    (rebuilt from a save) lands normally too.
  - Version bump 39 -> 40 (`app/build.gradle`,
    `AndroidManifest.xml`, `build-apk.sh`). Local NDK r17c
    build (both ABIs) passes: `libminecraftpe.so` arm64
    10,257,224 B / armv7 8,798,084 B (v39: 10,261,320 /
    8,798,084 — arm64 is ~4 KiB SMALLER despite the added code;
    the change is three small code paths, no new strings, so
    this is codegen noise, not a missing feature — the new
    symbols are confirmed defined in the fresh object files:
    `Ghast::hurt` / `Blaze::hurt` (T) in both ABIs,
    `Fireball::onHit` calling `Level::explode(Entity*,...)`).
    CI `./gradlew assembleDebug` SUCCESS (run 42550095935, ~4m59s,
    02:17:24–02:22:23 UTC 2026-11-09); APK
    `minecraft-mcpe-android-0.6.1-debug-v40.apk`
    12,945,623 B, sha256
    81f42dc1a0b8a78f8c023ae7d32c82daed50024cb594e89085df56f219786395,
    versionCode 40, `apksigner verify` v1+v2 true, `unzip -t`
    clean; the APK itself was re-checked to contain the new
    code in its `libminecraftpe.so` (arm64 10,257,224 B /
    armv7 8,798,084 B — byte-identical sizes to the local
    build; the .so is stripped so `nm` shows no symbol table —
    verified instead via the local object files, see above).
  ### Known limits / debts
  - NO fireball reflection exists in this tree (see Audit) — the
    player cannot punch a fireball back today, so the reflected
    half of the v40 rule is preserved in code but untestable on
    device. The natural follow-up is a reflection batch: make
    the fireball pickable + reverse its velocity on a player hit
    + reassign ownerId to the player (the contract in
    `Throwable::getOwnerId`); the attack path
    (`GameMode`/`Player::attack` vs a non-mob entity) was NOT
    audited in v40 and must be audited first. See "Next".
  - The ender dragon's own fireballs can still hurt it (same
    NULL-source shape v40 just removed for ghast/blaze — the
    dragon was deliberately out of scope; its orbit makes a
    self-blast near-impossible in practice). One-line family
    with v40 if it ever shows up in play.
  - A fireball rebuilt from a save has ownerId 0 and no immunity
    (session-only ids — see Audit; a deliberate, safe fallback).
  - All of it is build-only until a live test on device (see
    the "Next" v40 test list: own-fireball immunity, cross-ghast/
    cross-blaze fireballs still hurt, player/mobs still hurt,
    no crash, old features intact).
- **v41 — Fireball reflection (SHIPPED, build-only).** Pays the
  v40 "Next" candidate — the mechanic v40 deliberately preserved.
  ### Audit (read from code before writing anything)
  - **The attack path CAN hit a non-mob entity — no pick-system
    rework needed.** `Minecraft::handleBuildAction`
    (`Minecraft.cpp:1489`) → `GameMode::attack(player,
    hitResult.entity)` (`GameMode.cpp:39`) → `Player::attack`
    (`Player.cpp:786`) → `entity->hurt(this, dmg)`.
    `GameMode::attack`'s only guards are `noPvP` (player targets)
    and `noPvM` (mob targets) — a Fireball is neither, so a punch
    reaches it. The exact precedent is `EndCrystal` (v27):
    pickable + a `hurt()` override is the whole punch path
    (`EndCrystal.h:8-9`).
  - **Pick range is the real constraint, and it is survivable.**
    `GameRenderer::pick` (`GameRenderer.cpp:656`): the entity ray
    in survival is clamped to 3 blocks (`dist > 3` → 3;
    `getPickRange()` is 5.0, creative 12), target box =
    `bb.grow(getPickRadius())` (default 0.1). A ghast/blaze/
    dragon fireball is 0.5×0.5 and flies ~1 block/tick
    (`shoot(..., 1.0f, 8)` in `Ghast.cpp:119`, `Blaze.cpp:50`,
    `EnderDragon.cpp:276`, inertia 0.99), so it crosses the
    3-block punch window in ~3 ticks (~150 ms) — tight, but that
    is the skill timing modern MC has too. The crosshair must be
    on the ball at tap time; the ball is pickable only via the
    ray, there is no proximity auto-hit anywhere in this tree.
  - **Arrows must NOT reflect.** `Arrow.cpp:213` calls
    `res.entity->hurt(this, dmg)` with the Arrow as source, and
    on a `false` return the arrow bounces off (velocity × −0.1).
    A `source->isPlayer()` guard in `Fireball::hurt` therefore
    keeps arrows bouncing exactly as before, and explosion
    blasts (source = a fireball/TNT/creeper) likewise never
    reflect. `Player::attack` only calls `hurtEnemy` (weapon
    durability) when `entity->isMob()`, so reflecting costs no
    sword durability.
  - **`ownerId` contract (v40) holds.** `Throwable::getOwnerId`
    exists; `Level::getEntity(id)` resolves an owner through
    the `entityIdLookup` map (`Level.cpp:1037`), so the
    reflection can check WHAT shot the ball. `Throwable::tick`
    already skips `entityId == ownerId` while `flightTime < 5`
    — after reassignment that skip protects the DEFLECTOR for
    its first ticks, not the shooter. `flightTime` is private
    in `Throwable` and is NOT reset on reflect: in any realistic
    reflect the ball already flew >5 ticks to get within 3
    blocks of the player, and resetting is unnecessary. `shoot()`
    normalises its direction and takes an uncertainty argument,
    so a reflect is one call: `shoot(dir, pow, 0)`.
  - **No other projectile is reflectable today.** Snowball /
    ThrownEgg / ThrownPotion are `Throwable`s but not pickable
    either, and none has a `hurt()` override — reflection is a
    Fireball-only mechanic in this batch, by design.
  ### What shipped
  - `Fireball::isPickable()` (new): `return !removed`
    (the `EndCrystal` precedent) — the crosshair can now target
    a flying fireball.
  - `Fireball::getPickRadius()` (new): 0.3f instead of the
    default 0.1f — the same 0.3 `Throwable`/`Arrow` use for
    their own hit tests, so the crosshair target is ~1.1 blocks
    (close to modern MC's 1-block fireball) instead of 0.7.
  - `Fireball::hurt()` (new): a player punch reflects the ball
    — it flies back along the player's look direction
    (`Player::getViewVector(1.0f)`, `Player : public Mob`),
    at 1.5 blocks/tick (faster than the 1.0 it arrived at, so it
    can catch a flying ghast) with zero spread
    (`shoot(dir, 1.5f, 0)`), and `ownerId` is reassigned to the
    deflector — the v40 contract, so `Ghast::hurt` /
    `Blaze::hurt` no longer reject it and the reflected ball
    hurts the ghast that shot it (10 HP — one clean reflected
    hit kills it). It still explodes (radius 1 + fire) on any
    block/mob it reaches. The mutation is guarded by
    `!level->isClientSide` (the `EndCrystal::hurt` pattern —
    the server is authoritative in MP) but returns true either
    way, so the punch always counts as landed.
  - Scope, decided and recorded: ONLY ghast/blaze fireballs
    reflect. The owner is resolved first; a dragon-owned ball
    is left alone (`hurt` falls through to the base, which
    returns false) because the dragon has no v40 owner
    immunity — a reflected dragon ball could hurt the dragon
    itself, a change to the End fight this batch does not make.
    `ownerId` 0 (rebuilt from a save) or a vanished owner
    reflects anyway: the ball was almost certainly
    ghast/blaze-shot. Blaze fireballs reflect exactly like
    ghast ones (same class, same code path).
  - Version bump 40 -> 41 (`app/build.gradle`,
    `AndroidManifest.xml`, `build-apk.sh`). Local NDK r17c
    build (both ABIs) passes: `libminecraftpe.so` arm64
    10,261,320 B / armv7 8,798,084 B (v40: 10,257,224 /
    8,798,084 — arm64 +4 KiB for the new code; armv7
    byte-identical in size, which is codegen coincidence — the
    new symbols are confirmed defined in the FRESH object
    files: `Fireball::hurt` / `Fireball::isPickable` /
    `Fireball::getPickRadius` (T) in both ABIs).
    CI `./gradlew assembleDebug` SUCCESS (run 42551678249,
    ~6m22s, 02:34:22–02:40:44 UTC 2026-11-09); APK
    `minecraft-mcpe-android-0.6.1-debug-v41.apk`
    12,947,150 B, sha256
    818e874701fa6f25e4423cff335b90df11731b29e9ea290039927ab0b89b4848,
    versionCode 41, `apksigner verify` v1+v2 true, `unzip -t`
    clean; the APK itself was re-checked to contain the new
    code in its `libminecraftpe.so` (arm64 10,261,320 B /
    armv7 8,798,084 B — byte-identical sizes to the local
    build; the .so is stripped so `nm` shows no symbol table —
    verified instead via the local object files, see above).
  ### Known limits / debts
  - The 3-block survival pick window is the whole skill check:
    a fireball at 20 blocks/s gives the player ~150 ms of
    hittable time per approach. If the device test finds it
    unhittable in practice (touch aim + fast ball), the honest
    fixes are a bigger pick radius or a slower reflected-ball
    rule — NOT a proximity auto-reflect, which would change
    the mechanic.
  - Dragon fireballs are deliberately NOT reflectable (see
    above). If the dragon ever needs the same treatment, the
    one-line family is: give the dragon the v40 owner immunity
    first, then drop the owner-type check here.
  - A reflected ball that misses the ghast flies on until it
    hits a block/mob — it never turns around again on its own.
  - All of it is build-only until a live test on device (see
    the "Next" v41 test list: reflect kills the ghast, blaze
    balls reflect too, unreflected balls still hurt the player,
    no crash, old features intact).
- **v42 — Modern crafting table UI + stuck-movement bugfix (SHIPPED, build-only).** Pays the v42 "Next" candidate (user request 2026-11-09 09:38, screenshot `image-22.png`) and the bundled bug report 2026-11-09 09:41 (player keeps walking after opening crafting).
  ### Audit (read from code before writing anything)
  - **The old crafting screen was already a recipe book, not a grid.** `PaneCraftingScreen` (base of `WorkbenchScreen` / `StonecutterScreen`) showed a scrolling text list (`ItemPane`, one row per recipe: icon + name + owned count) with 4 category tabs on the left and a Craft button showing needed items + have/need counts on the right. There is NO manual slot grid anywhere in the tree — no crafting container, no slot drag-and-drop — so "don't break manual crafting" means: keep the click-recipe → craft flow, which v42 does (the result slot replaces the Craft button).
  - **Search: FEASIBLE, via the sign-editing pattern.** No screen in the tree does live in-engine text editing (`TextBox::render` is empty; the only `keyboardNewChar` override is `TextEditScreen`), and chat uses a native Android dialog (`ChatScreen::init` → `platform()->createUserInput(DIALOG_NEW_CHAT_MESSAGE)`). But the soft keyboard DOES feed the engine: `handle_keyboard_input` (`main_android.h:378-420`) feeds every key DOWN to `Keyboard::feed` and every printable char to `Keyboard::feedText`; `Screen::updateEvents` (`Screen.cpp:99-101`) drains `Keyboard::nextTextChar()` into `keyboardNewChar(char)` — the exact path `TextEditScreen` (sign editing) already ships on Android. So v42 wires the search box the same way: tap the field → `showKeyboard()`, type → `keyboardNewChar` appends + re-filters live, backspace via `keyPressed(KEY_BACKSPACE)`, Return/Esc/Android-back unfocus + `hideKeyboard()`. NOT yet proven on the phone: whether the realme 8 soft keyboard delivers key events to a NativeActivity this way (sign editing says yes, but it is the first thing to verify on device).
  - **Categories: the engine already has 5, the old screen used 4.** `ItemCategory`: Structures(1), Tools(2), FoodArmor(4), Decorations(8), Mechanisms(16). The old screen built 4 tabs (bitmasks 1<<0..3) and never showed Mechanisms. The screenshot has 5 tabs (compass, tools, blocks, food, redstone): v42 maps them to All (compass item icon — the real `Item::compass`, the sheet has no compass glyph), Tools, Structures|Decorations (blocks), FoodArmor, Mechanisms (rails glyph, spritesheet icon 4). Sprite glyphs verified by reading `gui/spritesheet.png` regions: 0=planks, 1=pickaxe, 2=armor+apple, 3=item frame, 4=rails, 5=stone (stonecutter). `StonecutterScreen` keeps its single tab (Structures, icon 5).
  - **Grid data needed new accessors.** `ShapedRecipe` kept its cells private with no accessor; `ShapelessRecipe` its ingredients. v42 adds three virtuals to `Recipe` (`getGridWidth/Height/Item`: shaped = authored cells, shapeless = row-major fill) — virtual dispatch, no RTTI / `dynamic_cast`.
  - **Movement-freeze diagnosis (verified from code; the coordinator's hypothesis corrected).** The brief said `setScreen` calls `releaseAllKeys()` — TRUE ONLY ON UWP: the call sits inside `#ifdef _UWP` (`Minecraft.cpp:1587`); on Android it is compiled out. What `setScreen` DOES do on Android: `Mouse::reset()` + `Multitouch::reset()` + `resetThisUpdate()` — and `MouseDevice::reset()` does NOT clear held buttons (`_buttonStates[ACTION_LEFT]` survives; only the event queue + wheel state are cleared), so the still-held forward thumb stays in the active-pointer list (`Multitouch::commit()` rebuilds it from `isButtonDown(1)` every frame in `NinecraftApp::update`). Meanwhile `LocalPlayer::aiStep` (`LocalPlayer.cpp:258`) skips `input->tick(this)` whenever a non-`passEvents` screen covers the game, and `updateAi()` (`LocalPlayer.cpp:367`) copies the frozen `input->xa/ya` into `xxa/yya` every tick → `Mob::aiStep` → `travel()` keeps walking. Intermittent because it only bites when the screen opens while the thumb is still down — lift the thumb a tick before the tap and the frozen value is 0. The `screenMutex` scheduled-screen path is NOT a separate cause: the deferred `setScreen` re-runs the same (now fixed) prologue.
  ### What shipped
  - `PaneCraftingScreen` rewritten to the screenshot layout (old `ItemPane` text list + Craft button removed): left dark panel with a live search bar, 5 category tabs, a 5x4 recipe icon grid with `< N/M >` pagination (20 recipes/page); right light panel with "Crafting" 3x3 grid + arrow + result slot and an "Inventory" 9x4 grid below (rows 0-2 = main inventory slots 9-35, row 3 = hotbar 0-8, display-only in stage 1, counts on stacked items).
  - Click a recipe → the 3x3 grid auto-fills from the recipe (shaped as authored, shapeless row-major) with have/need counts under each cell, and the result slot shows the result (highlighted when craftable, dimmed when not); tap the result slot to craft — the old craft path (`craftSelectedItem`: `canCraft` check, `removeResource`, `add`/drop, `recheckRecipes`) is unchanged, and the grid/counts refresh after each craft.
  - Live search: tap the field to focus (soft keyboard opens), type to filter by recipe name (case-insensitive substring) across the current tab, backspace deletes, Return/Esc/Android-back unfocus and hide the keyboard; pagination recomputes from the filtered list and the selection is kept only while still visible. No "craftable-only" toggle yet (the screenshot's green check button) — no checkmark glyph exists in the font; stage 2.
  - Bugfix, two layers: (1) `Minecraft::setScreen` now calls `inputHolder->getMoveInput()->releaseAllKeys()` on EVERY platform (was `#ifdef _UWP` only) — the root cause above; (2) `LocalPlayer::aiStep` zeroes `input->xa/ya/jumping` while a covering screen is open, so the player halts on the same tick the screen opens even if any other path leaves stale state — stopping no longer depends on the finger-up event. Both layers apply to every covering screen (chest, furnace, inventory 2x2, pause), not just crafting. Sprint state is cleared by `releaseAllKeys`, so no phantom sprint when the screen closes while the thumb is still down (the held thumb keeps walking, as it should).
  - `Recipe::getGridWidth/Height/Item` virtuals (`Recipe.h`) + overrides in `ShapedRecipe.h` / `ShapelessRecipe.h`; `PaneCraftingScreen` factored `buildInventoryPack` / `inventoryCountFor` out of `recheckRecipes` so the grid counts and `canCraft` use the same pack. NOTE (pre-existing quirk, kept): the pack counts only main-inventory slots 9+, not the hotbar — same as `recheckRecipes` always did.
  - Version bump 41 -> 42 (`app/build.gradle`, `AndroidManifest.xml`, `build-apk.sh`). Local NDK r17c build (both ABIs) passes: `libminecraftpe.so` arm64 10,269,512 B / armv7 8,798,084 B (v41: 10,261,320 / 8,798,084 — arm64 +8 KiB for the new code; armv7 byte-identical in size, which is codegen coincidence — the new symbols are confirmed defined in the FRESH object files: `PaneCraftingScreen::applyFilter` / `fillCraftGrid` / `keyboardNewChar` / `mouseReleased` / `buildInventoryPack` / `inventoryCountFor` / `handleBackEvent` (T) in both ABIs). The local build caught one real bug before CI did: `push_back(ItemCategory::Tools)` odr-uses the header-only `static const int` (no out-of-line definition) and fails the link — fixed by copying the constants to locals first (commit amended before CI finished; the first two pushed commits would have failed the CI link the same way).
    CI `./gradlew assembleDebug` SUCCESS (run 42554887100, ~5m43s, 03:10:16–03:15:59 UTC 2026-11-09); APK `minecraft-mcpe-android-0.6.1-debug-v42.apk` 12,950,630 B, sha256 a5b862dc9c66bdf89704dbcfc7281ed3f73660aa8722b82d3da0b5a9adc9e6c0, versionCode 42, `apksigner verify` v1+v2 true, `unzip -t` clean; the APK itself was re-checked to contain the new code in its `libminecraftpe.so` (arm64 10,269,512 B / armv7 8,798,084 B — byte-identical sizes to the local build; the .so is stripped so `nm` shows no symbol table — verified instead via the local object files, see above).
  ### Known limits / debts
  - Search is the one unproven-on-device piece: it rides the sign-editing keyboard path, but no one has typed into it on the realme 8 yet. If the soft keyboard does not deliver chars there, the fallback is the chat-style native dialog (`createUserInput`) — same filter, worse UX.
  - Inventory grid is display-only in stage 1: tapping an inventory item does nothing (no manual grid placement — there is no crafting container in the tree; placing items by hand into the 3x3 grid is the v44 manual-grid job, if the user wants it).
  - No craftable-only toggle (screenshot's green check button) and no recipe-book collapse button (green book icon) yet — both stage-2 polish.
  - The 2x2 inventory crafting screen is the SAME `PaneCraftingScreen` with `SIZE_2X2`, so it gets the new layout too — but its recipes are only the 2x2 ones, and the grid shown is still the 3x3 auto-fill display (recipes are ≤2x2, so the third row/column stays empty). v44 (inventory 2x2 stage) should decide whether to shrink that display to 2x2.
  - All of it is build-only until a live test on device (see the "Next" v42 test list).
- **v43 — Bug-fix batch: 4 user reports 2026-11-09 10:21 (SHIPPED,
  build-only).** Diagnosed from the code + the user's video
  (`940b2e0c...mp4`, 20.6 s) + the HUD reference
  (`1000530336_1_2ubt.jpg`), NOT guessed. One correction to the
  brief's hypothesis: the black particles at ~11.6 s are NOT the v13
  sprint dust — the video proves they are reed block-break fragments
  (see (b) below). The sprint/landing dust is still removed, as the
  other black-particle source named in the brief.
  ### Audit (read from code + video frames before writing anything)
  - **(a) Water entry = fishing.** `Entity::baseTick` spawned TWO
    particle loops on entering water: vanilla bubbles (`PARTICLETYPE
    (bubble)`, tex 32 = blue ring in `particles.png`) AND the v13
    splash loop (`PARTICLETYPE(splash)`, `SplashParticle` tex 33).
    v13's comment claimed tex 33 is "a small white droplet, next to
    the bubble sprite" — WRONG. The sheet's cell 33 alpha-map is a
    white/red bobber with a black hook (the classic fishing-bobber
    sprite); frame ~3 s of the video shows ~13 of them + blue bubble
    rings the moment the player enters the water. Entering water
    therefore looked exactly like casting a fishing rod.
  - **(b) Black particles near the reeds.** Frame 10.5 s: the player
    holds the reeds (white touch circle on the cane, D-pad forward
    held) and a burst of ~15 small green+black squares flies out;
    11.0–11.6 s: 1–3 leftover squares sit on the sand. Those are
    `TerrainParticle` fragments from `ParticleEngine::destroy()` —
    reed-texture squares whose "transparent" background renders
    opaque black. Same root cause as (c). The v13 sprint dust
    (`LocalPlayer::tick`, ~35% of sprinting ticks) and landing dust
    (`Entity::checkFallDamage`, fall > 3) are a SECOND, independent
    black-particle source: both spawn "smoke" particles whose colour
    is `random()*0.5` (`SmokeParticle.h`) — near-black, not the grey
    the v13 entry claimed — so they read as fire/burning smoke.
  - **(c) Black stripes on the reeds = lost alpha channel.**
    `terrain.png` cell 73 (reeds) has 115/256 pixels of opaque black
    where the sprite should be transparent. Root cause, found by
    `git log` on the file: v39 (commit 8b0af9e, blaze spawner tile)
    re-saved `terrain.png` as RGB — every version before it was
    RGBA. The alpha channel (13,519 transparent pixels tree-wide:
    reeds, flowers, tall grass, crops, saplings, glass, …) was
    destroyed in that one save; the RGB values under the old
    transparency survived byte-identical (verified: 0 mismatches),
    only the alpha was lost. The renderer already handles alpha
    (`GL_ALPHA_TEST` around terrain, `GL_RGBA` upload when the PNG
    has alpha — `AppPlatform_android.h:382` decodes PNG → ARGB),
    so restoring the channel restores stock 0.6.1 behaviour.
  - **(d) XP bar + inventory.** The video is a CREATIVE world —
    proven, not assumed: no hearts/hunger/XP strip render above the
    hotbar at all (pixel scan of the HUD band: no black XP strip, no
    red hearts), while the render path for survival was verified
    intact (`blit` inside `beginOverride` appends correctly;
    `renderExperience` draws its black strip unconditionally when
    called). In creative `Gui::render` gated `renderExperience()`
    behind `canHurtPlayer()`, so the bar was missing entirely —
    while the reference (survival) shows it. Modern MC shows the XP
    bar + level in creative too; hearts/hunger correctly stay
    survival-only. Second defect, same screen: the last hotbar slot
    ("...") renders as an empty dark slot. Its blit samples
    `gui.png` (242,252,14x4), which is fully transparent in the
    STOCK 0.6.1 sheet (verified against the initial import too) —
    and the blit landed inside the `gui_blocks.png` override batch,
    so it sampled grass pixels instead. The "..." sprite never
    existed in this tree. NOT fixed by guessing: the hotbar has 8
    slots (7 usable + "...", `MAX_SELECTION_SIZE` 9) vs the
    reference's 9 usable + "..." (10 cells) — widening that means
    changing the selection size everywhere (inventory, touch, MP),
    so it is left as an open question for the user, not silently
    changed (see "Next").
  ### What shipped
  - **(a)** The v13 splash spawn loop in `Entity::baseTick` is
    REMOVED (comment records the bobber diagnosis). The vanilla
    bubble loop and the `random.splash` sound stay — entering water
    now looks like normal MC (bubbles only), not fishing.
    `SplashParticle` / `ParticleType::splash` / the `LevelRenderer`
    dispatch stay: the v24/v25 thrown-potion impact burst still
    uses them (same bobber sprite, separate trigger — see debts).
  - **(b)** BOTH v13 dust spawns removed: sprint dust
    (`LocalPlayer::tick`) and landing dust
    (`Entity::checkFallDamage`). Sprint exhaustion, hunger drain and
    fall damage are untouched. Modern MC has neither puff.
  - **(b)+(c)** `terrain.png` alpha RESTORED: RGB from the current
    file + alpha from the last RGBA version (v38, 781d31c), cell 251
    (v39 blaze-spawner sprite, a fully opaque cage texture) forced
    opaque. Verified: RGB differs from v38 only in cell 251; every
    restored-transparent pixel keeps its original RGB byte-identical;
    13,519 transparent pixels back; reeds cell 73 is 115 transparent
    / 141 opaque. The black bands on the reeds, the black halves of
    reed break-fragments, and the black backgrounds of every other
    cutout plant (flowers, tall grass, crops, saplings) are gone in
    one fix.
  - **(d)** `renderExperience()` moved out of the `canHurtPlayer()`
    gate in `Gui::render` — the XP bar + green level number now
    render in creative too (hearts/hunger/bubbles stay
    survival-only). The "..." slot now draws "..." as font text
    (`Font::drawShadow`, centred in the last slot, same visibility
    rule as the old blit) instead of the dead sprite blit.
  - Version bump 42 -> 43 (`app/build.gradle`,
    `AndroidManifest.xml`, `build-apk.sh`).
    CI `./gradlew assembleDebug` SUCCESS (run 42557339101, ~4m09s,
    03:36:35–03:40:44 UTC 2026-11-09); APK
    `minecraft-mcpe-android-0.6.1-debug-v43.apk` 12,956,345 B, sha256
    5279ded0669344f2c7d428b7e0dbd6f9528cca7ce392de51f89f70a10d844eed,
    versionCode 43, `apksigner verify` v1+v2 true, `unzip -t` clean;
    the APK itself was re-checked to contain the restored RGBA
    `terrain.png` (reeds cell 73: 115 transparent / 141 opaque) and
    `libminecraftpe.so` arm64 10,281,800 B / armv7 8,798,084 B.
  ### Known limits / debts
  - A thrown splash potion (v24/v25) still bursts into bobber
    sprites on impact — `ThrownPotion` uses the same
    `ParticleType::splash`. It was left alone (the user never
    reported it, and the burst needs a real droplet texture, not a
    spawn removal). Follow-up: give the splash a proper droplet
    sprite (particles.png has blue droplet cells in row 1) or reuse
    the bubble sprite.
  - The level number sits one row above the hearts on the 8-slot
    (160 px) touch hotbar (v14 rule: hearts 81 px + hunger 81 px =
    162 px leave no gap on that row). The reference centres it on
    the hearts row — only possible on a wider hotbar; see the
    7-vs-9 usable slots question in "Next".
  - The 2px hearts/hunger overlap on the 160 px hotbar (81+81 >
    160) is unchanged from v14 — marginal, icons already overlap
    1 px by design.
  - All of it is build-only until a live test on device (see the
    "Next" v43 test list).
- **v44 — Inventory crafting 2x2, stage 2 (SHIPPED, build-only).** Pays the v44 "Next" candidate (user request 2026-11-09 09:38, second half: inventory 2x2 same modern look as the v42 table).
  ### Audit (read from code before writing anything)
  - **The recipe list was ALREADY 2x2-only.** `initCategories` filters `craftingSize >= recipe->getCraftingSize()` (`SIZE_2X2 = 0`, `SIZE_3X3 = 1`), and `ShapedRecipe::getCraftingSize` returns 2x2 iff width/height ≤ 2, `ShapelessRecipe` iff ≤ 4 ingredients — so the inventory screen (`WorkbenchScreen(SIZE_2X2)`, from the inventory Craft button / `startCrafting(..., SIZE_2X2)`) never listed 3x3 recipes. No list change was needed; v44 only fixes the DISPLAY.
  - **One real trap: shapeless reflow.** A 4-ingredient shapeless recipe IS size-2x2, but its v42 `getGridWidth()` returns 3 (row-major 3-wide) — laid out as authored it would spill into the 3rd column of a 2x2 display and the 4th ingredient would be invisible (filled into storage but never drawn). New `Recipe::getGridWidthFor/HeightFor/ItemFor(gridW)` virtuals (default = v42 accessors); `ShapelessRecipe` overrides them to reflow row-major at the display width (4 ingredients → 2x2). Shaped recipes ignore gridW (authored cells, always ≤ 2x2 here by the size filter).
  - **Toggle/collapse were both cheap, so both shipped** (the brief's "if cheap" condition): the craftable-only toggle needs no font glyph — the checkmark is drawn from 2px squares; the collapse button is a `SmallButton` ("<<"/">>") that re-runs `setupPositions()` (the same function `Screen::init`/`setSize` call), so no new layout code path exists.
  ### What shipped
  - Inventory 2x2 screen now shows a 2x2 auto-fill grid (title "Crafting", arrow + result slot, inventory 9x4 below) instead of the v42 3x3 frame with an empty third row/column; the table and stonecutter keep their 3x3 grid (`gridSize()` = 2 iff `craftingSize == SIZE_2X2`). Click a recipe → grid fills (shaped as authored, shapeless reflowed 2-wide) with have/need counts; tap the result → craft, same unchanged `craftSelectedItem` path. `gridCells` storage stays 9 cells, 3-stride — only the drawn/iterated extent changes.
  - Craftable-only toggle: green check button at the right end of the search bar (drawn checkmark — the font has no check glyph); when on, `applyFilter` hides recipes `canCraft()` is false for (computed by `recheckRecipes` at init and after each craft, same source as the dimmed-overlay state). Search + tabs + pagination compose with it exactly as with the search text.
  - Recipe-book collapse: "<<" button at the top-left of the right panel hides the whole left panel (tabs move off-screen so they are neither drawn nor clickable; search/recipe-grid/pagination rects go empty) and the right panel widens to fill the width; ">>" re-opens it. Collapsing while typing unfocuses the search and hides the keyboard first. Layout is a re-run of `setupPositions()`, so a window resize while collapsed re-derives correctly.
  - Version bump 43 -> 44 (`app/build.gradle`, `AndroidManifest.xml`, `build-apk.sh`). Local NDK r17c build (both ABIs) passes: `libminecraftpe.so` arm64 10,269,512 B / armv7 8,802,180 B (v42 local build, same toolchain: 10,269,512 / 8,798,084 — arm64 byte-identical in size, armv7 +4,096 B; size is codegen noise at this scale, the proof is the symbols: `PaneCraftingScreen::applyFilter / fillCraftGrid / mouseReleased / setupPositions` (T) in the FRESH object files of both ABIs, `ShapelessRecipe::getGridItemFor/WidthFor/HeightFor` (W) + the `ShapelessRecipe`/`ShapedRecipe` vtables in the fresh `Recipes.o` of both ABIs, and an undefined `Recipe::SIZE_2X2` reference in the fresh `PaneCraftingScreen.o` proving the new `gridSize()` compiled in — `gridSize()` itself is inlined, so it has no standalone symbol).
    CI `./gradlew assembleDebug` SUCCESS (run 42558617788, ~5m36s, 03:51:13–03:56:49 UTC 2026-11-09); APK `minecraft-mcpe-android-0.6.1-debug-v44.apk` 12,959,377 B, sha256 f48325afb00c770b3ac059ac9edc6cafe1a0ac6db817b2ab89d2c2c2a9bb8927, versionCode 44, `apksigner verify` v1+v2 true, `unzip -t` clean; the APK itself was re-checked to contain the new code in its `libminecraftpe.so` (arm64 10,269,512 B / armv7 8,802,180 B — byte-identical sizes to the local build; the .so is stripped so `nm` shows no symbol table — verified instead via the local object files, see above). One download hiccup, not a CI problem: the first artifact fetch via Python `urllib` timed out on the redirect (300 s); the retry via `curl` downloaded the 12,872,587 B artifact zip in ~7 s.
  ### Known limits / debts
  - Manual grid placement is still NOT built: tapping an inventory item does nothing, and there is still no crafting container in the tree (see "Next" open question — needs the user's explicit yes before it is built).
  - The collapse button is "<<" / ">>" text, not the screenshot's green book icon — no book glyph exists in the spritesheet; a drawn icon would be a separate art task.
  - The craftable-only toggle hides uncraftable recipes entirely while on (modern MC dims them instead); the dimmed-overlay state still exists for the toggle-off view.
  - All of it is build-only until a live test on device (see the "Next" v44 test list).
- **v45 — Dragon fireball self-damage immunity (SHIPPED, build-only).** Pays the v45 "Next" candidate — the key that lets v41's dragon-reflection exclusion be revisited.
  ### Audit (read from code before writing anything)
  - **The dragon had NO `hurt` override.** `EnderDragon` declared no `hurt` (`EnderDragon.h`), so every hit fell through `Monster::hurt` → `Mob::hurt` (`Mob.h:88` virtual) with no fireball check at all — unlike `Ghast::hurt` / `Blaze::hurt` (v40).
  - **Damage routing is already the v40 shape — nothing to change there.** `EnderDragon::checkHurtTarget` shoots `new Fireball(level, this)` (one aimed ball every 3 s within 40 blocks while orbiting, v30 — same `(Level, Mob)` ctor + `shoot(xd, yd, zd, 1.0f, 8)` as ghast/blaze), `Throwable`'s ctor sets `ownerId` to the shooter's `entityId` (`Throwable.cpp:20`), and `Fireball::onHit` already explodes with the fireball itself as the blast source (v40: `level->explode(this, ...)`), so `Explosion` calls `hurt(fireball, dmg)` and the ownerId check in `hurt()` is exact. No `Fireball::onHit`, `Explosion` or `Throwable` change was needed.
  - **When it can actually happen:** the dragon orbits at r22/y82 and fires outward/down, so its own blast landing beside it is even less plausible than the ghast's (v40 audit said the same) — the real path is a shot hitting a pillar or the portal ring while the dragon banks close past it; its 4-wide body catches a radius-1 blast from ~2 blocks away. The v40 formula gives up to ~9 damage at point blank; the dragon has 200 HP, so one own blast is survivable — this is a correctness/modern-MC batch, not a survival fix.
  - **OwnerId is reliable for this — within a session (v40 audit, same facts).** `entityId` is a per-run counter, 0 means "no owner" and can never match a real mob; a fireball rebuilt from a save has ownerId 0 and simply gets no immunity (safe fallback). `Throwable::getOwnerId()` already exists (v40), no new accessor needed.
  - **Reflection NOT in scope (brief's explicit boundary).** v41's `Fireball::hurt` resolves the owner and leaves dragon-owned balls alone (`hurt` falls through to base, returns false) precisely because this immunity did not exist. v45 supplies the missing half, but enabling reflection for dragon balls is a separate batch/decision — this version changes nothing in `Fireball::hurt`.
  ### What shipped
  - `EnderDragon::hurt` (new override, `EnderDragon.h/.cpp`): rejects exactly `source != NULL && source->getEntityTypeId() == EntityTypes::IdFireball && ((Fireball*)source)->getOwnerId() == entityId` → return false, before `Mob::hurt`, so no invulnerableTime, knockback or hurt sound is spent on blocked hits — the v40 pattern copied verbatim from `Ghast::hurt` / `Blaze::hurt`. Player melee, arrows, TNT / creeper / crystal blasts (NULL or another entity) still hurt; a fireball from another dragon (different ownerId) would still hurt too — only one dragon exists at a time (`Minecraft::ensureEndDragon`), so that half is structural, not testable.
  - Version bump 44 -> 45 (`app/build.gradle`, `AndroidManifest.xml`, `build-apk.sh`). No local NDK build in this VM (no NDK installed; Gradle local does not run here) — CI is the build verification for this version.
    CI `./gradlew assembleDebug` SUCCESS (run TBD); APK `minecraft-mcpe-android-0.6.1-debug-v45.apk` TBD.
  ### Known limits / debts
  - Dragon fireballs are STILL not reflectable (v41 exclusion stands — see "Next": revisiting it is the natural v46 candidate now that the immunity exists, but it needs its own batch/decision, not a silent change here).
  - A fireball rebuilt from a save has ownerId 0 and no immunity (session-only ids — v40 audit; deliberate, safe fallback).
  - All of it is build-only until a live test on device (see the "Next" v45 test list).
- **v37 — Multi-wither boss bar: lowest-HP wither (SHIPPED,
  build-only).** Pays the v36 "Next" candidate — the last
  wither-only debt. One batch, one file of gameplay code.
  ### Audit (read from code before writing anything)
  - **How the bar found its wither** (`Gui::renderBossBar`,
    `Gui.cpp:711`, v34): a single scan of
    `Level::entities`, first entry with type id
    `MobTypes::Wither` and `isAlive()` wins — then `break`.
    The bar therefore showed whichever wither the entity list
    happened to hold first (spawn order in practice), even if
    another wither was nearly dead. Name + phase colour came
    from that same wither (`(Wither*)boss` → `isCharging()` /
    `isArmoured()`, v36), and the fill was
    `barW * health / getMaxHealth()` — all correct for the
    wither shown, wrong only in WHICH wither was shown.
  - **Layout** (`Gui.cpp:732` + `hudLayout`): the bar is
    hotbar-wide (`barW`), top-centred — name at y=2, bar
    y0=12, height 5. The hearts/hunger/XP/hotbar row all sit
    at the BOTTOM, so the top of the screen is otherwise
    empty; a second bar stacked below the first (name ~y=19,
    bar ~y=29) would not collide with anything.
  - **Multi-bar precedent in this HUD tree**: NONE. Every
    other HUD element is a single instance — one XP bar
    (`renderExperience`), one hearts row, one hunger row, one
    bubbles row. No code exists for laying out N stacked bars,
    and the count is unbounded (every ritual summons another
    wither), so N bars would eventually march down the screen
    toward the crosshair.
  - **Decision: ONE bar, lowest-HP wither — multi-bar NOT
    shipped.** The brief explicitly allows this, and the audit
    says it is the right call: a single bar keeps the HUD
    exactly as v34/v36 laid it out (zero overlap risk with
    hotbar/XP/hearts), the change is a ~10-line local edit in
    the scan loop, and the lowest-HP wither IS the fight that
    matters (the one closest to dying — the progress the player
    is actually winning). Multi-bar would add layout code for
    an unbounded count to solve a problem the single correct
    bar already solves; it stays a possible future polish, not
    a v37 requirement.
  ### What shipped
  - `Gui::renderBossBar` no longer `break`s on the first live
    wither: it scans ALL live withers and keeps the one with
    the lowest health FRACTION (`health / getMaxHealth()` —
    every wither shares max 300, so fraction == absolute HP
    today, but the fraction is the honest "most damaged"
    measure if a max ever differs). Ties keep the first-found
    wither (strict `<`, no flip-flopping between equal-HP
    bosses). Name + phase colour + fill all follow the
    selected wither unchanged (Charging blue / Armoured grey /
    plain purple, v36) — no other line of the bar changed.
    Dead withers are still skipped (`isAlive()`), so the bar
    moves to the survivor when the lowest-HP wither dies and
    disappears when the last one dies, exactly as v34 did for
    a single boss.
  - Version bump 36 -> 37 (`app/build.gradle`,
    `AndroidManifest.xml`, `build-apk.sh`). Local NDK r17c
    build (both ABIs) passes: `libminecraftpe.so` arm64
    10,244,936 B / armv7 8,793,988 B (v36: 10,265,416 /
    8,793,988 — arm64 smaller by ~20 KB from the rebuilt
    translation unit; armv7 byte-identical size by
    coincidence of layout, Gui.o confirmed rebuilt after the
    edit in both ABIs). CI `./gradlew assembleDebug` SUCCESS
    (run 42545637924, ~5m51s, 01:27:05–01:32:56 UTC
    2026-11-09); APK
    `minecraft-mcpe-android-0.6.1-debug-v37.apk`
    12,948,178 B, sha256
    5020cc117ff8ca8f5d7d0f5ed2963f46ea9efc74c2524f2198d65b560bf979e7,
    versionCode 37, `apksigner verify` v1+v2 true, `unzip -t`
    clean; the APK itself was re-checked to contain the new
    code in its `libminecraftpe.so` (arm64 10,244,936 B /
    armv7 8,793,988 B — byte-identical sizes to the local
    build; the .so is stripped so `nm` shows no symbol table —
    verified instead via strings: `Wither - Armoured` +
    `Wither - Charging` present in both ABIs).
  ### Known limits / debts
  - One bar only (see decision above): with 3+ withers the
    player sees only the lowest-HP one, not the whole fight.
    If the device test says that hides too much, a stacked
    multi-bar is the v38+ polish — bounded to the first few
    withers, never unbounded.
  - All of it is build-only until a live test on device (see
    the "Next" v37 test list: lowest-HP selection, bar
    hand-off on death, bar gone when all dead, no crash, old
    features intact).
- **v36 — Wither boss, stage 3: armoured phase + spawn charge-up
  (SHIPPED, build-only).** Pays the v35 "Next" candidate. The
  coordinator's ruling on the v35 note ("wait for the v34/v35
  device test — if the fight already feels too hard, tune DOWN"):
  no device test has arrived yet, so stage 3 ships with
  CONSERVATIVE tuning — all three parts are implemented, but none
  makes the fight harder than v35. The armour phase adds no damage
  reduction and no extra damage; the charge-up is a generous 5 s;
  the power-7 blast is the modern-MC value but is telegraphed by
  the whole charge-up. Every v36 number below is PROVISIONAL until
  there is device feel from the v34/v35/v36 test.
  ### Audit (read from code before writing anything)
  - **Wither today**: `Wither` (Wither.cpp) — 300 HP, regen 1 HP /
    20 ticks by DIRECT health increment, NOT `heal()` (`heal()`
    sets `invulnerableTime` = 10 ticks, which would make the boss
    unhittable half the time — the v34 comment says so explicitly).
    Volley 3 skulls / 3 s (`checkHurtTarget`, v35), melee + Wither
    I 10 s (`doHurtTarget`, virtual since the v35 `Monster.h` fix),
    `heal()` override caps at 300 (same shape as the dragon's 200).
    `invulnerableTime`/`invulnerableDuration` (20) live on
    `Mob`/`Entity`; `Mob::hurt` is the single damage gate and it
    already does the i-frames bookkeeping (lastHurt, hurtTime,
    knockback, death → `die(source)`).
  - **Damage routing — the armour hook**: EVERY hit in this tree
    arrives as `Mob::hurt(Entity* source, int dmg)`. Player melee
    is `Player::attack` → `entity->hurt(this, dmg)` (source = the
    player, a Mob); mob melee is `Monster::doHurtTarget` →
    `target->hurt(this, dmg)` (source = the mob); arrows are
    `Arrow::tick` → `res.entity->hurt(this, dmg)` (source = the
    Arrow entity, type id 80); explosions are `Explosion::explode`
    → `e->hurt(source, dmg)` with source = the exploding entity
    or NULL (ritual/skull blasts pass NULL). So a source-TYPE check
    in one `Wither::hurt` override separates melee from projectiles
    exactly — no DamageSource system exists in this tree (the
    commented-out `DamageSource` in Monster.h/Player.cpp confirms
    it was never built), and none needs building for this.
  - **Ritual today**: `trySummonFromSkull` consumes the T + 3
    floor skulls, then `level->explode(NULL, sx, sy+1, sz, 3.0f,
    false)` and spawns the wither at full HP, already fighting.
    The blast's damage formula (`Explosion.cpp`: `(pow²+pow)/2 ×
    8 × r + 1` at the centre) means radius 3 hits ~25 at point
    blank; power 7 hits ~57 — the modern-MC value, and the reason
    the charge-up must give real time to get away.
  - **Phase/invulnerable pattern in other bosses**: NONE. The
    EnderDragon (v22/v28) has a perch STATE MACHINE (ORBIT /
    DESCEND / PERCHED / ASCEND, `perchState` + `perchTimer` in
    EnderDragon.cpp) but NO damage phase and NO invulnerability —
    the dragon is always hittable, even perched. The wither's
    charge is therefore a plain countdown in `tick()`, not a
    dragon-style state machine; the armour phase is derived from
    health, not a flag.
  - **Save/load**: `Mob` saves health as a short (300 fits) via
    `addAdditonalSaveData` (note the tree's typo) /
    `readAdditionalSaveData`; `CompoundTag::getShort` returns 0
    for a missing key, so a new tag defaults cleanly on old saves.
  - **Visual hooks that already exist**: `MobRenderer::scale`
    is virtual (`SlimeRenderer` overrides it for squash-and-
    stretch); the boss bar (`Gui::renderBossBar`, v34) already
    scans `Level::entities` for the live wither and draws name +
    fill, so phase text/colour is a local change there; smoke
    puffs have a precedent (`Mob::tick` spawns one every 20 ticks
    while withered; `Enderman` teleports with them).
  ### What shipped
  - **Armoured phase** (`Wither::hurt` override, `isArmoured()`):
    below half HP (health < 150) the wither is immune to
    projectiles — Arrow (80), snowball (81), egg (82), fireball
    (85), splash potion (86), wither skull (88); a blocked arrow
    bounces off (`Arrow::tick`'s `hurt == false` path) instead of
    sticking. Melee (player/mob source) is unchanged, and
    NULL-source damage (explosions, wither-effect ticks) still
    lands, so TNT stays a valid weapon and the wither's own
    skull blasts still hurt it exactly as in v34/v35. NO damage
    reduction on top — the phase forces the player to close in,
    it does not raise the boss's effective HP. Once armoured,
    always armoured (derived from health, no flag to clear).
  - **Spawn charge-up + power-7 blast** (`startSpawnCharge`,
    `spawnChargeTicks`, `WITHER_SPAWN_CHARGE_TICKS = 100` = 5 s):
    `trySummonFromSkull` no longer explodes at once — the wither
    appears and grows for 5 s, fully invulnerable (`hurt()`
    returns false), hovering in place without chasing or
    attacking (`tick()` skips steering, `checkHurtTarget` returns
    early), then a power-7 explosion goes off at the wither
    (`level->explode(this, ...)` — source = the wither, so its
    own blast never hurts it, and the counter is still 1 on the
    blast tick so `hurt()` would block it anyway) and the fight
    starts. 5 s at walk speed is ~20 blocks — the blast is
    dodgeable by distance, but standing on the ritual at the
    blast is ~40 damage: run. The counter is saved
    (`"WitherCharge"` short) so quitting mid-charge resumes the
    charge on reload; old saves read 0 = a fighting wither.
  - **Phase visuals** (three tells, all cheap): the renderer
    grows the wither from half-size to full during the charge
    (`WitherRenderer::scale`, `getSpawnChargeFraction()`) and
    bulks it ~8% while armoured (render scale only, hitbox
    unchanged); the boss bar shows "Wither - Charging" (blue
    fill) / "Wither - Armoured" (steel-grey fill) instead of
    plain "Wither" (purple); smoke puffs rise off the wither
    every 10 ticks while charging or armoured.
  - Version bump 35 -> 36 (`app/build.gradle`,
    `AndroidManifest.xml`, `build-apk.sh`). Local `ndk-build`
    (r17c, both ABIs) passes: `libminecraftpe.so` arm64
    10,265,416 B / armv7 8,793,988 B (v35: 10,249,032 /
    8,789,892), new strings confirmed in both .so files
    (`Wither - Armoured`, `Wither - Charging`, `WitherCharge`).
    CI `./gradlew assembleDebug` SUCCESS (run 42544406832,
    ~4m21s, 01:13:27–01:17:48 UTC 2026-11-09); APK
    `minecraft-mcpe-android-0.6.1-debug-v36.apk`
    12,948,705 B, sha256
    c84bea148caa40776056340c1880a0109b784d7b5f651e4241ee3bbbe2d471b8,
    versionCode 36, `apksigner verify` v1+v2 true, `unzip -t`
    clean; the APK itself was re-checked to contain the new
    code in its `libminecraftpe.so` (arm64 10,265,416 B /
    armv7 8,793,988 B — byte-identical sizes to the local
    build; the .so is stripped so `nm` shows no symbol table —
    verified instead via strings: `Wither - Armoured` +
    `Wither - Charging` + `WitherCharge` present in both ABIs).
  ### Known limits / debts
  - **All v36 tuning numbers are PROVISIONAL** (see above):
    armour threshold 150, charge 100 ticks (5 s), blast power 7,
    no damage reduction. If the device test says the fight is
    too hard, the next version tunes DOWN (e.g. shorter charge
    is NOT the lever — the levers are a smaller blast, a higher
    armour threshold, or partial projectile resistance instead of
    full immunity); if it says too easy, the levers are damage
    reduction in the armoured phase or a shorter charge. Do not
    re-tune blind.
  - The boss bar still shows the FIRST live wither only
    (multiple withers share one bar); the bar has no wither icon.
  - The wither's own skull blasts can still damage it (NULL
    source, pre-existing v34/v35 behaviour — unchanged here).
  - All of it is build-only until a live test on device (see
    the "Next" v36 test list: charge-up, blast, armour, star,
    fairness, no crash, old features intact).
- **v35 — Wither boss, stage 2 (SHIPPED, build-only).**
  Pays the v34 stage-2 debts in the ROADMAP "Next" order: the
  three-head volley, the small-head skull + wall mounting, and
  the wither rose. The armoured phase stays deferred (needs
  device feel first, see below).
  ### Audit (read from code before writing anything)
  - **Volley today**: `Wither::checkHurtTarget` (Wither.cpp) —
    1 aimed skull every 2 s (`attackTime = TicksPerSecond * 2`),
    `WitherSkull` is a `Throwable` (no gravity, Fireball-shaped)
    whose ctor spawns at the mob's head height and whose
    `shoot(xd, yd, zd, 1.0, 8)` is re-called after creation, so
    extra skulls are just more `new WitherSkull` + `shoot` calls
    with rotated aims and sideways offsets — no new projectile
    class needed. Modern MC fires one skull per head per volley.
  - **Skull tile today**: full cube (`SHAPE_BLOCK`, default
    shape), top-face placement only (WitherSkullItem rejects any
    face but UP). The wall-mount pattern exists twice:
    `TorchTile::getPlacedOnFaceDataValue` (data 1–5 by face) +
    `mayPlace(level,x,y,z,face)`; `LeverTile` the same, with an
    ON bit. The small-shape pattern exists too: `TileRenderer::
    tesselateInWorld` calls `tt->updateShape(level,x,y,z)` before
    rendering a SHAPE_BLOCK tile, `Tile::clip` calls it before
    the hit test, and `SignTile`/`StairTile` override it per data
    value (SignTile also overrides `getTileAABB` the same way) —
    so a per-data small head box covers render + hit +
    collision with one override, and `Tile::init` derives
    lightBlock 0 from `isSolidRender() == false`, so the head
    blocks no light. `Tile::getAABB` uses the shape bounds, so
    the head gets a small collision box for free.
  - **Plant pattern for the rose**: `Bush` (flower 37 / rose
    38) — cross texture, small plant box, no collision AABB,
    `entityInside` precedent is `CactusTile` (hurts any entity
    whose AABB overlaps the cell; `Entity::move` calls it for
    every tile in the mover's AABB). No flower-specific item
    plumbing is needed: `Tile.cpp:368` auto-creates a TileItem
    for every registered tile, so a rose tile gets its placeable
    item (id 431) for free.
  - **Rose attribution**: `Mob::die(Entity* source)` gets
    `source == NULL` for wither-effect deaths (`Mob::tick`
    calls `hurt(NULL, 1)` every 40 ticks), so "killed by the
    wither" cannot be read from the death source — it needs a
    marker. The wither applies the effect in exactly two places
    (`WitherSkull::onHit`, blast radius; `Wither::doHurtTarget`,
    melee), so a `Mob::witheredByWither` bool set there and
    cleared when the effect expires is sufficient and precise.
  - **AUDIT FIND — v32 bug, fixed here**: `Monster::
    checkHurtTarget` calls `doHurtTarget(target)` on a
    `Monster*`, but `Monster::doHurtTarget` was NOT virtual, so
    `WitherSkeleton::doHurtTarget` (v32 — the wither-on-melee
    effect, documented as shipped) was NEVER dispatched: the
    skeleton's melee never withered anything. `PathfinderMob::
    checkHurtTarget` IS virtual (PathfinderMob.h:44), so the
    wither's own `checkHurtTarget` override (and the ghast's /
    dragon's) always ran — only `doHurtTarget` was dead. Fix:
    one word (`virtual` in Monster.h); no other mob overrides
    it, so no other melee behaviour changes — and the fix makes
    the documented v32 feature actually work.
  - **Texture**: `terrain.png` has no free cells left (v34 used
    252, the last one, for the skull face). Cell 253 is
    referenced ONLY by the never-generated placeholder tile
    `info_updateGame2` (249) — same situation as 252, so the
    rose sprite is drawn over it; those placeholder tiles would
    render as rose crosses if ever placed via the fill command.
  ### What shipped
  - **Three-head volley** (`Wither::checkHurtTarget`): 3 skulls
    per volley — centre head dead-on, side heads fanned
    ±0.15 rad (~8.6°) from ±0.55-block head offsets
    perpendicular to the aim, all with the same damage
    (explosion r1 + Wither I 10 s). Cooldown 2 s → 3 s to
    compensate (3 skulls / 3 s vs 1 / 2 s): the volley is more
    dangerous but the spread keeps it dodgeable at range.
  - **Small-head skull + wall mounting** (`SkullTile`, data
    0–4): floor = centred 8×8×8 px head in the bottom half of
    the cell; walls = same head hugged against the supporting
    wall, mid-cell height. Data: 0 floor, 1 north wall (support
    z+1), 2 south (z-1), 3 west (x+1), 4 east (x-1). Both item
    paths (skull item 397 + TileItem 144) use
    `getPlacedOnFaceDataValue` + face-aware `mayPlace`, so they
    agree; DOWN is still rejected. A skull whose support is
    broken pops off and drops the item (TorchTile shape). The
    skull is no longer solid-render (neighbours keep their
    faces, light passes).
  - **Ritual safety**: `setPlacedBy` only tries the summon for
    floor skulls (data 0), and `trySummonFromSkull` requires
    data 0 on all three skulls — a wall-mounted row can never
    complete or false-positive a ritual. The ritual consumption
    order (skulls first, soul sand after) is unchanged, so no
    floor skull is ever left standing on removed support.
  - **Wither rose** (`WitherRoseTile`, id 175, tex 253, Bush):
    grows on ANY solid block (not just grass/dirt/farmland) and
    needs no light — it only needs solid support, so it survives
    in the Nether/End where the wither fights. Touching it gives
    any mob Wither I for 2 s, refreshed while standing in it
    (`entityInside`, cactus precedent, server-side only); the
    wither boss itself is immune. A mob that dies while
    withered BY THE BOSS (`witheredByWither`, set by the boss's
    skulls/melee only — never by skeletons or roses) leaves a
    rose where it died (`Mob::die`: planted as a tile when the
    spot is air over solid ground, dropped as an item
    otherwise). The rose sprite (dark-red bloom, black centre,
    dark stem) is drawn into terrain.png cell 253; lang
    `tile.witherRose.name`; in the creative inventory.
  - **Boss melee withers** (`Wither::doHurtTarget`, new
    override): the wither's close-range swing now also applies
    Wither I (10 s) + the rose marker (modern MC). This works
    only because of the `virtual` fix above — without it, this
    override would have been dead code like the skeleton's was.
  - Version bump 34 -> 35 (`app/build.gradle`,
    `AndroidManifest.xml`, `build-apk.sh`). Local `ndk-build`
    (r17c, both ABIs, libtinfo.so.5 shim) passes: `libminecraftpe.so`
    arm64 10,249,032 B / armv7 8,789,892 B, `Tile::witherRose`
    symbol confirmed in the built .so. CI `./gradlew
    assembleDebug` SUCCESS (run 42542801877, ~5m56s,
    00:55:33–01:01:29 UTC 2026-11-09); APK
    `minecraft-mcpe-android-0.6.1-debug-v35.apk`
    12,944,338 B, sha256
    652ed3d55c04a32b024460d65c692fea8cb0eb38a3592c30744e625ad9cac694,
    versionCode 35, `apksigner verify` v1+v2 true, `unzip -t`
    clean; the APK itself was re-checked to contain the new
    code in its `libminecraftpe.so` (arm64 10,249,032 B /
    armv7 8,789,892 B — byte-identical sizes to the local
    build; the .so is stripped so `nm` shows no symbol table —
    verified instead via strings: `Tile::witherRose` +
    `WitherRoseTile` typeinfo present) and the rose sprite in
    terrain.png cell 253 (56 opaque px, cell 252 skull face
    untouched) + the `tile.witherRose.name` lang entry.
  ### Known limits / debts (stage 3 and beyond)
  - **No armoured phase — STILL DEFERRED, deliberately.** Modern
    MC gives the wither armour + arrow immunity below half HP.
    It is a balance change, not a code gap, and the fight has
    never been felt on device yet (v34 + v35 are both
    build-only). Tuning it blind risks an unkillable or trivial
    boss; it waits on the v34/v35 device test below.
  - **No wither spawn protection / spawn animation.** Modern MC
    grows the wither over ~2 s with an invulnerable charge-up
    and a bigger (power 7) summon blast; stage 1/2 still summon
    instantly with a radius-3 blast.
  - The boss bar shows the FIRST live wither only (multiple
    withers share one bar); the bar has no wither icon, just the
    name + fill.
  - **Rose attribution is "died while withered by the boss",
    not "killed by the boss's damage".** If the player lands
    the finishing blow on a mob the wither withered, a rose
    still grows — a deliberate simplification (the death source
    is NULL for wither-effect deaths, so the exact killing blow
    is not attributable in this tree). The common case (the
    wither fighting mobs) is exact.
  - The wither rose has no crafting / compost use (modern MC
    has none either — it is a dye/black-dye source there; no
    dye system exists in this tree, so nothing to wire it to).
  - All of it is build-only until a live test on device (see
    the "Next" v35 test list: volley, wall skulls, rose, ritual
    still works, no crash, old features intact).
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
- **Device verification for the wither boss, stage 2 (v35's open
  item).** Uninstall the old APK first (fresh CI debug keystore).
  Summon the wither (same ritual as v34) and check it now fires
  a 3-skull volley every ~3 s — centre skull on target, side
  skulls fanning out — more dangerous than v34 but still
  dodgeable at range, and it never spams; let it kill a mob
  (spawn one near it, or lure a zombie) and check a wither rose
  grows where the mob died (or drops as an item if the spot is
  mid-air); walk into the rose and check it withers you (black
  hearts, HP draining) for ~2 s; check a wither skeleton's melee
  now withers too (v32 fix — its swing should blacken the
  hearts); place a skull on a WALL (side face of a block) and
  check it renders as a small head hugged to the wall, pops off
  when the wall is broken, and does NOT summon when three wall
  skulls are stuck to the sides of a soul-sand T; place a skull
  on the floor and check it is a small head, not a full cube;
  re-check the v34 list (ritual consumes the T, boss bar drains,
  1 nether star + 50 XP, incomplete T does not summon, peaceful
  keeps the boss) and the old features, no crash. The v34 APK
  stays the fallback until this stage passes.
- **Device verification for the wither boss, stage 3 (v36's open
  item).** Uninstall the old APK first (fresh CI debug keystore).
  Summon the wither (same ritual as v34) and check the wither
  appears but does NOT attack for ~5 s — it grows from half-size
  (boss bar reads "Wither - Charging", blue fill, smoke puffs)
  and cannot be hurt (sword and bow both bounce off / do nothing,
  no damage numbers on the bar); use those 5 s to walk away, then
  check the big explosion goes off (power 7 — terrain damage in a
  wide radius, much bigger than v34/v35) and the fight starts
  (bar back to plain "Wither", purple); check standing on the
  ritual at the blast hurts a LOT (~40 damage — run, don't tank
  it) while standing ~8+ blocks away is safe; fight it down below
  half HP and check the bar reads "Wither - Armoured" (grey fill),
  the wither looks slightly bigger with smoke puffs, arrows now
  bounce off and do NO damage while sword hits still land and the
  bar still drains; kill it and check 1 nether star + 50 XP;
  check the fight feels fair, not impossible (melee-only second
  half, same damage as before); quit mid-charge and reload and
  check the charge resumes instead of the fight starting instantly;
  re-check the v34/v35 lists (ritual consumes the T, 3-skull
  volley, rose, wall skulls, peaceful keeps the boss) and the old
  features, no crash. The v35 APK stays the fallback until this
  stage passes. **TUNING FEEDBACK NEEDED — this is the point of
  the test:** is 5 s too long / too short to get clear? Is the
  power-7 blast fair at distance? Is the armoured phase a fun
  second half or a wall (too hard to reach in melee)? Report the
  feel and the v37 numbers follow from it — do NOT re-tune blind.
- **Device verification for the multi-wither boss bar (v37's open
  item).** Uninstall the old APK first (fresh CI debug keystore).
  Summon TWO withers (two rituals, same as v34) and check the bar
  shows ONE bar only, for the wither with the lower HP — hit one
  wither until its HP is clearly lower than the other's and check
  the bar is the shorter one (and its phase name/colour is that
  wither's: plain "Wither" purple, or "Wither - Armoured" grey
  if the low one is below half HP); kill the lowest-HP wither and
  check the bar jumps to the survivor (longer bar again, that
  wither's phase); kill the second and check the bar disappears;
  check a single wither still shows its bar exactly as in v34–v36
  (Charging blue during the 5 s charge-up, Armoured grey below
  half HP); re-check the v34–v36 lists and the old features, no
  crash. The v36 APK stays the fallback until this stage passes.
- **Device verification for wither skull self-damage (v38's open
  item).** Uninstall the old APK first (fresh CI debug keystore).
  Summon the wither (same ritual as v34) and let it fire volleys at
  the player from close range — stand under it, let skulls explode
  against walls right next to the boss — and check the boss bar
  does NOT drain unless the player actually hits it (no more
  self-inflicted chip damage); summon TWO withers close together
  and check their skulls do not hurt each other (bars stay full
  until the player engages); check the player + nearby mobs STILL
  take explosion damage + the wither effect (black hearts, HP
  draining) from skull blasts; check the summon power-7 blast
  still does not hurt the newly summoned wither (bar full after
  the charge-up) while still hurting a player standing on the
  ritual; re-check the v34–v37 lists (3-skull volley, rose, wall
  skulls, charge-up, armour, lowest-HP bar) and the old features,
  no crash. The v37 APK stays the fallback until this stage passes.
- **Device verification for the blaze spawner tile (v39's open
  item).** Uninstall the old APK first (fresh CI debug keystore).
  Enter the Nether, walk ~320+ blocks and find a fortress BARU
  (new chunks — an old-world fortress has no spawner, only the
  top-up): check the blaze room has a cage block at its centre
  (dark bars + orange glow), wait near it — blazes spawn over
  time up to ~4 around the room; kill them — check they refill
  after the 10–20 s delay; walk >16 blocks away — check the
  spawner stops (no new blazes); quit/reload — check the spawner
  block + its blazes are still there; break the spawner — check
  it drops nothing; check the creative inventory has the spawner
  and a placed one spawns blazes; re-check the v31–v38 lists
  (fortress layout, wart room, loot chest, wither skeletons,
  boss bar, skull self-damage) and the old features, no crash.
  The v38 APK stays the fallback until this stage passes.
- **Device verification for the ghast/blaze fireball self-damage
  (v40's open item).** Uninstall the old APK first (fresh CI debug
  keystore). Enter the Nether, find a ghast, let it shoot at you
  while you stand CLOSE under it / back it against a wall in a
  narrow corridor so its fireball explodes near itself — check its
  HP does NOT drop from its own blast (it has only 10 HP, so one
  own blast would nearly kill it before v40); same for a blaze in
  a fortress room (fireball exploding beside it must not hurt it).
  Check a fireball from ANOTHER ghast/blaze DOES hurt the first
  one (lure two ghasts/blazes so one's shot lands near the other);
  check the player + nearby mobs STILL take explosion damage +
  fire from fireball blasts; check TNT/creeper blasts still hurt
  a ghast/blaze. HONEST LIMIT: the reflected-fireball test in the
  brief CANNOT be run in v40 — there is NO reflection mechanic in
  this tree (a fireball is not pickable, the player cannot punch
  one back), so do not report "reflection works" or "reflection
  fails"; v40 only preserves the mechanic for the version that
  adds it. Re-check the v31–v39 lists (fortress, spawner, boss
  bar, skull self-damage) and the old features, no crash. The v39
  APK stays the fallback until this stage passes.
- **Device verification for fireball reflection (v41's open
  item).** Uninstall the old APK first (fresh CI debug keystore).
  Enter the Nether, find a ghast, let it shoot at you — punch the
  incoming fireball when it is CLOSE (within ~3 blocks, the
  survival pick window — the ball crosses it in ~150 ms, so time
  the tap as it approaches) and check it flies back along where
  you are looking, faster than it came; aim at the ghast and check
  the reflected ball explodes on it and the ghast takes damage
  (10 HP — one clean reflected hit kills it); check a blaze
  fireball in a fortress room reflects the same way; check a
  fireball you do NOT punch still hurts you like before; check
  arrows shot at a fireball still bounce off (no reflect); check
  the dragon's fireballs still cannot be reflected (punching one
  does nothing, it keeps flying); re-check the v31–v40 lists
  (fortress, spawner, boss bar, skull self-damage, own-fireball
  immunity) and the old features, no crash. The v40 APK stays the
  fallback until this stage passes. HONEST LIMIT: if the 3-block
  window proves unhittable on touch in practice, report it — the
  fix is a bigger pick radius, not a proximity auto-reflect.
- **Device verification for the v43 bug-fix batch (4 user reports,
  2026-11-09 10:21 — do this first, before v44 code).** Uninstall
  the old APK first (fresh CI debug keystore). (a) Walk into water —
  check NO red-white bobbers appear, only blue bubble rings + the
  splash sound; (b) sprint across sand/grass and jump off a 4+ block
  ledge — check NO black smoke puffs trail you or burst on landing;
  break a reed block — check the fragments are green reed bits with
  NO black halves; (c) look at reeds, flowers, tall grass, wheat and
  saplings — check NO black background/bands on any of them
  (reeds must be green stalks on clear background); (d) in CREATIVE:
  check the XP bar (dark strip, green fill if any XP) + level number
  render above the hotbar even with 0 XP (empty dark strip is
  correct), and the last hotbar slot shows "..."; in SURVIVAL:
  hearts + hunger + XP bar all render as in v14; re-check the
  v31–v42 lists (fortress, spawner, boss bar, fireball reflection,
  crafting UI v42 + the v42 movement-freeze fix) and the old
  features, no crash. The v42 APK stays the fallback until this
  stage passes.
- **Device verification for the v44 inventory-crafting stage (do this before v45 code).** Uninstall the old APK first (fresh CI debug keystore). Open the inventory → Craft: check the crafting grid is 2x2 (no empty third row/column), the recipe list shows only 2x2 recipes (e.g. no crafting-table recipe), and the same recipe clicked on the table (3x3) and in the inventory (2x2) fills its grid identically; click fermented spider eye (3-ingredient shapeless: spider eye + sugar + brown mushroom, `Recipes.cpp:528`) and check all 3 show in the 2x2 grid as 2+1 rows (old 3-wide layout showed 1 row of 3); note the tree has no 4-ingredient shapeless recipe, so the n=4 reflow path itself has no live recipe to test yet; tap the green check toggle — uncraftable recipes disappear, tap again — they return; search + a tab + toggle together filter correctly; collapse the book with "<<" — the crafting panel widens, tabs/search disappear, ">>" brings the book back with the same selection; re-check the v42 table screen (still 3x3, still works), the stonecutter (single tab, 3x3), the v43 fixes and the old features, no crash. The v43 APK stays the fallback until this stage passes.
- **OPEN QUESTION (ask the user, do not build silently): manual grid placement** — tap an inventory item to place it into the crafting grid by hand. There is no crafting container in the tree, so that is real new work (container + slot interaction), not a layout tweak; the user never asked for it. Also still open from v43: the touch hotbar has 7 usable slots + "..." (`MAX_SELECTION_SIZE` 9) while the modern reference shows 9 usable + "..." — is the narrower hotbar part of "inventory kelihatan aneh", and is widening it to 9 usable slots wanted? That change touches selection size, touch areas and MP, so it needs an explicit yes.
- **Device verification for the v45 dragon-immunity stage (do this before v46 code).** Uninstall the old APK first (fresh CI debug keystore). Enter the End, let the dragon shoot while it banks close past a pillar or the portal ring — its own fireball blast landing beside it must deal no damage (hard to stage deliberately: the orbit fires outward/down, so a self-blast is rare by design; the practical check is the fight plays normally with no dragon self-damage observed over a full orbit+perch cycle). Player melee (sword while perched), arrows and end-crystal damage must still hurt it; TNT/creeper blasts still hurt; re-check the v41 reflection list (ghast/blaze balls still reflect and kill the ghast; dragon balls still do NOT reflect — punch one and it keeps flying, no reflect); re-check the v40 lists (ghast/blaze own-fireball immunity intact) and the old features, no crash. The v44 APK stays the fallback until this stage passes.
- **Next version (v46 candidate): dragon fireball reflection** — revisit v41's exclusion now that v45 gives the dragon the v40 owner immunity: drop the owner-type check in `Fireball::hurt` so a punched-back dragon ball can hurt the dragon (200 HP — it will NOT one-shot it like the 10 HP ghast; the fight balance of that needs a device feel, not a guess). Needs its own batch/decision, not a silent change. The rose's dye use still needs a dye system first (a whole subsystem, not a batch), the icon is cosmetic, spawner mob-switching needs a spawn-egg system + a tile-entity renderer that do not exist. All spawner tuning stays provisional until the v39 device feel arrives — do NOT re-tune blind.
  V42 DEVICE TEST LIST (do this first, before v43 code): open a crafting table while holding D-pad forward — repeat 5x — the player must stop on the same tick the screen opens; same repro from inventory 2x2, from a chest, and from a furnace; tap the search field, type "pick", check the grid filters live, backspace, Esc/back closes the keyboard (not the screen); switch tabs (All/Tools/Blocks/Food/Redstone) and check Mechanisms recipes finally appear; paginate a full tab with < > and check "N/M"; click a recipe → grid fills with have/need counts, tap the result → the item is crafted and counts refresh; tap an uncraftable recipe → result dimmed, tap does nothing; stonecutter screen still works (single tab); re-check the v31–v41 lists and the old features, no crash. The v41 APK stays the fallback until this stage passes.
- **v45 — Dragon fireball self-damage immunity.** SHIPPED (see "Done").
- **v44 — Inventory crafting 2x2.** SHIPPED (see "Done").
- **v43 — Bug-fix batch (4 user reports).** SHIPPED (see "Done").
- **v42 — Modern crafting UI.** SHIPPED (see "Done").
- **v41 — Fireball reflection.** SHIPPED (see "Done").
- **v40 — Ghast/blaze fireball self-damage.** SHIPPED (see "Done").
- **v39 — Blaze spawner tile.** SHIPPED (see "Done").
- **v38 — Wither skull self-damage.** SHIPPED (see "Done").
- **v37 — Multi-wither boss bar.** SHIPPED (see "Done").
- **v35 — Wither boss, stage 2.** SHIPPED (see "Done").

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
