# API Reference

Two entry points, same functionality:

- **`UGameRebellionSubsystem`** — a `UGameInstanceSubsystem`. The full API. Every
  function is `BlueprintCallable`, so Blueprint can reach all of it through a
  **Get GameInstance Subsystem** node.
- **`UGameRebellionBPLibrary`** — static `BlueprintFunctionLibrary` wrappers for
  the most common calls. Each takes a `WorldContextObject` instead of a subsystem
  reference, so the nodes need no plumbing.

```cpp
#include "GameRebellionSubsystem.h"

UGameRebellionSubsystem* GR =
    GetGameInstance()->GetSubsystem<UGameRebellionSubsystem>();
```

The subsystem is absent outside a game instance (commandlets, some editor
contexts) — null-check it.

## Return codes

Every non-`const` function returns `int32`:

| Value | Meaning |
| --- | --- |
| `0` | Success |
| `-1` | Generic failure — check `GetLastError()` |
| `-2` | Invalid arguments |
| `-3` | Invalid state (usually: SDK not initialized) |

## Lifecycle

| Function | Notes |
| --- | --- |
| `int32 InitSDK()` | Initializes from the settings object. Called automatically on game instance start when `ApiKey` is set and `bAutoTrackSession` is on. Safe to call explicitly when it is off. |
| `int32 ShutdownSDK(const FString& EndReason = "normal")` | Flushes and tears down. Called automatically on subsystem deinitialize. |
| `int32 GetState() const` | `0` uninitialized · `1` initializing · `2` ready · `3` paused · `4` flushing · `5` shutting down · `6` error. |
| `FString GetLastError() const` | Last error message from the native SDK, or empty. |
| `FString DrainLogs() const` | Drains the native log buffer. The plugin already does this on a timer into `UE_LOG`; call it only if you want the lines yourself. |

Session start/stop and app deactivate / reactivate / terminate are wired
automatically — you do not need to track sessions by hand.

## Events

| Function | Notes |
| --- | --- |
| `int32 TrackJson(const FString& EventName, const FString& JsonPayload)` | Any event in the schema. `JsonPayload` must be a JSON object string. |
| `int32 TrackProgression(Type, Status, Progression01 = "", Progression02 = "", Progression03 = "")` | `Type` e.g. `"level"`, `"world"`; `Status` e.g. `"start"`, `"complete"`, `"fail"`. The three `Progression*` fields are a hierarchy, coarsest first. |
| `int32 TrackLevelUp(double Level)` | |
| `int32 TrackAchievement(const FString& Id)` | |
| `int32 TrackTransaction(double Amount, const FString& Currency, Type = "", Description = "")` | `Currency` is an ISO 4217 code, e.g. `"USD"`. |
| `int32 TrackLog(Type, Category = "", Message = "", Description = "")` | `Type` e.g. `"error"`, `"warning"`, `"info"`. |

All parameters are `const FString&` unless noted.

## Metrics

| Function | Notes |
| --- | --- |
| `int32 RecordFrame(double Fps)` | Aggregated into FPS metrics — call it on a sampling timer, not every frame. |
| `int32 RecordMemory(double MemoryMB)` | |

## Consent and transport

| Function | Notes |
| --- | --- |
| `int32 SetConsent(bool bGranted)` | Until granted, events are not transmitted. Call as soon as the user's choice is known. |
| `int32 SetPaused(bool bPaused)` | Pauses collection and sending without shutting down. |
| `int32 SetNetworkOnline(bool bOnline)` | Tell the SDK about connectivity you already track, so it can back off instead of retrying. |
| `int32 Flush()` | Sends buffered events now instead of waiting for the batch limits. |

## Blueprint function library

`UGameRebellionBPLibrary` — static nodes, each taking a world context:

| Node | Maps to |
| --- | --- |
| `Track Json` | `TrackJson` |
| `Track Level Up` | `TrackLevelUp` |
| `Track Achievement` | `TrackAchievement` |
| `Track Transaction` | `TrackTransaction` |
| `Set Consent` | `SetConsent` |
| `Flush Events` | `Flush` |
| `Get SDK State` | `GetState` (pure) |
| `Get SDK Last Error` | `GetLastError` (pure) |

For `Track Progression`, `Track Log`, `Record Frame` and `Record Memory`, use the
subsystem node — those have no static wrapper.

## Settings

`UGameRebellionSettings` (`UDeveloperSettings`, `Config=Game`) — **Project
Settings → Plugins → GameRebellion**, persisted to `Config/DefaultGame.ini`
under `[/Script/GameRebellion.GameRebellionSettings]`.

| Setting | Type | Default | Notes |
| --- | --- | --- | --- |
| `ApiKey` | `FString` | *(empty)* | Required. |
| `Environment` | `EGrEnvironment` | `Production` | `Production` · `Staging` · `Development`. |
| `BuildNumber` | `FString` | *(empty)* | Falls back to the project's `BUILD_VERSION`, then `ProjectVersion`. |
| `bAutoTrackSession` | `bool` | `true` | Auto-init on game instance start. |
| `BatchSizeBytes` | `int32` | `65536` | Clamped by the core to 1 KB – 1 MB. |
| `BatchMaxEvents` | `int32` | `100` | Clamped to 1 – 1000. |
| `FlushIntervalMs` | `int32` | `30000` | Clamped to 1 s – 5 min. |
| `bEnableCompression` | `bool` | `false` | Gzip batches before upload. |

The game version reported with events comes from the project's `ProjectVersion`;
there is no separate setting.

Read them at runtime with `GetDefault<UGameRebellionSettings>()`. They are
`BlueprintReadOnly` — change behaviour through the settings or the API, not by
writing to the object.
