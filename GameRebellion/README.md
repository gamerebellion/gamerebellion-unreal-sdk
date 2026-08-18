# GameRebellion Analytics SDK — Unreal Plugin

Cross-platform analytics SDK: session tracking, event reporting, and game
telemetry. Supports Win64, Mac, Linux, Android, and iOS.

## Install

1. Copy this whole folder (as `GameRebellion/`) into your project's `Plugins/`
   directory — e.g. `YourProject/Plugins/GameRebellion/`. You need the entire
   folder: `GameRebellion.uplugin`, `Source/`, and `ThirdParty/` (the native
   libraries). The `.uplugin` file alone is not enough. This is a source-only
   plugin — there is no `Binaries/` folder, and no `EngineVersion` pin, so it
   works against any UE 5.x version; your engine compiles `Source/` itself
   the first time you open the project.
2. Open your `.uproject`. The editor will offer to build the module the
   first time — accept, this compiles `Source/` locally against
   `ThirdParty/`.
3. Confirm the plugin is enabled: **Edit → Plugins → Analytics → GameRebellion
   Analytics SDK**.

## Configure

Project Settings → **Plugins → GameRebellion** (or edit
`Config/DefaultGame.ini` directly under `[/Script/GameRebellion.GameRebellionSettings]`):

| Setting | Notes |
|---|---|
| `ApiKey` | Required. The SDK won't auto-init without it. |
| `Environment` | `Production` (default) / `Staging` / `Development`. |
| `bAutoTrackSession` | Default `true` — auto-initializes the SDK on game instance start if `ApiKey` is set. Set `false` to call `InitSDK()` manually instead. |
| `BatchSizeBytes` / `BatchMaxEvents` / `FlushIntervalMs` | Event batching tuning; defaults are reasonable for most games. |
| `BuildNumber` | Optional; falls back to your project's `BUILD_VERSION` / `ProjectVersion` if left empty. |

Example `DefaultGame.ini` entry:
```ini
[/Script/GameRebellion.GameRebellionSettings]
ApiKey=your-api-key-here
Environment=Production
```

## Usage

The SDK is exposed as a `UGameInstanceSubsystem`, callable from Blueprint or
C++. Grab it via `GetGameInstance()->GetSubsystem<UGameRebellionSubsystem>()`,
or use the static Blueprint-function-library wrappers
(`UGameRebellionBPLibrary`, take a `WorldContextObject` instead).

Common calls (same set on both APIs):
- `TrackJson(EventName, JsonPayload)` — generic event
- `TrackLevelUp(Level)`, `TrackAchievement(Id)`, `TrackTransaction(Amount, Currency, Type, Description)`
- `SetConsent(bGranted)`, `Flush()`, `GetState()`, `GetLastError()`

If `bAutoTrackSession` is on and `ApiKey` is set, session tracking starts
automatically — no manual `InitSDK()` call needed for the common case.

## Platform notes

- **Win64 / Mac / Linux / Android**: the native library is loaded at runtime
  via `dlopen`/`LoadLibrary`. If `ThirdParty/GameRebellionCore/<Platform>/`
  is missing the binary for your platform, the SDK will fail silently at
  `gr_initialize` — check `GetLastError()` / the log if events aren't
  reaching the backend.
- **iOS**: the SDK is statically linked at compile time (Apple disallows
  dlopen of non-embedded code), so it must successfully link during your
  build — this happens automatically via `Source/GameRebellion/GameRebellion.Build.cs`.
- **Android**: requires the bundled `libssl.so` / `libcrypto.so` in
  `ThirdParty/GameRebellionCore/Android/arm64-v8a/` (HTTPS won't work
  without them) — these are staged into the APK automatically via the
  plugin's `GameRebellion_APL.xml`.

## How to verify integration succeeded

Watch your log (Xcode console / `adb logcat | grep -E "GameRebellion|GRC"` /
desktop log window) for this sequence on first launch:

```
[GameRebellion] Native library loaded → gr_initialize OK
→ [GRC][TLS] Loaded N embedded CA root(s)   (mobile only)
→ [Batcher] Timer fired → [Batcher] Sent batch
→ [Transport] Batch sent successfully
```

Then confirm on the dashboard: a `session_start` event arrives with the
correct `"platform"` value and real device/OS info — not a placeholder.

| Platform | Confirms success | Platform label to check |
|---|---|---|
| **Win64** | DLL loads from `ThirdParty/Win64/`; play a level → events on dashboard | `"platform":"pc"` |
| **Mac** | Run the actual staged `.app` (not a bare archive copy); play a level | `"platform":"pc"` |
| **Linux** | `.so` loads on target distro; play a level | `"platform":"pc"`, Linux OS string |
| **Android** | `adb shell run-as <pkg> ls lib/arm64-v8a/` shows `libGameRebellion.Core.so` + `libssl.so` + `libcrypto.so`; fresh install online shows the TLS log line above with no SIGABRT | `"platform":"android"`, real model/OS — never `"Linux PC"` |
| **iOS** | Static link succeeds at build time (a missing runtime archive fails the *build*, not runtime); fresh install online reaches `READY` | `"platform":"ios"`, real model/OS/resolution/timezone |

Two other checks worth doing on every platform before calling it done:
- **Offline → kill → relaunch**: generate events offline, force-kill the app,
  reconnect and relaunch — queued events should replay with no duplicates.
- **`GetSDKState()`** should move `UNINITIALIZED → INITIALIZING → READY`
  and never get stuck in `INITIALIZING` while offline.

(Full QA test matrix, including edge cases and known regressions per
platform: `docs/TEST_CASES.md` in the SDK repo, sections 4–5.)

## Troubleshooting

- `GetSDKLastError()` / `GetLastError()` returns the native SDK's last error
  string — check this first if events aren't sending.
- No events arriving at all: confirm `ApiKey` is set and `Environment`
  matches the backend you're checking (events sent to `Staging` won't show
  up in `Production` dashboards).
