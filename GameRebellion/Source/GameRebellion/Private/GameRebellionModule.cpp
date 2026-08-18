#include "GameRebellionModule.h"
#include "GameRebellionNativeLibrary.h"
#include "CoreGlobals.h"

#define LOCTEXT_NAMESPACE "FGameRebellionModule"

void FGameRebellionModule::StartupModule()
{
	// Analytics has no role in headless commandlets (cook, build, resave, etc.),
	// and loading the desktop native binary there is both pointless and fragile:
	// the cook commandlet is a host-OS (e.g. Mac) process that would load the
	// host dylib even when cooking for another platform, and any load failure
	// logged at Error level FAILS THE COOK (UE aborts a cook on any logged error).
	// A quarantined/missing/mis-signed desktop binary must never break an iOS/
	// Android cook. So skip the native load entirely in commandlets.
	if (IsRunningCommandlet())
	{
		return;
	}

	if (FGameRebellionNativeLibrary::Load())
	{
		UE_LOG(LogTemp, Log, TEXT("[GameRebellion] Native library loaded"));
	}
	else
	{
		// Warning, not Error: analytics being unavailable is not fatal to the
		// game, and Error would fail any (future) commandlet path that reaches here.
		UE_LOG(LogTemp, Warning, TEXT("[GameRebellion] Native library unavailable: %s"),
			*FGameRebellionNativeLibrary::GetLoadError());
	}
}

void FGameRebellionModule::ShutdownModule()
{
	FGameRebellionNativeLibrary::Unload();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FGameRebellionModule, GameRebellion)
