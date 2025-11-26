// Copyright TeamName. All Rights Reserved.


#include "RRReplaySubsystem.h"

#include "Engine/DemoNetDriver.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Player/RRReplayPlayerController.h"

static const FString ReplayName = TEXT("ReplayTest");

void URRReplaySubsystem::StartRecordingReplay()
{
	UE_LOG(LogTemp, Warning, TEXT("===== Start recording replay! ====="));
	GetGameInstance()->StartRecordingReplay(ReplayName, FString());
}

void URRReplaySubsystem::StopRecordingReplay()
{
	GetGameInstance()->StopRecordingReplay();
	UE_LOG(LogTemp, Warning, TEXT("===== Stop recording replay! ====="));
}

void URRReplaySubsystem::PlayReplay()
{
	UE_LOG(LogTemp, Warning, TEXT("===== Play replay! ====="));

	FNetworkReplayDelegates::OnReplayPlaybackComplete.AddUObject(this, &ThisClass::OnReplayPlaybackComplete);

	bIsReplaying = true;
	
	GetGameInstance()->PlayReplay(ReplayName);
	
	/* Check DemoNetDriver */
	GetWorld()->GetTimerManager().SetTimer(CheckDemoNetDriverTimerHandle, this, &ThisClass::CheckDemoNetDriver, 0.1f, true);
}

AActor* URRReplaySubsystem::GetReplayCamera() const
{
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClassWithTag(GetWorld(), AActor::StaticClass(), TEXT("ReplayCamera"), Actors);

	if (Actors.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("===== No replay camera found! ====="));
		return nullptr;
	}
	
	return Actors[0];
}

void URRReplaySubsystem::StartReplayWorld() const
{
	GetWorld()->GetWorldSettings()->SetPauserPlayerState(nullptr);
}

void URRReplaySubsystem::StopReplayWorld(APlayerController* PlayerController) const
{
	GetWorld()->GetWorldSettings()->SetPauserPlayerState(PlayerController->PlayerState);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void URRReplaySubsystem::OnReplayPlaybackComplete(UWorld* World)
{
	bIsReplaying = false;
	
	UE_LOG(LogTemp, Warning, TEXT("===== Replay completed! ====="));
	FNetworkReplayDelegates::OnReplayPlaybackComplete.RemoveAll(this);

	OnReplayCompleted.Broadcast();
}

void URRReplaySubsystem::CheckDemoNetDriver()
{
	if (UDemoNetDriver* DemoDriver = GetWorld()->GetDemoNetDriver())
	{
		if (DemoDriver->IsPlaying())
		{
			GetWorld()->GetTimerManager().ClearTimer(CheckDemoNetDriverTimerHandle);
			
			/* Check ReplayPlayerController */
			GetWorld()->GetTimerManager().SetTimer(CheckReplayPlayerControllerTimerHandle, this, &ThisClass::CheckReplayPlayerController, 0.1f, true);
		}
	}
}

void URRReplaySubsystem::CheckReplayPlayerController()
{
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		StopReplayWorld(PC);
		
		if (ARRReplayPlayerController* ReplayPC = Cast<ARRReplayPlayerController>(PC))
		{
			AActor* ReplayCamera = GetReplayCamera();
			
			if (!IsValid(ReplayCamera))
			{
				return;
			}
			
			ReplayPC->SetViewTarget(ReplayCamera);
			ReplayPC->ShowReplayWidget();
			
			GetWorld()->GetTimerManager().ClearTimer(CheckReplayPlayerControllerTimerHandle);
		}
	}
}
