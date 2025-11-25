// Copyright TeamName. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RRReplaySubsystem.generated.h"

class ASkippingStone;
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReplayCompleted);

UCLASS()
class RIPPLERUN_API URRReplaySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Replay")
	void StartRecordingReplay();
	UFUNCTION(BlueprintCallable, Category = "Replay")
	void StopRecordingReplay();
	UFUNCTION(BlueprintCallable, Category = "Replay")
	void PlayReplay();
	
	UPROPERTY(BlueprintAssignable, Category = "Replay")
	FOnReplayCompleted OnReplayCompleted;
	
private:
	UFUNCTION()
	void OnReplayPlaybackComplete(UWorld* World);
	
	UFUNCTION()
	void ConnectReplayCamera();
	
	UFUNCTION()
	void TestLog();
	
	FTimerHandle TestTimerHandle;
};
