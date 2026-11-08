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
- World 1024×1024 ONLY with measurements (v3/v6 rejected it on paper:
  4096 chunks ~339 MB + ~400k VBO buffers). The engine is a fixed chunk
  cache — a truly infinite world like modern Minecraft is NOT feasible
  without rewriting the engine; don't promise it.
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

## Next (v11)

- **v12 — Minecarts + rails.** Rideable minecart on rails.

## Queued (priority order — re-evaluate after each version)

- **v12 — Minecarts + rails.** Rideable minecart on rails.
- **v13 — More mobs.** Slime (+ slimeball), Enderman (+ ender pearl),
  witch — whichever fits the AI framework (GoalSelector exists).
- **v14 — Graphics polish.** Clouds, water animation, sky, particles
  (breaking, splashing). Low risk, high visibility.
- **v15 — Music + ambient sounds.** 102 sound files exist; check what
  plays today and add a music/ambience layer.
- **v16 — Brewing.** Brewing stand + potions. Blocked on Nether
  ingredients (nether wart, blaze powder) — needs a Nether source first,
  so this waits behind v17 or a Nether-reactor loot change.
- **v17 — Nether dimension.** Big. The old Nether Reactor exists as a
  placeholder; a real Nether needs a second dimension + portal.
- **v18 — The End.** Biggest. Ender dragon, end stone, chorus — only
  after Nether works.
- **v19 — Villages + trading.** Village generation + villagers + trading
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
- (none yet — fill in as versions prove things out)
- **Infinite world** — not feasible: fixed `CHUNK_CACHE_WIDTH` chunk
  cache; max sane size is 512×512 today (1024 pending measurement).
