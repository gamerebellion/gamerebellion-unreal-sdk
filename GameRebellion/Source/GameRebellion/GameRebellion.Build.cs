using UnrealBuildTool;
using System.IO;

public class GameRebellion : ModuleRules
{
	public GameRebellion(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"DeveloperSettings",
			"Projects",
			"RHI"
		});

		string ThirdPartyRoot = Path.GetFullPath(
			Path.Combine(ModuleDirectory, "..", "..", "ThirdParty", "GameRebellionCore"));

		PublicIncludePaths.Add(Path.Combine(ThirdPartyRoot, "include"));

		// Runtime-loaded via FPlatformProcess::GetDllHandle -- no link-time dependency.
		// Just ensure the native binaries get packaged.
		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			RuntimeDependencies.Add(Path.Combine(ThirdPartyRoot, "Win64", "GameRebellion.Core.dll"));
		}
		else if (Target.Platform == UnrealTargetPlatform.Linux)
		{
			RuntimeDependencies.Add(Path.Combine(ThirdPartyRoot, "Linux", "GameRebellion.Core.so"));
		}
		else if (Target.Platform == UnrealTargetPlatform.Mac)
		{
			RuntimeDependencies.Add(Path.Combine(ThirdPartyRoot, "Mac", "GameRebellion.Core.dylib"));
		}
		else if (Target.Platform == UnrealTargetPlatform.Android)
		{
			string ArmDir = Path.Combine(ThirdPartyRoot, "Android", "arm64-v8a");
			RuntimeDependencies.Add(Path.Combine(ArmDir, "GameRebellion.Core.so"));

			string JniSo = Path.Combine(ArmDir, "libgr-android-adapter.so");
			if (File.Exists(JniSo))
			{
				RuntimeDependencies.Add(JniSo);
			}

			string AplPath = Path.Combine(ModuleDirectory, "GameRebellion_APL.xml");
			AdditionalPropertiesForReceipt.Add("AndroidPlugin", AplPath);
		}
		else if (Target.Platform == UnrealTargetPlatform.IOS)
		{
			// iOS is the one platform that links the SDK STATICALLY: Apple forbids
			// dlopen of non-embedded code and NativeAOT-iOS emits a static archive.
			// The loader binds the gr_* symbols directly (see
			// GameRebellionNativeLibrary.cpp), so a missing symbol is a link error
			// here, never a runtime failure. No RuntimeDependencies: nothing to stage.
			string IosDir = Path.Combine(ThirdPartyRoot, "iOS");

			// ILC-compiled managed core (gr_* exports live here).
			PublicAdditionalLibraries.Add(Path.Combine(IosDir, "libGameRebellion.Core.a"));

			// NativeAOT runtime + platform shims the core archive depends on.
			// This exact set comes from the undefined-symbol audit performed by
			// scripts/build-unreal-aot-ios.sh (which vendors the files here).
			// Globalization/ICU is absent by design (InvariantGlobalization).
			PublicAdditionalLibraries.Add(Path.Combine(IosDir, "libbootstrapperdll.o"));
			PublicAdditionalLibraries.Add(Path.Combine(IosDir, "libRuntime.WorkstationGC.a"));
			PublicAdditionalLibraries.Add(Path.Combine(IosDir, "libeventpipe-disabled.a"));
			PublicAdditionalLibraries.Add(Path.Combine(IosDir, "libstandalonegc-disabled.a"));
			PublicAdditionalLibraries.Add(Path.Combine(IosDir, "libstdc++compat.a"));
			PublicAdditionalLibraries.Add(Path.Combine(IosDir, "libSystem.Native.a"));
			PublicAdditionalLibraries.Add(Path.Combine(IosDir, "libSystem.Security.Cryptography.Native.Apple.a"));
			PublicAdditionalLibraries.Add(Path.Combine(IosDir, "libSystem.Net.Security.Native.a"));

			// System dependencies of the shim archives: Security for AppleCrypto
			// (TLS/X509), GSS for NetSecurityNative (Negotiate auth), c++ for
			// libstdc++compat. Verified by a clean clang link of a test binary.
			PublicFrameworks.AddRange(new string[] { "Foundation", "CoreFoundation", "Security", "GSS" });
			PublicSystemLibraries.Add("c++");
		}
	}
}
