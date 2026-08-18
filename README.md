# GameRebellion

[Documentation](https://docs.gamerebellion.com/) · [Unity SDK](https://github.com/gamerebellion/gamerebellion-unity-sdk) · [Unreal SDK](https://github.com/gamerebellion/gamerebellion-unreal-sdk) · [S2S API](https://docs.gamerebellion.com/sdk/sdk-integrations/server-to-server)

Game analytics SDK that tracks in-game events and contextualizes your data against **400,000+ titles** across the gaming industry — progression benchmarks, monetization baselines, genre comparisons, and real-time dashboards.

Unreal Engine 5, source-only plugin. Win64 · Mac · Linux · Android · iOS.

#### Initialize and track events in your game

Initialization is configuration, not code. Set your API key in
**Project Settings → Plugins → GameRebellion** and the SDK starts with your game.

```ini
; Config/DefaultGame.ini
[/Script/GameRebellion.GameRebellionSettings]
ApiKey=YOUR_API_KEY
Environment=Production
```

```cpp
#include "GameRebellionSubsystem.h"

UGameRebellionSubsystem* GR =
    GetGameInstance()->GetSubsystem<UGameRebellionSubsystem>();

// Track a level completion with progression context
GR->TrackProgression(TEXT("level"), TEXT("complete"), TEXT("world_1"), TEXT("level_5"));

// Track a purchase
GR->TrackTransaction(4.99, TEXT("USD"), TEXT("iap"), TEXT("gem_pack_500"));

// Track any custom event
GR->TrackJson(TEXT("boss_defeated"), TEXT("{\"boss\":\"dragon\",\"time_sec\":142}"));
```

From Blueprint, use the **GameRebellion** nodes — either the static
`UGameRebellionBPLibrary` wrappers (`Track Json`, `Track Level Up`,
`Track Achievement`, `Track Transaction`, `Set Consent`, `Flush Events`) or any
subsystem function via a **Get GameInstance Subsystem → GameRebellionSubsystem**
node.

---

## In-Game Events

26 event types across 6 categories. Track what matters, ignore what doesn't.

| Category | Events | Example |
| --- | --- | --- |
| **Session** | session_start · session_stop · login · logout | Automatic lifecycle tracking |
| **Progression** | progression · level_up · achievement | Player journey and milestones |
| **Monetization** | transaction · crypto_transaction | IAP, subscriptions, crypto payments |
| **Marketing** | impression · click · conversion · creator_code_use | Attribution and campaign tracking |
| **Social** | friend_invite · group_join · chat_message · voice_call | Community and multiplayer signals |
| **Technical** | FPS · memory · log · health | Performance and stability metrics |

The plugin ships typed helpers for the most common ones —
`TrackProgression`, `TrackLevelUp`, `TrackAchievement`, `TrackTransaction`,
`TrackLog`, `RecordFrame`, `RecordMemory` — and session events are automatic.
Everything else goes through `TrackJson("event_name", jsonPayload)`, which
accepts any event in the schema.

---

## Integrations

| Engine / Method | Status | Package |
| --- | --- | --- |
| **Unity** | Available | [com.gamerebellion.sdk](https://github.com/gamerebellion/gamerebellion-unity-sdk) via UPM or `.unitypackage` |
| **Unreal Engine** | Available | [gamerebellion-unreal-sdk](https://github.com/gamerebellion/gamerebellion-unreal-sdk) — UE5 plugin, drop-in or git submodule |
| **Server-to-Server** | Available | [REST API](https://docs.gamerebellion.com/sdk/sdk-integrations/server-to-server) — any language, any engine |
| **Godot · Native Mobile · Web · Flutter** | Planned | Use S2S API in the meantime |

---

## Quick Start

**1. Install** — copy the `GameRebellion/` folder from this repo into your
project's `Plugins/` directory:

```
MyProject
└── Plugins
    └── GameRebellion
        └── GameRebellion.uplugin
```

Grab it from the [latest release](https://github.com/gamerebellion/gamerebellion-unreal-sdk/releases)
zip, or add the repo as a submodule:

```bash
git submodule add https://github.com/gamerebellion/gamerebellion-unreal-sdk \
    MyProject/Plugins/GameRebellionSDK
```

You need the **whole folder** — `Source/` and `ThirdParty/` included. The
`.uplugin` alone is not enough. This is a source-only plugin with no
`EngineVersion` pin, so it builds against any UE 5.x; your engine compiles it
the first time you open the project.

**2. Configure** — Project Settings → **Plugins → GameRebellion** → set `ApiKey`.
That's the only required setting.

**3. Track** — start sending events:

```cpp
UGameRebellionSubsystem* GR =
    GetGameInstance()->GetSubsystem<UGameRebellionSubsystem>();

GR->TrackProgression(TEXT("level"), TEXT("complete"), TEXT("world_1"));
```

Session tracking has already started by this point — `bAutoTrackSession` is on
by default, so the SDK initializes with your game instance as soon as `ApiKey`
is set.

---

## Configuration

Settings live in **Project Settings → Plugins → GameRebellion**, and persist to
`Config/DefaultGame.ini` under
`[/Script/GameRebellion.GameRebellionSettings]`. Defaults are production-ready —
most games only ever set `ApiKey`.

### Environments

The SDK targets **Production** by default. Point QA or internal builds at
Staging using UE's config hierarchy — no code change and no separate build path:

```ini
; Config/DefaultGame.ini — shipped game
[/Script/GameRebellion.GameRebellionSettings]
ApiKey=YOUR_API_KEY
Environment=Production
```

```ini
; Config/Development/DefaultGame.ini — QA / internal build
[/Script/GameRebellion.GameRebellionSettings]
Environment=Staging
```

### Settings reference

| Setting | Default | What it does |
| --- | --- | --- |
| `ApiKey` | *(empty)* | **Required.** The SDK will not auto-initialize without it. |
| `Environment` | `Production` | Backend the SDK talks to — `Production` / `Staging` / `Development`. |
| `BuildNumber` | *(empty)* | Build identifier reported with events. Empty = the project's `BUILD_VERSION`, then `ProjectVersion`. |
| `bAutoTrackSession` | `true` | Initializes the SDK on game instance start when `ApiKey` is set. Set `false` to call `InitSDK()` yourself. |
| `BatchSizeBytes` | `65536` | Max size of one event batch before sending. Clamped to 1 KB – 1 MB. |
| `BatchMaxEvents` | `100` | Max events per batch before sending. Clamped to 1 – 1000. |
| `FlushIntervalMs` | `30000` | How often buffered events are flushed even if batch limits aren't reached. Clamped to 1 s – 5 min. |
| `bEnableCompression` | `false` | Gzip event batches before upload. |

The game version reported with events comes from your project's
`ProjectVersion` — there is no separate setting for it.

Batching defaults are tuned for production; lowering them gives fresher data at
the cost of more network and battery use.

### Logs

The plugin drains the native SDK's diagnostics into `UE_LOG` automatically, so
transport and pipeline messages show up in the normal engine log (and in
`adb logcat` / the Xcode console on mobile). On first launch you should see:

```
[GameRebellion] Native library loaded → gr_initialize OK
→ [Batcher] Timer fired → [Batcher] Sent batch
→ [Transport] Batch sent successfully
```

If events aren't arriving, check `GetLastError()` and `GetState()`.

---

## Platform notes

- **Win64 / Mac / Linux / Android** — the native library is loaded at runtime.
  If `ThirdParty/GameRebellionCore/<Platform>/` is missing your platform's
  binary, `gr_initialize` fails and `GetLastError()` says so.
- **iOS** — statically linked at compile time (Apple disallows `dlopen` of
  non-embedded code), handled automatically by `GameRebellion.Build.cs`.
- **Android** — ships `libssl.so` / `libcrypto.so`, staged into the APK by the
  plugin's UPL. HTTPS won't work without them.

See [docs/Platforms.md](docs/Platforms.md) for detail.

---

Full integration guide at **[docs.gamerebellion.com](https://docs.gamerebellion.com)**.
