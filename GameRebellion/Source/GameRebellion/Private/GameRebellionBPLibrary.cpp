#include "GameRebellionBPLibrary.h"
#include "GameRebellionSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

UGameRebellionSubsystem* UGameRebellionBPLibrary::GetSubsystem(const UObject* WorldContextObject)
{
	if (!WorldContextObject) return nullptr;

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if (!World) return nullptr;

	UGameInstance* GI = World->GetGameInstance();
	if (!GI) return nullptr;

	return GI->GetSubsystem<UGameRebellionSubsystem>();
}

int32 UGameRebellionBPLibrary::TrackJson(const UObject* WorldContextObject,
	const FString& EventName, const FString& JsonPayload)
{
	if (auto* Sub = GetSubsystem(WorldContextObject))
		return Sub->TrackJson(EventName, JsonPayload);
	return -3;
}

int32 UGameRebellionBPLibrary::TrackLevelUp(const UObject* WorldContextObject, double Level)
{
	if (auto* Sub = GetSubsystem(WorldContextObject))
		return Sub->TrackLevelUp(Level);
	return -3;
}

int32 UGameRebellionBPLibrary::TrackAchievement(const UObject* WorldContextObject, const FString& Id)
{
	if (auto* Sub = GetSubsystem(WorldContextObject))
		return Sub->TrackAchievement(Id);
	return -3;
}

int32 UGameRebellionBPLibrary::TrackTransaction(const UObject* WorldContextObject,
	double Amount, const FString& Currency, const FString& Type, const FString& Description)
{
	if (auto* Sub = GetSubsystem(WorldContextObject))
		return Sub->TrackTransaction(Amount, Currency, Type, Description);
	return -3;
}

int32 UGameRebellionBPLibrary::SetConsent(const UObject* WorldContextObject, bool bGranted)
{
	if (auto* Sub = GetSubsystem(WorldContextObject))
		return Sub->SetConsent(bGranted);
	return -3;
}

int32 UGameRebellionBPLibrary::FlushEvents(const UObject* WorldContextObject)
{
	if (auto* Sub = GetSubsystem(WorldContextObject))
		return Sub->Flush();
	return -3;
}

int32 UGameRebellionBPLibrary::GetSDKState(const UObject* WorldContextObject)
{
	if (auto* Sub = GetSubsystem(WorldContextObject))
		return Sub->GetState();
	return 0;
}

FString UGameRebellionBPLibrary::GetSDKLastError(const UObject* WorldContextObject)
{
	if (auto* Sub = GetSubsystem(WorldContextObject))
		return Sub->GetLastError();
	return TEXT("Subsystem not available");
}
