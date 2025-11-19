// Copyright TeamName. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "RRGameUserSettings.generated.h"

UCLASS(BlueprintType)
class RIPPLERUN_API URRGameUserSettings : public UGameUserSettings
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Audio")
	FORCEINLINE void SetMasterVolume(const float Volume) { MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f); }
	UFUNCTION(BlueprintCallable, Category = "Audio")
	FORCEINLINE void SetMusicVolume(const float Volume) { MusicVolume = FMath::Clamp(Volume, 0.0f, 1.0f); }
	UFUNCTION(BlueprintCallable, Category = "Audio")
	FORCEINLINE void SetSFXVolume(const float Volume) { SFXVolume = FMath::Clamp(Volume, 0.0f, 1.0f); }
	
	UFUNCTION(BlueprintPure, Category = "Audio")
	FORCEINLINE float GetMasterVolume() const { return MasterVolume; }
	UFUNCTION(BlueprintPure, Category = "Audio")
	FORCEINLINE float GetMusicVolume() const { return MusicVolume; }
	UFUNCTION(BlueprintPure, Category = "Audio")
	FORCEINLINE float GetSFXVolume() const { return SFXVolume; }
	
private:
	UPROPERTY(Config)
	float MasterVolume = 1.0f;
	UPROPERTY(Config)
	float MusicVolume = 1.0f;
	UPROPERTY(Config)
	float SFXVolume = 1.0f;
};
