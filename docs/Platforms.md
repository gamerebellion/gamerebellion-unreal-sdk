# Platform Notes

| Platform | Architecture | Linkage | Shipped as |
| --- | --- | --- | --- |
| Win64 | x64 | runtime (`LoadLibrary`) | `GameRebellion.Core.dll` |
| Mac | universal (arm64 + x64) | runtime (`dlopen`) | `GameRebellion.Core.dylib` |
| Linux | x64 | runtime (`dlopen`) | `GameRebellion.Core.so` |
| Android | arm64-v8a | runtime (`dlopen`) | `GameRebellion.Core.so` + OpenSSL pair |
| iOS | arm64 (device) | **static, at compile time** | `libGameRebellion.Core.a` + runtime archives |

The native libraries live in
`GameRebellion/ThirdParty/GameRebellionCore/<Platform>/` and are wired up by
`Source/GameRebellion/GameRebellion.Build.cs`.

## Desktop — Win64 / Mac / Linux

Nothing to configure. The library is loaded on module startup and staged into
packaged builds automatically.

If `ThirdParty/GameRebellionCore/<Platform>/` is missing your platform's binary,
`gr_initialize` fails and `GetLastError()` reports it — the game keeps running,
it just sends nothing. Check the log for
`[GameRebellion] Native library not loaded`.

## Android

**Architecture:** arm64-v8a only. Make sure **Project Settings → Platforms →
Android → Support arm64** is enabled. `armv7` is not supported.

**The bundled OpenSSL matters.** `ThirdParty/GameRebellionCore/Android/arm64-v8a/`
contains `libssl.so` and `libcrypto.so` alongside the core library. These are not
optional: Android ships no loadable public OpenSSL, and .NET's crypto shim
`dlopen`s them on the first TLS handshake. Without them the process aborts on the
first HTTPS request. The plugin's UPL (`Source/GameRebellion/GameRebellion_APL.xml`)
stages them into the APK for you.

**UPL changes need a clean Android intermediate.** UE caches the generated gradle
project and manifest, and re-applies UPL only when that cache is regenerated. If
you have modified the plugin's UPL — or you are debugging a missing library in the
APK — delete `Intermediate/Android` in your project before repackaging. An
incremental repackage silently ships the previous manifest and library set.

**Verifying:**

```bash
adb logcat | grep -E "GameRebellion|GRC"
```

Look for `[GRC][TLS] Loaded N embedded CA root(s)` — that means the TLS path came
up and the OpenSSL pair was found.

## iOS

**Statically linked.** Apple disallows `dlopen` of code that isn't embedded in
the app, so on iOS the SDK is linked at compile time. Consequences:

- `ThirdParty/GameRebellionCore/iOS/` must be complete or **your build fails to
  link** — it is not a silent runtime degradation like the other platforms. It
  needs `libGameRebellion.Core.a` plus eight NativeAOT runtime archives
  (`libbootstrapperdll.o`, `libRuntime.WorkstationGC.a`, `libSystem.Native.a`,
  `libSystem.Net.Security.Native.a`,
  `libSystem.Security.Cryptography.Native.Apple.a`, `libeventpipe-disabled.a`,
  `libstandalonegc-disabled.a`, `libstdc++compat.a`).
- Undefined `gr_*` symbols at link time mean that directory is incomplete. The
  usual cause is copying only part of the plugin folder, or a Git LFS clone that
  produced pointer files — `libGameRebellion.Core.a` should be tens of MB, not
  ~130 bytes.
- Link time increases noticeably on the first build.

**Device vs simulator.** The shipped archives are built for arm64 devices.

**Verifying:** watch the Xcode console for the same sequence as other platforms,
beginning with `gr_initialize OK`.

## Data directory

On sandboxed platforms (Android, iOS) the SDK's durable event cache needs a
writable directory, set before initialization. The plugin handles this for you
using the project's saved directory — no action needed.

## Consent and privacy

The plugin transmits nothing until consent is granted where your jurisdiction
requires it — call `SetConsent(true)` once the user's choice is known. Obtaining
consent is your responsibility as the integrating developer; see the
[LICENSE](../LICENSE.md) and
[Privacy Policy](https://gamerebellion.com/privacy-policy).

## Engine versions

The plugin ships source-only with no `EngineVersion` field in its `.uplugin`, so
it is not pinned to a particular engine build — your project's UBT compiles it
against whatever engine you have. Developed and verified against **UE 5.5**. The
native libraries are plain C ABI and carry no engine dependency.

If a future engine version breaks compilation, it will break at build time with a
normal C++ error, not at runtime. Please
[open an issue](https://github.com/gamerebellion/gamerebellion-unreal-sdk/issues)
with the engine version and the error.
