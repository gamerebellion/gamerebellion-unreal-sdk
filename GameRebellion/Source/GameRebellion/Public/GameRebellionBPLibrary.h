#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameRebellionBPLibrary.generated.h"

UCLASS()
class GAMEREBELLION_API UGameRebellionBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="GameRebellion",
		meta=(WorldContext="WorldContextObject"))
	static int32 TrackJson(const UObject* WorldContextObject,
		const FString& EventName, const FString& JsonPayload);

	UFUNCTION(BlueprintCallable, Category="GameRebellion|Events",
		meta=(WorldContext="WorldContextObject"))
	static int32 TrackLevelUp(const UObject* WorldContextObject, double Level);

	UFUNCTION(BlueprintCallable, Category="GameRebellion|Events",
		meta=(WorldContext="WorldContextObject"))
	static int32 TrackAchievement(const UObject* WorldContextObject, const FString& Id);

	UFUNCTION(BlueprintCallable, Category="GameRebellion|Events",
		meta=(WorldContext="WorldContextObject"))
	static int32 TrackTransaction(const UObject* WorldContextObject,
		double Amount, const FString& Currency,
		const FString& Type = TEXT(""), const FString& Description = TEXT(""));

	UFUNCTION(BlueprintCallable, Category="GameRebellion",
		meta=(WorldContext="WorldContextObject"))
	static int32 SetConsent(const UObject* WorldContextObject, bool bGranted);

	UFUNCTION(BlueprintCallable, Category="GameRebellion",
		meta=(WorldContext="WorldContextObject"))
	static int32 FlushEvents(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category="GameRebellion",
		meta=(WorldContext="WorldContextObject"))
	static int32 GetSDKState(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category="GameRebellion",
		meta=(WorldContext="WorldContextObject"))
	static FString GetSDKLastError(const UObject* WorldContextObject);

private:
	static class UGameRebellionSubsystem* GetSubsystem(const UObject* WorldContextObject);
};
