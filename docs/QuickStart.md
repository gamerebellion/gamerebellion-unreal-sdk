# Quick Start

Assumes the plugin is installed — see [Installation.md](Installation.md).

## Configure

**Project Settings → Plugins → GameRebellion** → set **Api Key**. That is the
only required setting; everything else has a production-ready default.

Settings persist to `Config/DefaultGame.ini`, so you can also edit them directly
or set them from CI:

```ini
[/Script/GameRebellion.GameRebellionSettings]
ApiKey=your-api-key-here
Environment=Production
```

With `ApiKey` set and `bAutoTrackSession` on (the default), the SDK initializes
with your game instance and session tracking starts immediately. There is no
`Initialize()` call to make.

To initialize manually instead, set `bAutoTrackSession=False` and call `InitSDK()`
when you're ready — for example after your own consent prompt.

## Track an event — C++

Add `GameRebellion` to your module's dependencies in `YourModule.Build.cs`:

```csharp
PrivateDependencyModuleNames.AddRange(new string[] { "GameRebellion" });
```

Then:

```cpp
#include "GameRebellionSubsystem.h"

void AMyGameMode::OnLevelComplete(int32 WorldIndex, int32 LevelIndex)
{
    if (UGameRebellionSubsystem* GR =
            GetGameInstance()->GetSubsystem<UGameRebellionSubsystem>())
    {
        GR->TrackProgression(
            TEXT("level"), TEXT("complete"),
            FString::Printf(TEXT("world_%d"), WorldIndex),
            FString::Printf(TEXT("level_%d"), LevelIndex));
    }
}
```

Always null-check the subsystem: it does not exist outside a game instance
(commandlets, some editor contexts).

## Track an event — Blueprint

Two options:

**Static nodes** — search for `Track Json`, `Track Level Up`,
`Track Achievement`, `Track Transaction`, `Set Consent`, `Flush Events`,
`Get SDK State`, `Get SDK Last Error`. These take care of finding the subsystem
for you.

**Subsystem node** — for the full API (including `Track Progression`,
`Track Log`, `Record Frame`, `Record Memory`), drop a **Get GameInstance
Subsystem** node, set its class to **GameRebellionSubsystem**, and drag off the
return value.

## Common calls

```cpp
// Progression
GR->TrackProgression(TEXT("level"), TEXT("start"), TEXT("world_1"), TEXT("level_5"));
GR->TrackProgression(TEXT("level"), TEXT("fail"),  TEXT("world_1"), TEXT("level_5"));
GR->TrackLevelUp(12);
GR->TrackAchievement(TEXT("first_blood"));

// Monetization
GR->TrackTransaction(4.99, TEXT("USD"), TEXT("iap"), TEXT("gem_pack_500"));

// Diagnostics
GR->TrackLog(TEXT("error"), TEXT("netcode"), TEXT("Desync detected"), TEXT("room=42"));
GR->RecordFrame(58.4);
GR->RecordMemory(1280.0);

// Anything else in the schema
GR->TrackJson(TEXT("boss_defeated"),
    TEXT("{\"boss\":\"dragon\",\"time_sec\":142,\"attempts\":7}"));
```

Every `Track*` call returns an `int32` result code — `0` on success. They are
non-blocking: events are buffered and sent in batches.

## Consent

If your game needs explicit analytics consent, gate it:

```cpp
GR->SetConsent(bUserAgreed);
```

Call it as soon as you know the answer. Until consent is granted, events are not
transmitted.

## Verify it works

Watch the log on first launch — the plugin drains the native SDK's diagnostics
into `UE_LOG`:

```
[GameRebellion] Native library loaded → gr_initialize OK
→ [GRC][TLS] Loaded N embedded CA root(s)     (mobile only)
→ [Batcher] Timer fired → [Batcher] Sent batch
→ [Transport] Batch sent successfully
```

On mobile:

```bash
adb logcat | grep -E "GameRebellion|GRC"     # Android
```

Nothing arriving? Force a send and read the error:

```cpp
GR->Flush();
UE_LOG(LogTemp, Warning, TEXT("GR state=%d err=%s"),
    GR->GetState(), *GR->GetLastError());
```

`GetState()` returns `0` uninitialized, `1` initializing, `2` ready, `3` paused,
`4` flushing, `5` shutting down, `6` error.

## Staging builds

`Environment` is a config value, so point QA builds at Staging through UE's
config hierarchy — no code change:

```ini
; Config/Development/DefaultGame.ini
[/Script/GameRebellion.GameRebellionSettings]
Environment=Staging
```

Next: [API.md](API.md) for the full surface, [Platforms.md](Platforms.md) for
per-platform notes.
