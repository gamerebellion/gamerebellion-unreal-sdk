// GameRebellionNative.h
// C ABI declarations for the GameRebellion NativeAOT shared library.
// Mirror of core/GameRebellionSdk.cs [UnmanagedCallersOnly] exports.

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Error codes
#define GR_OK                0
#define GR_ERR_GENERIC      -1
#define GR_ERR_INVALID_ARGS -2
#define GR_ERR_INVALID_STATE -3

// Environment enum
#define GR_ENV_DEVELOPMENT  0
#define GR_ENV_STAGING      1
#define GR_ENV_PRODUCTION   2

// SDK state enum
#define GR_STATE_UNINITIALIZED 0
#define GR_STATE_INITIALIZING  1
#define GR_STATE_READY         2
#define GR_STATE_PAUSED        3
#define GR_STATE_FLUSHING      4
#define GR_STATE_SHUTTING_DOWN 5
#define GR_STATE_ERROR         6

#pragma pack(push, 8)

typedef struct {
    const char* ApiKey;
    const char* GameVersion;
    const char* BuildNumber;
    int32_t     Environment;
    uint32_t    BatchSizeBytes;
    uint32_t    BatchMaxEvents;
    uint32_t    FlushIntervalMs;
    int32_t     EnableCompression;
    int32_t     AutoTrackSession;
} GrConfig;

typedef struct {
    const char* Platform;
    const char* DeviceModel;
    const char* OsVersion;
    const char* GameVersion;
    const char* BuildNumber;
    const char* AppInstallTime;
    const char* AppUpdateTime;
    double      BatteryLevel;       // NaN = unknown
    double      FreeStorageMB;      // NaN = unknown
    double      MemoryUsageMB;      // NaN = unknown
    const char* DeviceId;
    const char* Timezone;
    const char* ScreenResolution;
    const char* ConnectionType;
    const char* LocalIp;
    const char* ComputerName;
} GrDeviceInfo;

#pragma pack(pop)

// ── Lifecycle ──
int32_t gr_initialize(const GrConfig* config);
// Absolute writable directory for the SDK's durable event cache. Must be
// called BEFORE gr_initialize on sandboxed platforms (Android, iOS).
int32_t gr_set_data_dir(const char* dir_utf8);
int32_t gr_set_device_info(const GrDeviceInfo* info);
int32_t gr_shutdown(const char* end_reason);
int32_t gr_get_state(void);
int32_t gr_get_last_error(char* out_buffer, uint32_t capacity);
int32_t gr_set_consent(int32_t state);
int32_t gr_set_paused(int32_t paused);
int32_t gr_set_network_online(int32_t online);
int32_t gr_flush(void);
int32_t gr_drain_logs(char* out_buffer, uint32_t capacity);

// ── Generic event ──
int32_t gr_track_json(const char* event_name, const char* json_utf8);

// ── Typed events ──
int32_t gr_track_log(
    const char* type, const char* category, const char* message,
    const char* description, const char* error_code, const char* error_domain,
    const char* feature_name, const char* stack_trace, const char* exception_type,
    const char* method_name, double line_number, const char* thread_id,
    int32_t is_user_affected);

int32_t gr_track_feature_use(
    const char* type, const char* name, const char* category,
    double time_spent, const char* completion_status);

int32_t gr_track_progression(
    const char* type, const char* status,
    const char* progression01, const char* progression02, const char* progression03,
    const char* difficulty, double attempt_number, double score,
    double completion_time, double completion_percentage,
    double objectives_completed, double objectives_total,
    double lives_used, double lives_remaining, double health_remaining,
    int32_t new_record, double enemies_defeated, double items_collected,
    double distance_traveled, double deaths_count, double currency_earned,
    const char* currency_type, double experience_gained, int32_t level_up_triggered);

int32_t gr_track_level_up(double level);
int32_t gr_track_achievement(const char* id);
int32_t gr_track_level_completed(int32_t level_id, int32_t score,
    const char* difficulty, const char* email);

int32_t gr_track_login(
    const char* external_user_id, const char* login_method, const char* login_provider,
    int32_t is_first_login, int32_t is_returning_user, double days_since_last_login,
    double login_attempt_count, int32_t pii_consent, int32_t analytics_consent,
    const char* account_created_time, const char* account_level, const char* account_status,
    const char* username, const char* display_name, const char* first_name,
    const char* last_name, const char* nickname, const char* email, const char* phone,
    const char* gender, const char* age_range, double birth_year,
    const char* country, const char* region, const char* city, const char* language);

int32_t gr_track_logout(const char* end_reason, double login_duration);
int32_t gr_track_friend_invite(const char* invite_method, double invite_count);

int32_t gr_track_group_join(
    const char* group_id, const char* group_name, double group_size,
    const char* group_type, const char* join_method);

int32_t gr_track_chat_message(
    const char* chat_type, double message_length,
    const char* message_type, const char* language_detected);

int32_t gr_track_voice_call_start(
    const char* type, const char* call_id, double participant_count);

int32_t gr_track_voice_call_stop(const char* call_id, const char* stop_reason);

int32_t gr_track_transaction(
    double amount, const char* currency, const char* type, const char* description,
    const char* transaction_hash, double usd_value, const char* platform_fee,
    const char* status, const char* failure_reason);

int32_t gr_track_crypto_transaction(
    const char* amount, const char* currency, const char* type, const char* description,
    const char* transaction_hash, const char* block_number, const char* blockchain,
    const char* nft_contract_address, const char* nft_token_id,
    const char* nft_collection, const char* nft_rarity, const char* nft_metadata_uri,
    const char* gas_fee, const char* gas_limit, const char* gas_price,
    const char* marketplace, const char* wallet_address,
    double usd_value, double exchange_rate, const char* platform_fee,
    const char* royalty_fee, const char* status, double confirmation_count,
    const char* failure_reason, const char* game_item_id,
    const char* player_wallet_type, const char* integration_method);

int32_t gr_track_ad_view(
    const char* category, const char* ad_sdk_name, const char* ad_placement,
    const char* ad_type, double ad_duration, int32_t ad_first,
    const char* network_id, const char* campaign_id, const char* ad_id,
    const char* network_event_id, double creative_width, double creative_height,
    double video_duration, double load_time);

int32_t gr_track_ad_click(
    const char* category, const char* ad_sdk_name, const char* ad_placement,
    const char* ad_type, double click_coordinate_x, double click_coordinate_y,
    double time_to_click, const char* network_id, const char* campaign_id,
    const char* ad_id, const char* network_event_id);

int32_t gr_track_ad_error(
    const char* category, const char* ad_sdk_name, const char* ad_placement,
    const char* ad_type, const char* ad_fail_show_reason, const char* error_code,
    const char* error_message, double retry_count, const char* network_id,
    const char* campaign_id, const char* ad_id, const char* network_event_id,
    double attempted_load_time);

int32_t gr_track_ad_reward(
    const char* category, const char* ad_sdk_name, const char* ad_placement,
    const char* ad_type, const char* reward_type, double reward_amount,
    const char* reward_currency, int32_t completion_required,
    const char* network_id, const char* campaign_id, const char* ad_id,
    const char* network_event_id);

// ── Metrics ──
int32_t gr_record_frame(double fps);
int32_t gr_record_memory(double memory_mb);

#ifdef __cplusplus
}
#endif
