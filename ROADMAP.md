# MCPE Android — Roadmap to Modern Minecraft

Goal: bring this MCPE 0.6.1 build (`Kallzzxa/Minecraft-MCPE-Android`) as close
as possible to modern Minecraft PE (Bedrock), one testable version at a time.
Each version is ONE coherent batch of features, shipped as a new APK
(`minecraft-mcpe-android-0.6.1-debug-v<N>.apk`, versionCode N).

## Rules

- One batch per version. Small enough to implement + CI in ~10–20 minutes.
- Never break what already works: right-side jump button, hunger + meat
  drops + furnace, sprint, 512×512 world, v4 lighting, 0.8× fog.
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

## Next (v8)

- **v7 — DONE (fallback: hostile mob drops, see "Done").** Boats were
  investigated and blocked (see below); v8 drops shipped in their place.

## Queued (priority order — re-evaluate after each version)

- **v8 — Night spawning verification + spider eye.** Drops are done (v7);
  what remains is verifying hostile mobs actually spawn at night
  (`Level::spawnMobs` / `MobSpawner::tick` runs when `_spawnEnemies` and
  difficulty > peaceful — code exists, needs a live night test on device).
  Optional: spider eye drop (its `dropDeathLoot` is commented out).
- **v9 — XP orbs + levels + HUD.** No XP system exists yet
  (no `ExperienceOrb` in the tree). Orbs on mob kill, level bar, level
  number. Enchanting comes later.
- **v10 — Carrots & potatoes.** New crop tiles + food items, farmable
  like wheat (CropTile exists). Carrot/potato drops from zombies as a bonus.
- **v11 — Enchanting.** Enchanting table + basic enchantments
  (sharpness, efficiency, protection, unbreaking). Needs v9 XP.
- **v12 — Redstone basics.** Redstone dust + torch + lever + simple power
  propagation. (Redstone ore tile exists; no wiring yet.)
- **v13 — Minecarts + rails.** Rideable minecart on rails.
- **v14 — More mobs.** Slime (+ slimeball), Enderman (+ ender pearl),
  witch — whichever fits the AI framework (GoalSelector exists).
- **v15 — Graphics polish.** Clouds, water animation, sky, particles
  (breaking, splashing). Low risk, high visibility.
- **v16 — Music + ambient sounds.** 102 sound files exist; check what
  plays today and add a music/ambience layer.
- **v17 — Brewing.** Brewing stand + potions. Blocked on Nether
  ingredients (nether wart, blaze powder) — needs a Nether source first,
  so this waits behind v18 or a Nether-reactor loot change.
- **v18 — Nether dimension.** Big. The old Nether Reactor exists as a
  placeholder; a real Nether needs a second dimension + portal.
- **v19 — The End.** Biggest. Ender dragon, end stone, chorus — only
  after Nether works.
- **v20 — Villages + trading.** Village generation + villagers + trading
  UI. Big; last among gameplay features.

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
