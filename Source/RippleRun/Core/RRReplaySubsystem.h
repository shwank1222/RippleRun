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
	
	UFUNCTION(BlueprintCallable, Category = "Replay")
	void StartReplayWorld() const;
	UFUNCTION(BlueprintCallable, Category = "Replay")
	void StopReplayWorld(APlayerController* PlayerController) const;
	
	UPROPERTY(BlueprintAssignable, Category = "Replay")
	FOnReplayCompleted OnReplayCompleted;
	
	UFUNCTION(BlueprintPure, Category = "Replay")
	FORCEINLINE bool IsReplaying() const { return bIsReplaying; }
	
private:
	AActor* GetReplayCamera() const;
	
	UFUNCTION()
	void OnReplayPlaybackComplete(UWorld* World);
	
	UFUNCTION()
	void CheckDemoNetDriver();
	UFUNCTION()
	void CheckReplayPlayerController();
	
	FTimerHandle CheckDemoNetDriverTimerHandle;
	FTimerHandle CheckReplayPlayerControllerTimerHandle;
	
	bool bIsReplaying = false;
};
