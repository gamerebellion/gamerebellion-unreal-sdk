#pragma once

#include "CoreMinimal.h"
#include "Containers/Ticker.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameRebellionSettings.h"
#include "GameRebellionSubsystem.generated.h"

UCLASS()
class GAMEREBELLION_API UGameRebellionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category="GameRebellion")
	int32 InitSDK();

	UFUNCTION(BlueprintCallable, Category="GameRebellion")
	int32 ShutdownSDK(const FString& EndReason = TEXT("normal"));

	UFUNCTION(BlueprintCallable, Category="GameRebellion")
	int32 SetConsent(bool bGranted);

	UFUNCTION(BlueprintCallable, Category="GameRebellion")
	int32 SetPaused(bool bPaused);

	UFUNCTION(BlueprintCallable, Category="GameRebellion")
	int32 SetNetworkOnline(bool bOnline);

	UFUNCTION(BlueprintCallable, Category="GameRebellion")
	int32 Flush();

	UFUNCTION(BlueprintCallable, Category="GameRebellion")
	int32 GetState() const;

	UFUNCTION(BlueprintCallable, Category="GameRebellion")
	FString GetLastError() const;

	UFUNCTION(BlueprintCallable, Category="GameRebellion")
	FString DrainLogs() const;

	UFUNCTION(BlueprintCallable, Category="GameRebellion")
	int32 TrackJson(const FString& EventName, const FString& JsonPayload);

	UFUNCTION(BlueprintCallable, Category="GameRebellion|Events")
	int32 TrackLog(const FString& Type, const FString& Category = TEXT(""),
		const FString& Message = TEXT(""), const FString& Description = TEXT(""));

	UFUNCTION(BlueprintCallable, Category="GameRebellion|Events")
	int32 TrackProgression(const FString& Type, const FString& Status,
		const FString& Progression01 = TEXT(""), const FString& Progression02 = TEXT(""),
		const FString& Progression03 = TEXT(""));

	UFUNCTION(BlueprintCallable, Category="GameRebellion|Events")
	int32 TrackLevelUp(double Level);

	UFUNCTION(BlueprintCallable, Category="GameRebellion|Events")
	int32 TrackAchievement(const FString& Id);

	UFUNCTION(BlueprintCallable, Category="GameRebellion|Events")
	int32 TrackTransaction(double Amount, const FString& Currency,
		const FString& Type = TEXT(""), const FString& Description = TEXT(""));

	UFUNCTION(BlueprintCallable, Category="GameRebellion|Metrics")
	int32 RecordFrame(double Fps);

	UFUNCTION(BlueprintCallable, Category="GameRebellion|Metrics")
	int32 RecordMemory(double MemoryMB);

private:
	void BindLifecycleDelegates();
	void UnbindLifecycleDelegates();
	void SendDeviceInfo();

	// Periodically drains the native SDK's internal log buffer to UE_LOG so the
	// transport/auth/pipeline diagnostics (e.g. "[Transport] Batch sent successfully"
	// or "[Transport] Validation error (422)") are visible in logcat/the UE log.
	void StartLogDrain();
	void StopLogDrain();
	bool TickLogDrain(float DeltaTime);
	void DrainNativeLogsOnce();

	FTSTicker::FDelegateHandle LogDrainTickerHandle;

	void OnAppWillDeactivate();
	void OnAppHasReactivated();
	void OnAppWillTerminate();

	bool bInitialized = false;

	FDelegateHandle DeactivateHandle;
	FDelegateHandle ReactivateHandle;
	FDelegateHandle TerminateHandle;
};
