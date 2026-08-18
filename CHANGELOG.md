# Changelog

All notable changes to the GameRebellion Unreal Engine plugin.

The plugin carries its own version number and its own release tag
(`unreal-vX.Y.Z`), so it can move independently of the core SDK. Each entry
records the core version the release was built against.

## 1.0.0

First public release of the Unreal Engine plugin. Built against GameRebellion
core 1.0.0.

### Added

- `UGameRebellionSubsystem` (`UGameInstanceSubsystem`) — the SDK entry point.
  Auto-initializes on game instance start when `ApiKey` is set and
  `bAutoTrackSession` is on.
- Blueprint access via `UGameRebellionBPLibrary` static wrappers, plus every
  subsystem function directly through a **Get GameInstance Subsystem** node.
- Typed events: `TrackProgression`, `TrackLevelUp`, `TrackAchievement`,
  `TrackTransaction`, `TrackLog`. Generic `TrackJson` for any other event in the
  schema.
- Metrics: `RecordFrame`, `RecordMemory`.
- Automatic session lifecycle tracking, wired to UE's app deactivate /
  reactivate / terminate delegates.
- Project Settings page under **Plugins → GameRebellion**
  (`UGameRebellionSettings`, persisted to `Config/DefaultGame.ini`).
- Consent and transport control: `SetConsent`, `SetPaused`, `SetNetworkOnline`,
  `Flush`, `GetState`, `GetLastError`.
- Native SDK diagnostics drained into `UE_LOG`, so transport and pipeline
  messages appear in the engine log, `adb logcat`, and the Xcode console.
- Platform support: Win64, Mac (universal), Linux x64, Android arm64-v8a, iOS.

### Notes

- **Source-only distribution.** The plugin ships no `Binaries/` and carries no
  `EngineVersion` field in its `.uplugin`, so it is not pinned to the engine
  version it was built against — your project's UBT compiles it. Verified
  against UE 5.5.
- **iOS** links the SDK statically at compile time (Apple disallows `dlopen` of
  non-embedded code). All other platforms load the native library at runtime.
- **Android** bundles `libssl.so` / `libcrypto.so`, staged into the APK by the
  plugin's UPL. Android ships no loadable public OpenSSL, and the .NET crypto
  shim `dlopen`s these on the first HTTPS request.
- Android ships arm64-v8a only.
