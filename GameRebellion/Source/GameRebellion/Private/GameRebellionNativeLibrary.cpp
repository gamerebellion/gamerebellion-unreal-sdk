#include "GameRebellionNativeLibrary.h"

#include "HAL/PlatformProcess.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"

// Avoid collision with Windows GetLastError macro
#ifdef GetLastError
#undef GetLastError
#endif

void* FGameRebellionNativeLibrary::Handle = nullptr;
FString FGameRebellionNativeLibrary::LastError;

gr_initialize_fn FGameRebellionNativeLibrary::Initialize = nullptr;
gr_set_data_dir_fn FGameRebellionNativeLibrary::SetDataDir = nullptr;
gr_set_device_info_fn FGameRebellionNativeLibrary::SetDeviceInfo = nullptr;
gr_set_consent_fn FGameRebellionNativeLibrary::SetConsent = nullptr;
gr_track_level_completed_fn FGameRebellionNativeLibrary::TrackLevelCompleted = nullptr;
gr_shutdown_fn FGameRebellionNativeLibrary::Shutdown = nullptr;
gr_get_state_fn FGameRebellionNativeLibrary::GetState = nullptr;
gr_get_last_error_fn FGameRebellionNativeLibrary::GrGetLastError = nullptr;
gr_set_paused_fn FGameRebellionNativeLibrary::SetPaused = nullptr;
gr_set_network_online_fn FGameRebellionNativeLibrary::SetNetworkOnline = nullptr;
gr_flush_fn FGameRebellionNativeLibrary::Flush = nullptr;
gr_track_json_fn FGameRebellionNativeLibrary::TrackJson = nullptr;
gr_track_log_fn FGameRebellionNativeLibrary::TrackLog = nullptr;
gr_track_feature_use_fn FGameRebellionNativeLibrary::TrackFeatureUse = nullptr;
gr_track_progression_fn FGameRebellionNativeLibrary::TrackProgression = nullptr;
gr_track_level_up_fn FGameRebellionNativeLibrary::TrackLevelUp = nullptr;
gr_track_achievement_fn FGameRebellionNativeLibrary::TrackAchievement = nullptr;
gr_track_login_fn FGameRebellionNativeLibrary::TrackLogin = nullptr;
gr_track_logout_fn FGameRebellionNativeLibrary::TrackLogout = nullptr;
gr_track_friend_invite_fn FGameRebellionNativeLibrary::TrackFriendInvite = nullptr;
gr_track_group_join_fn FGameRebellionNativeLibrary::TrackGroupJoin = nullptr;
gr_track_chat_message_fn FGameRebellionNativeLibrary::TrackChatMessage = nullptr;
gr_track_voice_call_start_fn FGameRebellionNativeLibrary::TrackVoiceCallStart = nullptr;
gr_track_voice_call_stop_fn FGameRebellionNativeLibrary::TrackVoiceCallStop = nullptr;
gr_track_transaction_fn FGameRebellionNativeLibrary::TrackTransaction = nullptr;
gr_track_crypto_transaction_fn FGameRebellionNativeLibrary::TrackCryptoTransaction = nullptr;
gr_track_ad_view_fn FGameRebellionNativeLibrary::TrackAdView = nullptr;
gr_track_ad_click_fn FGameRebellionNativeLibrary::TrackAdClick = nullptr;
gr_track_ad_error_fn FGameRebellionNativeLibrary::TrackAdError = nullptr;
gr_track_ad_reward_fn FGameRebellionNativeLibrary::TrackAdReward = nullptr;
gr_drain_logs_fn FGameRebellionNativeLibrary::DrainLogs = nullptr;
gr_record_frame_fn FGameRebellionNativeLibrary::RecordFrame = nullptr;
gr_record_memory_fn FGameRebellionNativeLibrary::RecordMemory = nullptr;

bool FGameRebellionNativeLibrary::Load()
{
    if (Handle)
    {
        return true;
    }

    ResetFunctions();
    LastError.Empty();

#if PLATFORM_IOS
    // iOS links libGameRebellion.Core.a statically (Apple forbids dlopen of
    // non-embedded code; NativeAOT-iOS emits a static archive), so the gr_*
    // symbols are resolved by the app linker and bound directly here -- no
    // GetDllHandle/GetDllExport. A missing symbol is a link error at package
    // time, never a runtime failure. Handle is a sentinel so IsLoaded() works.
    Initialize = &gr_initialize;
    SetDataDir = &gr_set_data_dir;
    SetDeviceInfo = &gr_set_device_info;
    SetConsent = &gr_set_consent;
    TrackLevelCompleted = &gr_track_level_completed;
    Shutdown = &gr_shutdown;
    GetState = &gr_get_state;
    GrGetLastError = &gr_get_last_error;
    SetPaused = &gr_set_paused;
    SetNetworkOnline = &gr_set_network_online;
    Flush = &gr_flush;
    TrackJson = &gr_track_json;
    TrackLog = &gr_track_log;
    TrackFeatureUse = &gr_track_feature_use;
    TrackProgression = &gr_track_progression;
    TrackLevelUp = &gr_track_level_up;
    TrackAchievement = &gr_track_achievement;
    TrackLogin = &gr_track_login;
    TrackLogout = &gr_track_logout;
    TrackFriendInvite = &gr_track_friend_invite;
    TrackGroupJoin = &gr_track_group_join;
    TrackChatMessage = &gr_track_chat_message;
    TrackVoiceCallStart = &gr_track_voice_call_start;
    TrackVoiceCallStop = &gr_track_voice_call_stop;
    TrackTransaction = &gr_track_transaction;
    TrackCryptoTransaction = &gr_track_crypto_transaction;
    TrackAdView = &gr_track_ad_view;
    TrackAdClick = &gr_track_ad_click;
    TrackAdError = &gr_track_ad_error;
    TrackAdReward = &gr_track_ad_reward;
    DrainLogs = &gr_drain_logs;
    RecordFrame = &gr_record_frame;
    RecordMemory = &gr_record_memory;

    Handle = reinterpret_cast<void*>(1);
    return true;
#else
    const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("GameRebellion"));
    if (!Plugin.IsValid())
    {
        LastError = TEXT("GameRebellion plugin not found");
        return false;
    }

    FString LibraryPath;
#if PLATFORM_WINDOWS
    LibraryPath = FPaths::Combine(Plugin->GetBaseDir(), TEXT("ThirdParty/GameRebellionCore/Win64/GameRebellion.Core.dll"));
#elif PLATFORM_LINUX
    LibraryPath = FPaths::Combine(Plugin->GetBaseDir(), TEXT("ThirdParty/GameRebellionCore/Linux/GameRebellion.Core.so"));
#elif PLATFORM_MAC
    LibraryPath = FPaths::Combine(Plugin->GetBaseDir(), TEXT("ThirdParty/GameRebellionCore/Mac/GameRebellion.Core.dylib"));
#elif PLATFORM_ANDROID
    LibraryPath = TEXT("GameRebellion.Core.so");
#else
    LastError = TEXT("Platform not supported for native SDK loading");
    return false;
#endif

    FPlatformProcess::PushDllDirectory(*FPaths::GetPath(LibraryPath));
    Handle = FPlatformProcess::GetDllHandle(*LibraryPath);
    FPlatformProcess::PopDllDirectory(*FPaths::GetPath(LibraryPath));
#if PLATFORM_ANDROID
    if (!Handle)
    {
        const FString FallbackPath = FPaths::Combine(
            Plugin->GetBaseDir(),
            TEXT("ThirdParty/GameRebellionCore/Android/arm64-v8a/GameRebellion.Core.so"));
        Handle = FPlatformProcess::GetDllHandle(*FallbackPath);
        if (Handle)
        {
            LibraryPath = FallbackPath;
        }
    }
#endif
    if (!Handle)
    {
        LastError = FString::Printf(TEXT("Failed to load native SDK library: %s"), *LibraryPath);
        return false;
    }

    Initialize = reinterpret_cast<gr_initialize_fn>(LoadExport("gr_initialize"));
    SetDataDir = reinterpret_cast<gr_set_data_dir_fn>(LoadExport("gr_set_data_dir"));
    SetDeviceInfo = reinterpret_cast<gr_set_device_info_fn>(LoadExport("gr_set_device_info"));
    SetConsent = reinterpret_cast<gr_set_consent_fn>(LoadExport("gr_set_consent"));
    TrackLevelCompleted = reinterpret_cast<gr_track_level_completed_fn>(LoadExport("gr_track_level_completed"));
    Shutdown = reinterpret_cast<gr_shutdown_fn>(LoadExport("gr_shutdown"));
    GetState = reinterpret_cast<gr_get_state_fn>(LoadExport("gr_get_state"));
    GrGetLastError = reinterpret_cast<gr_get_last_error_fn>(LoadExport("gr_get_last_error"));
    SetPaused = reinterpret_cast<gr_set_paused_fn>(LoadExport("gr_set_paused"));
    SetNetworkOnline = reinterpret_cast<gr_set_network_online_fn>(LoadExport("gr_set_network_online"));
    Flush = reinterpret_cast<gr_flush_fn>(LoadExport("gr_flush"));
    TrackJson = reinterpret_cast<gr_track_json_fn>(LoadExport("gr_track_json"));
    TrackLog = reinterpret_cast<gr_track_log_fn>(LoadExport("gr_track_log"));
    TrackFeatureUse = reinterpret_cast<gr_track_feature_use_fn>(LoadExport("gr_track_feature_use"));
    TrackProgression = reinterpret_cast<gr_track_progression_fn>(LoadExport("gr_track_progression"));
    TrackLevelUp = reinterpret_cast<gr_track_level_up_fn>(LoadExport("gr_track_level_up"));
    TrackAchievement = reinterpret_cast<gr_track_achievement_fn>(LoadExport("gr_track_achievement"));
    TrackLogin = reinterpret_cast<gr_track_login_fn>(LoadExport("gr_track_login"));
    TrackLogout = reinterpret_cast<gr_track_logout_fn>(LoadExport("gr_track_logout"));
    TrackFriendInvite = reinterpret_cast<gr_track_friend_invite_fn>(LoadExport("gr_track_friend_invite"));
    TrackGroupJoin = reinterpret_cast<gr_track_group_join_fn>(LoadExport("gr_track_group_join"));
    TrackChatMessage = reinterpret_cast<gr_track_chat_message_fn>(LoadExport("gr_track_chat_message"));
    TrackVoiceCallStart = reinterpret_cast<gr_track_voice_call_start_fn>(LoadExport("gr_track_voice_call_start"));
    TrackVoiceCallStop = reinterpret_cast<gr_track_voice_call_stop_fn>(LoadExport("gr_track_voice_call_stop"));
    TrackTransaction = reinterpret_cast<gr_track_transaction_fn>(LoadExport("gr_track_transaction"));
    TrackCryptoTransaction = reinterpret_cast<gr_track_crypto_transaction_fn>(LoadExport("gr_track_crypto_transaction"));
    TrackAdView = reinterpret_cast<gr_track_ad_view_fn>(LoadExport("gr_track_ad_view"));
    TrackAdClick = reinterpret_cast<gr_track_ad_click_fn>(LoadExport("gr_track_ad_click"));
    TrackAdError = reinterpret_cast<gr_track_ad_error_fn>(LoadExport("gr_track_ad_error"));
    TrackAdReward = reinterpret_cast<gr_track_ad_reward_fn>(LoadExport("gr_track_ad_reward"));
    DrainLogs = reinterpret_cast<gr_drain_logs_fn>(LoadExport("gr_drain_logs"));
    RecordFrame = reinterpret_cast<gr_record_frame_fn>(LoadExport("gr_record_frame"));
    RecordMemory = reinterpret_cast<gr_record_memory_fn>(LoadExport("gr_record_memory"));

    if (!Initialize || !Shutdown || !GetState)
    {
        LastError = TEXT("Missing required exports in native SDK library");
        Unload();
        return false;
    }

    return true;
#endif // PLATFORM_IOS
}

void FGameRebellionNativeLibrary::Unload()
{
#if PLATFORM_IOS
    // Statically linked: nothing to free, Handle is a sentinel.
    Handle = nullptr;
#else
    if (Handle)
    {
        FPlatformProcess::FreeDllHandle(Handle);
        Handle = nullptr;
    }
#endif
    ResetFunctions();
}

bool FGameRebellionNativeLibrary::IsLoaded()
{
    return Handle != nullptr;
}

const FString& FGameRebellionNativeLibrary::GetLoadError()
{
    return LastError;
}

void FGameRebellionNativeLibrary::ResetFunctions()
{
    Initialize = nullptr;
    SetDataDir = nullptr;
    SetDeviceInfo = nullptr;
    SetConsent = nullptr;
    TrackLevelCompleted = nullptr;
    Shutdown = nullptr;
    GetState = nullptr;
    GrGetLastError = nullptr;
    SetPaused = nullptr;
    SetNetworkOnline = nullptr;
    Flush = nullptr;
    TrackJson = nullptr;
    TrackLog = nullptr;
    TrackFeatureUse = nullptr;
    TrackProgression = nullptr;
    TrackLevelUp = nullptr;
    TrackAchievement = nullptr;
    TrackLogin = nullptr;
    TrackLogout = nullptr;
    TrackFriendInvite = nullptr;
    TrackGroupJoin = nullptr;
    TrackChatMessage = nullptr;
    TrackVoiceCallStart = nullptr;
    TrackVoiceCallStop = nullptr;
    TrackTransaction = nullptr;
    TrackCryptoTransaction = nullptr;
    TrackAdView = nullptr;
    TrackAdClick = nullptr;
    TrackAdError = nullptr;
    TrackAdReward = nullptr;
    DrainLogs = nullptr;
    RecordFrame = nullptr;
    RecordMemory = nullptr;
}

void* FGameRebellionNativeLibrary::LoadExport(const ANSICHAR* Name)
{
    if (!Handle)
    {
        return nullptr;
    }
    return FPlatformProcess::GetDllExport(Handle, ANSI_TO_TCHAR(Name));
}
