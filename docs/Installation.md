# Installation

The GameRebellion plugin is a **project plugin**: it lives in your project's
`Plugins/` folder, not in the engine.

## Requirements

- Unreal Engine 5.x. The plugin ships source-only with no `EngineVersion` pin,
  so your engine compiles it — there is no per-engine-version download.
- A C++ toolchain for your target platforms (the same one you already need to
  build a UE project). A Blueprint-only project will be converted to a C++
  project the first time it compiles the plugin; the editor offers to do this.
- A GameRebellion API key.

## Option 1 — release zip (recommended)

1. Download `GameRebellion-UE-<version>.zip` from
   [Releases](https://github.com/gamerebellion/gamerebellion-unreal-sdk/releases).
2. Create a `Plugins` folder in your project root if it doesn't exist.
3. Unzip into it:

```bash
unzip GameRebellion-UE-<version>.zip -d MyProject/Plugins/
```

Resulting hierarchy:

```
MyProject
├── MyProject.uproject
└── Plugins
    └── GameRebellion
        ├── GameRebellion.uplugin
        ├── Source/
        ├── Config/
        ├── Resources/
        └── ThirdParty/
```

Each release also publishes a `.sha256` file if you want to verify the download:

```bash
shasum -a 256 -c GameRebellion-UE-<version>.zip.sha256
```

## Option 2 — git submodule

Useful if you want to track the SDK in version control and update with a `git
pull`. UE scans `Plugins/` recursively, so the extra directory level is fine:

```bash
cd MyProject
git submodule add https://github.com/gamerebellion/gamerebellion-unreal-sdk \
    Plugins/GameRebellionSDK
git submodule update --init
```

The plugin is then at `Plugins/GameRebellionSDK/GameRebellion/` and the editor
finds it normally.

> Do **not** use the repo's "Download ZIP" button as a substitute for a release
> asset if your git client is unusual — the release zip is the supported path and
> contains exactly the plugin folder.

## Option 3 — copy the folder

Clone or download the repo and copy the `GameRebellion/` folder into
`MyProject/Plugins/`. You need the **whole folder** — `Source/` and
`ThirdParty/` included. The `.uplugin` file alone is not enough.

## First build

1. Open your `.uproject`. The editor detects a plugin module that needs
   compiling and offers to rebuild — accept. This compiles `Source/` against the
   native libraries in `ThirdParty/`.
2. If you'd rather build ahead of time, regenerate project files and build the
   editor target as usual (`GenerateProjectFiles` / your IDE), then open the
   project.
3. Confirm the plugin is enabled: **Edit → Plugins → Analytics → GameRebellion
   Analytics SDK**.

Next: [QuickStart.md](QuickStart.md).

## Troubleshooting

**"Plugin 'GameRebellion' failed to load because module could not be found"**
The module was not compiled. Close the editor, delete `Binaries/` and
`Intermediate/` in your **project** (not the plugin), and reopen — accept the
rebuild prompt.

**iOS link errors naming `gr_*` symbols**
`ThirdParty/GameRebellionCore/iOS/` is incomplete. It must contain
`libGameRebellion.Core.a` plus eight runtime archives. This is the usual symptom
of copying only part of the plugin folder, or of a Git LFS clone that fetched
pointer files instead of binaries — check that
`ThirdParty/GameRebellionCore/iOS/libGameRebellion.Core.a` is tens of MB, not
~130 bytes.

**Events never arrive, no errors**
The SDK will not auto-initialize without an `ApiKey`. Check the log for
`[GameRebellion] Auto-init skipped`, then see
[QuickStart.md](QuickStart.md#configure).

**Android crashes on the first network call**
`libssl.so` / `libcrypto.so` are missing from
`ThirdParty/GameRebellionCore/Android/arm64-v8a/`. See
[Platforms.md](Platforms.md#android).
