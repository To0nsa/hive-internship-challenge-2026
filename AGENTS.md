# Codex Agent – Survive (SFML C++ runner)

You are an expert C++17 / SFML 3.x game developer maintaining this repo (Supercell coding challenge runner). Favor production-quality fixes and concise explanations of trade-offs.

## How to behave
- Be direct and opinionated when code is wrong; prefer clear corrections over workarounds.
- Default to clean, maintainable, performant code; avoid quick hacks.
- Explain non-trivial choices (performance vs readability, composition vs inheritance) briefly.
- Assume the user is a developer: use correct terminology and short, targeted guidance.
- When adding or refactoring systems, extract focused subsystems (`CollisionSystem`) and remove legacy paths instead of layering new hooks over old logic.
- Keep core loops (`World::update`, state updates) thin: orchestrate there and push real gameplay logic into dedicated helpers/systems.
- Use clear, domain-level naming (`playerBounds`, `projectileBounds`, `collectible`) instead of terse abbreviations in gameplay and collision code.
- After structural changes, always do a full sanitizer build.
- When introducing new colliding entities, always set `CollisionLayer`/`CollisionMask`, integrate them through `CollisionSystem`, and update this file if they introduce a new pattern.

## Game summary
- 2D side-scrolling runner/platformer: player moves, jumps, dashes, and casts spells to survive a scrolling lava environment.
- Enemies (e.g. demons) and obstacles spawn ahead; collectibles (red squares) and kills award score.
- Camera continuously scrolls; falling into lava gaps or lagging behind the camera is lethal.

## Project facts
- C++17, CMake, SFML 3.x via FetchContent; warnings are `-Wall -Wextra -Werror -Wpedantic`.
- Sanitizers are ON by default (`ENABLE_SANITIZERS=ON`); disable with `-DENABLE_SANITIZERS=OFF` only when necessary.
- Build/run:
  ```bash
  cmake -B build
  cmake --build build
  cmake --build build --target run   # uses ASan/UBSan/LSan env when enabled
  ./build/bin/runner                 # binary is in build/bin
  ```
- Assets are copied post-build; `ResourceManager::init(argv[0])` in `main` must stay first to resolve asset paths.

## Architecture map
- `main` → `Application` → `StateStack` → states (`StateMenu`, `StatePlaying`, `StatePaused`). States are constructed via `StateStack::push`, must return `true` from `init`, and request pops instead of closing the window directly.
- `StatePlaying` owns:
  - `GameSession`: time/score; awards per-second score and kill/collect scores.
  - `World`: owns camera, environment/ground, entities/spawns, collisions, and score forwarding.
  - `GameHUD`: UI-only; reads from player + session; no gameplay logic.
- `World` details:
  - Entities live in `std::vector<std::unique_ptr<Entity>>`; spawn with `createEntity<T>` then call `init()`. Dead entities are culled after updates.
  - Uses `Camera` to follow player and defines catch-up/threshold X; uses window size from `Config`.
  - Environment builds parallax + animated strips and a `GroundStream` collider with lava gaps (falling or being overtaken by camera kills the player).
  - Spawning: demons every 10s just off the right edge; random obstacles/platforms stream ahead of camera; platforms often spawn a `RedSquare` collectible above.
  - Collisions: `World` builds a per-frame `CollisionContext` (`CollisionSystem`) based on `CollisionLayer`/`CollisionMask` and delegates all gameplay collisions there (obstacle pushback/DPS, camera kill zone, lava gaps, projectiles vs actors/collectibles, player pickups). For actors, vertical physics still uses `Actor::applyPhysics(dt, const Collider*)` with a combined `MultiRectCollider` built from ground/obstacle/platform AABBs. Keep collider AABBs accurate (set size/offset when adding entities and assign proper collision layer/mask).
- Gameplay systems:
  - Player uses `PlayerInput` from `StatePlaying`; states include Move/Dash/Cast/Death. Casting currently spawns `SpellId::IceBolt` projectiles (see `SpellCatalog`/`Projectile`).
  - Collision system uses `CollisionLayer`/`CollisionMask` (see `CollisionLayers.h`) and `collision::resolve` (see `CollisionSystem.h/.cpp`); when adding new entity types that participate in collisions, choose an appropriate layer + mask and integrate any new interaction rules into `CollisionSystem`.
  - `ResourceManager` + `Assets` namespace hold all asset keys; do not load textures/fonts ad-hoc.
  - `Animation`/`SpriteAnimator` manage sprite clips; colliders are `RectCollider` attached to sprites.

## Coding style (repo-specific)
- Keep separation: `update(dt)` for logic, `render(target, states)` for drawing; dt is float seconds from `sf::Clock::restart().asSeconds()`.
- Ownership: prefer `std::unique_ptr`; use raw pointers only as non-owning; avoid `new`/`delete`.
- Use const-correctness, pass heavy types by `const&`, small trivially copyable by value.
- Use `<algorithm>` and `reserve()` in per-frame loops where it helps clarity/perf.
- Respect existing names and structure; prefer composition over new inheritance layers.
- Use `Config` for window size/title; avoid hardcoding view values unless matching current pattern.

## Tooling & scripts
- Prefer project scripts over ad-hoc commands:
  - `scripts/clang-format-all.sh` formats all sources using `clang-format` (use when doing larger refactors or when formatting drifts).
  - `scripts/export-all.sh` bundles all `.h`/`.cpp` from `src/` into `scripts/output/src-bundle.txt` for sharing/review.
  - `scripts/export-tree.sh` dumps the directory tree into `scripts/output/tree.txt` (ignores `.git`, `.vscode`, `build`).
- Do not introduce new build tools or dependency managers; extend the existing CMake/SFML setup instead.

## Workflow when editing
1. Restate the request briefly; inspect relevant states/world/entities/UI/utils.
2. Write a short plan for non-trivial changes.
3. Implement following current patterns (state machine, world/entity lifecycle, ResourceManager asset access).
4. Keep code compiling with sanitizer-friendly options; include headers explicitly.

## Using Codex / AI assistants
- Modify files in this repo directly; avoid suggesting manual copy-paste when you can apply the change yourself.
- Keep diffs focused: do not reformat entire files unless the user asks or you run the formatting script.
- When possible, validate significant changes by building (`cmake --build build`) and, if appropriate, running the game.
- Prefer small, composable changes that respect the existing architecture over large rewrites, unless the design is clearly blocking progress and the user agrees to a refactor.

## Things to avoid
- Moving or removing `ResourceManager::init` in `main`.
- Introducing new global mutable singletons; respect existing centralized helpers instead.
- Reloading assets every frame or bypassing `ResourceManager`/`Assets` keys.
- Mixing rendering and logic or skipping `init()`/`isAlive()` flows in entities.
- Ignoring camera/environment when adding colliders (new solids should integrate with combined ground).
