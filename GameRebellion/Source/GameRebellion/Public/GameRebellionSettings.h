#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GameRebellionSettings.generated.h"

UENUM(BlueprintType)
enum class EGrEnvironment : uint8
{
	// Values MUST match the native C ABI contract (core GrEnvironment / Unity SDK):
	// the subsystem sends the raw numeric value to gr_initialize, and the core maps
	// 0->Production, 1->Staging, 2->Development. Do NOT reorder without changing the
	// core's GrConfigModel.FromCStruct in lockstep. (Config is serialized by name,
	// so existing DefaultGame.ini "Environment=..." values are unaffected.)
	Production  = 0,
	Staging     = 1,
	Development = 2
};

UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="GameRebellion"))
class GAMEREBELLION_API UGameRebellionSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UGameRebellionSettings();

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="GameRebellion")
	FString ApiKey;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="GameRebellion")
	EGrEnvironment Environment = EGrEnvironment::Production;

	/** Build identifier reported to analytics (e.g. CI build number). When empty,
	 *  falls back to the project-defined BUILD_VERSION, then to ProjectVersion. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="GameRebellion")
	FString BuildNumber;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="GameRebellion|Batching",
		meta=(ClampMin="1024", UIMin="1024"))
	int32 BatchSizeBytes = 65536;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="GameRebellion|Batching",
		meta=(ClampMin="1", UIMin="1"))
	int32 BatchMaxEvents = 100;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="GameRebellion|Batching",
		meta=(ClampMin="1000", UIMin="1000"))
	int32 FlushIntervalMs = 30000;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="GameRebellion")
	bool bEnableCompression = false;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="GameRebellion")
	bool bAutoTrackSession = true;

	virtual FName GetCategoryName() const override { return FName(TEXT("Plugins")); }
};
