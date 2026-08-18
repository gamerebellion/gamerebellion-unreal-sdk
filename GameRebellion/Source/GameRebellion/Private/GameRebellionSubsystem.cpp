#include "GameRebellionSubsystem.h"
#include "GameRebellionNativeLibrary.h"
#include "GameRebellionNative.h"
#include "Misc/CoreDelegates.h"
#include "Misc/App.h"
#include "Misc/ConfigCacheIni.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformMemory.h"
#include "HAL/PlatformProcess.h"
#include "Misc/Paths.h"
#include "RHI.h"

#include <cmath>

// Shorthand -- returns nullptr for empty FString, else UTF8 pointer.
// WARNING: the TCHAR_TO_UTF8 result is a temporary; only valid in the
// same statement.  For multi-arg native calls that is fine because each
// TCHAR_TO_UTF8 lives until the end of the full-expression.
#define GR_STR(s) ((s).IsEmpty() ? nullptr : TCHAR_TO_UTF8(*(s)))

// UE has no FApp::GetProjectVersion(); the configured ProjectVersion lives in
// GeneralProjectSettings (DefaultGame.ini). Read it from GConfig.
static FString GetConfiguredProjectVersion()
{
	FString ProjectVersion;
	GConfig->GetString(
		TEXT("/Script/EngineSettings.GeneralProjectSettings"),
		TEXT("ProjectVersion"), ProjectVersion, GGameIni);
	return ProjectVersion;
}

// FApp::GetBuildVersion() returns the engine branding string ("++UE5+Release-...")
// unless the project overrides BUILD_VERSION, so by itself it cannot identify a
// game build. Prefer the explicit setting, then a project-defined BUILD_VERSION,
// then ProjectVersion.
static FString GetConfiguredBuildNumber()
{
	const UGameRebellionSettings* Settings = GetDefault<UGameRebellionSettings>();
	if (Settings && !Settings->BuildNumber.IsEmpty())
	{
		return Settings->BuildNumber;
	}

	FString BuildVersion = FApp::GetBuildVersion();
	if (!BuildVersion.IsEmpty() && !BuildVersion.StartsWith(TEXT("++")))
	{
		return BuildVersion;
	}

	return GetConfiguredProjectVersion();
}

void UGameRebellionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const UGameRebellionSettings* Settings = GetDefault<UGameRebellionSettings>();
	if (Settings && Settings->bAutoTrackSession && !Settings->ApiKey.IsEmpty())
	{
		InitSDK();
	}
	else
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[GameRebellion] Auto-init skipped (bAutoTrackSession=%s, ApiKey %s) -- "
				 "Track* calls will silently fail until InitSDK() is called explicitly. "
				 "Configure Project Settings -> Plugins -> GameRebellion."),
			(Settings && Settings->bAutoTrackSession) ? TEXT("true") : TEXT("false"),
			(Settings && !Settings->ApiKey.IsEmpty()) ? TEXT("set") : TEXT("empty"));
	}
}

void UGameRebellionSubsystem::Deinitialize()
{
	if (bInitialized)
	{
		ShutdownSDK(TEXT("subsystem_deinitialize"));
	}
	Super::Deinitialize();
}

int32 UGameRebellionSubsystem::InitSDK()
{
	if (!FGameRebellionNativeLibrary::IsLoaded())
	{
		UE_LOG(LogTemp, Error, TEXT("[GameRebellion] Native library not loaded"));
		return GR_ERR_INVALID_STATE;
	}

	const UGameRebellionSettings* Settings = GetDefault<UGameRebellionSettings>();
	if (!Settings || Settings->ApiKey.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("[GameRebellion] ApiKey not configured"));
		return GR_ERR_INVALID_ARGS;
	}

	FTCHARToUTF8 ApiKeyUtf8(*Settings->ApiKey);
	FString GameVer = GetConfiguredProjectVersion();
	FString BuildVer = GetConfiguredBuildNumber();
	FTCHARToUTF8 GameVerUtf8(*GameVer);
	FTCHARToUTF8 BuildVerUtf8(*BuildVer);

	GrConfig Cfg = {};
	Cfg.ApiKey           = ApiKeyUtf8.Get();
	Cfg.GameVersion      = GameVerUtf8.Get();
	Cfg.BuildNumber      = BuildVerUtf8.Get();
	Cfg.Environment      = static_cast<int32>(Settings->Environment);
	Cfg.BatchSizeBytes   = static_cast<uint32>(Settings->BatchSizeBytes);
	Cfg.BatchMaxEvents   = static_cast<uint32>(Settings->BatchMaxEvents);
	Cfg.FlushIntervalMs  = static_cast<uint32>(Settings->FlushIntervalMs);
	Cfg.EnableCompression = Settings->bEnableCompression ? 1 : 0;
	Cfg.AutoTrackSession  = Settings->bAutoTrackSession ? 1 : 0;

	// Provide a writable directory for the durable pending-event cache, and push
	// platform/device info, BEFORE init. Both must be set before gr_initialize:
	// the cache dir is read when the SDK builds its pending-event store, and the
	// device-info override is read when the SDK builds the session_start event
	// (otherwise it falls back to the desktop provider and mislabels Android as "pc").
	if (FGameRebellionNativeLibrary::SetDataDir)
	{
		// The SDK .so writes via libc, not UE's IFileManager, so it needs a real
		// absolute OS path. FPaths::ProjectSavedDir() is a staged *relative* path
		// (../../../<Project>/Saved) that resolves to a read-only location (/...) when
		// passed to a non-UE writer on Android. ConvertToAbsolutePathForExternalAppForWrite
		// maps it to the writable sandbox dir (external files dir on Android).
		const FString DataDir = IFileManager::Get().ConvertToAbsolutePathForExternalAppForWrite(*FPaths::ProjectSavedDir());
		FGameRebellionNativeLibrary::SetDataDir(GR_STR(DataDir));
		UE_LOG(LogTemp, Log, TEXT("[GameRebellion] Data dir: %s"), *DataDir);
	}
	SendDeviceInfo();

	int32 Result = FGameRebellionNativeLibrary::Initialize(&Cfg);
	if (Result == GR_OK)
	{
		bInitialized = true;
		BindLifecycleDelegates();
		StartLogDrain();
		UE_LOG(LogTemp, Log, TEXT("[GameRebellion] SDK initialized"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[GameRebellion] Init failed (%d)"), Result);
	}
	return Result;
}

int32 UGameRebellionSubsystem::ShutdownSDK(const FString& EndReason)
{
	StopLogDrain();
	DrainNativeLogsOnce(); // capture anything emitted during shutdown/final flush
	UnbindLifecycleDelegates();
	int32 Result = GR_ERR_INVALID_STATE;
	if (FGameRebellionNativeLibrary::Shutdown)
	{
		Result = FGameRebellionNativeLibrary::Shutdown(GR_STR(EndReason));
	}
	bInitialized = false;
	return Result;
}

int32 UGameRebellionSubsystem::SetConsent(bool bGranted)
{
	if (!FGameRebellionNativeLibrary::SetConsent) return GR_ERR_INVALID_STATE;
	return FGameRebellionNativeLibrary::SetConsent(bGranted ? 1 : 0);
}

int32 UGameRebellionSubsystem::SetPaused(bool bPaused)
{
	if (!FGameRebellionNativeLibrary::SetPaused) return GR_ERR_INVALID_STATE;
	return FGameRebellionNativeLibrary::SetPaused(bPaused ? 1 : 0);
}

int32 UGameRebellionSubsystem::SetNetworkOnline(bool bOnline)
{
	if (!FGameRebellionNativeLibrary::SetNetworkOnline) return GR_ERR_INVALID_STATE;
	return FGameRebellionNativeLibrary::SetNetworkOnline(bOnline ? 1 : 0);
}

int32 UGameRebellionSubsystem::Flush()
{
	if (!FGameRebellionNativeLibrary::Flush) return GR_ERR_INVALID_STATE;
	return FGameRebellionNativeLibrary::Flush();
}

int32 UGameRebellionSubsystem::GetState() const
{
	if (!FGameRebellionNativeLibrary::GetState) return GR_STATE_UNINITIALIZED;
	return FGameRebellionNativeLibrary::GetState();
}

FString UGameRebellionSubsystem::GetLastError() const
{
	if (!FGameRebellionNativeLibrary::GrGetLastError) return TEXT("Library not loaded");
	char Buf[4096] = {};
	FGameRebellionNativeLibrary::GrGetLastError(Buf, sizeof(Buf));
	return FString(UTF8_TO_TCHAR(Buf));
}

FString UGameRebellionSubsystem::DrainLogs() const
{
	if (!FGameRebellionNativeLibrary::DrainLogs) return FString();
	char Buf[16384] = {};
	FGameRebellionNativeLibrary::DrainLogs(Buf, sizeof(Buf));
	return FString(UTF8_TO_TCHAR(Buf));
}

int32 UGameRebellionSubsystem::TrackJson(const FString& EventName, const FString& JsonPayload)
{
	if (!FGameRebellionNativeLibrary::TrackJson) return GR_ERR_INVALID_STATE;
	if (!bInitialized)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[GameRebellion] TrackJson called before SDK init completed -- event will be dropped."));
	}
	return FGameRebellionNativeLibrary::TrackJson(GR_STR(EventName), GR_STR(JsonPayload));
}

int32 UGameRebellionSubsystem::TrackLog(const FString& Type, const FString& Category,
	const FString& Message, const FString& Description)
{
	if (!FGameRebellionNativeLibrary::TrackLog) return GR_ERR_INVALID_STATE;
	return FGameRebellionNativeLibrary::TrackLog(
		GR_STR(Type), GR_STR(Category), GR_STR(Message), GR_STR(Description),
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
		std::nan(""), nullptr, -1);
}

int32 UGameRebellionSubsystem::TrackProgression(const FString& Type, const FString& Status,
	const FString& Progression01, const FString& Progression02, const FString& Progression03)
{
	if (!FGameRebellionNativeLibrary::TrackProgression) return GR_ERR_INVALID_STATE;
	return FGameRebellionNativeLibrary::TrackProgression(
		GR_STR(Type), GR_STR(Status),
		GR_STR(Progression01), GR_STR(Progression02), GR_STR(Progression03),
		nullptr,
		std::nan(""), std::nan(""), std::nan(""), std::nan(""),
		std::nan(""), std::nan(""), std::nan(""), std::nan(""), std::nan(""),
		-1, std::nan(""), std::nan(""), std::nan(""), std::nan(""), std::nan(""),
		nullptr, std::nan(""), -1);
}

int32 UGameRebellionSubsystem::TrackLevelUp(double Level)
{
	if (!FGameRebellionNativeLibrary::TrackLevelUp) return GR_ERR_INVALID_STATE;
	return FGameRebellionNativeLibrary::TrackLevelUp(Level);
}

int32 UGameRebellionSubsystem::TrackAchievement(const FString& Id)
{
	if (!FGameRebellionNativeLibrary::TrackAchievement) return GR_ERR_INVALID_STATE;
	return FGameRebellionNativeLibrary::TrackAchievement(GR_STR(Id));
}

int32 UGameRebellionSubsystem::TrackTransaction(double Amount, const FString& Currency,
	const FString& Type, const FString& Description)
{
	if (!FGameRebellionNativeLibrary::TrackTransaction) return GR_ERR_INVALID_STATE;
	return FGameRebellionNativeLibrary::TrackTransaction(
		Amount, GR_STR(Currency), GR_STR(Type), GR_STR(Description),
		nullptr, std::nan(""), nullptr, nullptr, nullptr);
}

int32 UGameRebellionSubsystem::RecordFrame(double Fps)
{
	if (!FGameRebellionNativeLibrary::RecordFrame) return GR_ERR_INVALID_STATE;
	return FGameRebellionNativeLibrary::RecordFrame(Fps);
}

int32 UGameRebellionSubsystem::RecordMemory(double MemoryMB)
{
	if (!FGameRebellionNativeLibrary::RecordMemory) return GR_ERR_INVALID_STATE;
	return FGameRebellionNativeLibrary::RecordMemory(MemoryMB);
}

// ── Native SDK log draining (diagnostics) ──

void UGameRebellionSubsystem::StartLogDrain()
{
	if (LogDrainTickerHandle.IsValid())
	{
		return;
	}
	// Drain every 3s on the game thread. The native call is a cheap buffer copy.
	LogDrainTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &UGameRebellionSubsystem::TickLogDrain), 3.0f);
}

void UGameRebellionSubsystem::StopLogDrain()
{
	if (LogDrainTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(LogDrainTickerHandle);
		LogDrainTickerHandle.Reset();
	}
}

bool UGameRebellionSubsystem::TickLogDrain(float /*DeltaTime*/)
{
	DrainNativeLogsOnce();
	return true; // keep ticking
}

void UGameRebellionSubsystem::DrainNativeLogsOnce()
{
	if (!FGameRebellionNativeLibrary::DrainLogs)
	{
		return;
	}
	const FString Logs = DrainLogs();
	if (Logs.IsEmpty())
	{
		return;
	}
	TArray<FString> Lines;
	Logs.ParseIntoArrayLines(Lines);
	for (const FString& Line : Lines)
	{
		if (!Line.IsEmpty())
		{
			UE_LOG(LogTemp, Log, TEXT("[GameRebellion][sdk] %s"), *Line);
		}
	}
}

// ── Lifecycle delegates ──

void UGameRebellionSubsystem::BindLifecycleDelegates()
{
	DeactivateHandle = FCoreDelegates::ApplicationWillDeactivateDelegate
		.AddUObject(this, &UGameRebellionSubsystem::OnAppWillDeactivate);
	ReactivateHandle = FCoreDelegates::ApplicationHasReactivatedDelegate
		.AddUObject(this, &UGameRebellionSubsystem::OnAppHasReactivated);
	TerminateHandle = FCoreDelegates::GetApplicationWillTerminateDelegate()
		.AddUObject(this, &UGameRebellionSubsystem::OnAppWillTerminate);
}

void UGameRebellionSubsystem::UnbindLifecycleDelegates()
{
	FCoreDelegates::ApplicationWillDeactivateDelegate.Remove(DeactivateHandle);
	FCoreDelegates::ApplicationHasReactivatedDelegate.Remove(ReactivateHandle);
	FCoreDelegates::GetApplicationWillTerminateDelegate().Remove(TerminateHandle);
}

void UGameRebellionSubsystem::OnAppWillDeactivate()
{
	if (bInitialized && FGameRebellionNativeLibrary::SetPaused)
	{
		FGameRebellionNativeLibrary::SetPaused(1);
	}
}

void UGameRebellionSubsystem::OnAppHasReactivated()
{
	if (bInitialized && FGameRebellionNativeLibrary::SetPaused)
	{
		FGameRebellionNativeLibrary::SetPaused(0);
	}
}

void UGameRebellionSubsystem::OnAppWillTerminate()
{
	if (bInitialized)
	{
		ShutdownSDK(TEXT("app_terminate"));
	}
}

// ── Device info ──

void UGameRebellionSubsystem::SendDeviceInfo()
{
	if (!FGameRebellionNativeLibrary::SetDeviceInfo) return;

	// Map UE platform names to the SDK's canonical vocabulary ("android"/"ios"/"pc").
	// The NativeAOT build runs as linux-bionic and would otherwise self-detect as a
	// Linux PC, mislabeling mobile events.
	FString Platform;
	{
		const FString UePlatform = FPlatformProperties::PlatformName();
		if (UePlatform.Equals(TEXT("Android"), ESearchCase::IgnoreCase))      Platform = TEXT("android");
		else if (UePlatform.Equals(TEXT("IOS"), ESearchCase::IgnoreCase))     Platform = TEXT("ios");
		else                                                                  Platform = TEXT("pc");
	}
	FString DeviceModel  = FPlatformMisc::GetDeviceMakeAndModel();
	FString OsVersion    = FPlatformMisc::GetOSVersion();
	FString GameVersion  = GetConfiguredProjectVersion();
	FString BuildNumber  = GetConfiguredBuildNumber();
	FString DeviceId     = FPlatformMisc::GetDeviceId();
	FString ComputerName = FPlatformProcess::ComputerName();

	FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
	double MemoryMB = static_cast<double>(MemStats.UsedPhysical) / (1024.0 * 1024.0);

	FTCHARToUTF8 PlatformU(*Platform);
	FTCHARToUTF8 ModelU(*DeviceModel);
	FTCHARToUTF8 OsU(*OsVersion);
	FTCHARToUTF8 GameVerU(*GameVersion);
	FTCHARToUTF8 BuildU(*BuildNumber);
	FTCHARToUTF8 DevIdU(*DeviceId);
	FTCHARToUTF8 CompU(*ComputerName);

	GrDeviceInfo Info = {};
	Info.Platform        = PlatformU.Get();
	Info.DeviceModel     = ModelU.Get();
	Info.OsVersion       = OsU.Get();
	Info.GameVersion     = GameVerU.Get();
	Info.BuildNumber     = BuildU.Get();
	Info.AppInstallTime  = nullptr;
	Info.AppUpdateTime   = nullptr;
	Info.BatteryLevel    = std::nan("");
	Info.FreeStorageMB   = std::nan("");
	Info.MemoryUsageMB   = MemoryMB;
	Info.DeviceId        = DevIdU.Get();
	Info.Timezone        = nullptr;
	Info.ScreenResolution = nullptr;
	Info.ConnectionType  = nullptr;
	Info.LocalIp         = nullptr;
	Info.ComputerName    = CompU.Get();

	FGameRebellionNativeLibrary::SetDeviceInfo(&Info);
}
