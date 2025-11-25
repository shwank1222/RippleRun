// Copyright TeamName. All Rights Reserved.


#include "RRReplaySubsystem.h"

#include "RRGameState.h"
#include "Gameplay/Stone/SkippingStone.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

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

	GetGameInstance()->PlayReplay(ReplayName);

	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ThisClass::ConnectReplayCamera, 0.5f, false);
}

void URRReplaySubsystem::ConnectReplayCamera()
{
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClassWithTag(GetWorld(), AActor::StaticClass(), TEXT("ReplayCamera"), Actors);

	if (Actors.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("===== No replay camera found! ====="));
		return;
	}

	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		PC->SetViewTargetWithBlend(Actors[0]);
	}

	GetWorld()->GetTimerManager().SetTimer(TestTimerHandle, this, &ThisClass::TestLog, 0.1f, true);
}

void URRReplaySubsystem::TestLog()
{
	UE_LOG(LogTemp, Warning, TEXT("===== Test Log! ====="));

	if (ARRGameState* GS = GetWorld()->GetGameState<ARRGameState>())
	{
		if (IsValid(GS->ReplayTargetStone))
		{
			// UE_LOG(LogTemp, Warning, TEXT("===== Test Value: %.2f ====="), GS->ReplayTargetStone->TestValue);
		}
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void URRReplaySubsystem::OnReplayPlaybackComplete(UWorld* World)
{
	UE_LOG(LogTemp, Warning, TEXT("===== Replay completed! ====="));
	FNetworkReplayDelegates::OnReplayPlaybackComplete.RemoveAll(this);

	OnReplayCompleted.Broadcast();

	GetWorld()->GetTimerManager().ClearTimer(TestTimerHandle);
}
