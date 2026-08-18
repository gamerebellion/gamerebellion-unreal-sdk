#pragma once

#include "CoreMinimal.h"
#include "GameRebellionNative.h"

// Function pointer types for each gr_* export
typedef int32_t (*gr_initialize_fn)(const GrConfig*);
typedef int32_t (*gr_set_data_dir_fn)(const char*);
typedef int32_t (*gr_set_device_info_fn)(const GrDeviceInfo*);
typedef int32_t (*gr_set_consent_fn)(int32_t);
typedef int32_t (*gr_track_level_completed_fn)(int32_t, int32_t, const char*, const char*);
typedef int32_t (*gr_shutdown_fn)(const char*);
typedef int32_t (*gr_get_state_fn)();
typedef int32_t (*gr_get_last_error_fn)(char*, uint32_t);
typedef int32_t (*gr_set_paused_fn)(int32_t);
typedef int32_t (*gr_set_network_online_fn)(int32_t);
typedef int32_t (*gr_flush_fn)();
typedef int32_t (*gr_track_json_fn)(const char*, const char*);
typedef int32_t (*gr_track_log_fn)(const char*, const char*, const char*, const char*,
	const char*, const char*, const char*, const char*, const char*, const char*,
	double, const char*, int32_t);
typedef int32_t (*gr_track_feature_use_fn)(const char*, const char*, const char*, double, const char*);
typedef int32_t (*gr_track_progression_fn)(const char*, const char*, const char*, const char*, const char*,
	const char*, double, double, double, double, double, double, double, double, double,
	int32_t, double, double, double, double, double, const char*, double, int32_t);
typedef int32_t (*gr_track_level_up_fn)(double);
typedef int32_t (*gr_track_achievement_fn)(const char*);
typedef int32_t (*gr_track_login_fn)(const char*, const char*, const char*,
	int32_t, int32_t, double, double, int32_t, int32_t,
	const char*, const char*, const char*, const char*, const char*, const char*,
	const char*, const char*, const char*, const char*, const char*, const char*, double,
	const char*, const char*, const char*, const char*);
typedef int32_t (*gr_track_logout_fn)(const char*, double);
typedef int32_t (*gr_track_friend_invite_fn)(const char*, double);
typedef int32_t (*gr_track_group_join_fn)(const char*, const char*, double, const char*, const char*);
typedef int32_t (*gr_track_chat_message_fn)(const char*, double, const char*, const char*);
typedef int32_t (*gr_track_voice_call_start_fn)(const char*, const char*, double);
typedef int32_t (*gr_track_voice_call_stop_fn)(const char*, const char*);
typedef int32_t (*gr_track_transaction_fn)(double, const char*, const char*, const char*,
	const char*, double, const char*, const char*, const char*);
typedef int32_t (*gr_track_crypto_transaction_fn)(const char*, const char*, const char*, const char*,
	const char*, const char*, const char*, const char*, const char*, const char*, const char*,
	const char*, const char*, const char*, const char*, const char*, const char*,
	double, double, const char*, const char*, const char*, double, const char*, const char*,
	const char*, const char*);
typedef int32_t (*gr_track_ad_view_fn)(const char*, const char*, const char*, const char*,
	double, int32_t, const char*, const char*, const char*, const char*,
	double, double, double, double);
typedef int32_t (*gr_track_ad_click_fn)(const char*, const char*, const char*, const char*,
	double, double, double, const char*, const char*, const char*, const char*);
typedef int32_t (*gr_track_ad_error_fn)(const char*, const char*, const char*, const char*,
	const char*, const char*, const char*, double, const char*, const char*,
	const char*, const char*, double);
typedef int32_t (*gr_track_ad_reward_fn)(const char*, const char*, const char*, const char*,
	const char*, double, const char*, int32_t, const char*, const char*,
	const char*, const char*);
typedef int32_t (*gr_drain_logs_fn)(char*, uint32_t);
typedef int32_t (*gr_record_frame_fn)(double);
typedef int32_t (*gr_record_memory_fn)(double);

class GAMEREBELLION_API FGameRebellionNativeLibrary
{
public:
	static bool Load();
	static void Unload();
	static bool IsLoaded();
	static const FString& GetLoadError();

	// Function pointers -- valid after Load() returns true
	static gr_initialize_fn Initialize;
	static gr_set_data_dir_fn SetDataDir;
	static gr_set_device_info_fn SetDeviceInfo;
	static gr_set_consent_fn SetConsent;
	static gr_track_level_completed_fn TrackLevelCompleted;
	static gr_shutdown_fn Shutdown;
	static gr_get_state_fn GetState;
	static gr_get_last_error_fn GrGetLastError;
	static gr_set_paused_fn SetPaused;
	static gr_set_network_online_fn SetNetworkOnline;
	static gr_flush_fn Flush;
	static gr_track_json_fn TrackJson;
	static gr_track_log_fn TrackLog;
	static gr_track_feature_use_fn TrackFeatureUse;
	static gr_track_progression_fn TrackProgression;
	static gr_track_level_up_fn TrackLevelUp;
	static gr_track_achievement_fn TrackAchievement;
	static gr_track_login_fn TrackLogin;
	static gr_track_logout_fn TrackLogout;
	static gr_track_friend_invite_fn TrackFriendInvite;
	static gr_track_group_join_fn TrackGroupJoin;
	static gr_track_chat_message_fn TrackChatMessage;
	static gr_track_voice_call_start_fn TrackVoiceCallStart;
	static gr_track_voice_call_stop_fn TrackVoiceCallStop;
	static gr_track_transaction_fn TrackTransaction;
	static gr_track_crypto_transaction_fn TrackCryptoTransaction;
	static gr_track_ad_view_fn TrackAdView;
	static gr_track_ad_click_fn TrackAdClick;
	static gr_track_ad_error_fn TrackAdError;
	static gr_track_ad_reward_fn TrackAdReward;
	static gr_drain_logs_fn DrainLogs;
	static gr_record_frame_fn RecordFrame;
	static gr_record_memory_fn RecordMemory;

private:
	static void* Handle;
	static FString LastError;
	static void ResetFunctions();
	static void* LoadExport(const ANSICHAR* Name);
};
