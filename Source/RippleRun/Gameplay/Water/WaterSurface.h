#pragma once

#include "GameFramework/Actor.h"
#include "WaterSurface.generated.h"

class UBoxComponent;
class ASkippingStone;
class UBoxComponent;

UCLASS()
class RIPPLERUN_API AWaterSurface : public AActor
{
    GENERATED_BODY()

public:
    AWaterSurface();

protected:
    virtual void BeginPlay() override;

#pragma region Components
protected:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water|Visual")
    UStaticMeshComponent* WaterMesh;

#pragma endregion

#pragma region Parameters
public:
    // 스플래시 / 파동 세기
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water|Effect")
    float WaveStrength = 10.f;

    // 이펙트
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water|Effect")
    UParticleSystem* SplashEffect;

    UPROPERTY(VisibleAnywhere, Category = "Water|Collision")
    UBoxComponent* KillZone;

#pragma endregion

#pragma region Functions

public:
    UFUNCTION(BlueprintCallable)
    void GenerateWave(const FVector& HitLocation, float ImpactStrength);

    
protected:

    UFUNCTION()
    void HandleStoneOverlap(
        UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult
    );

	UFUNCTION()
    void HandleKillZoneOverlap(
        UPrimitiveComponent* OverlappedComp, 
        AActor* OtherActor, 
        UPrimitiveComponent* OtherComp, 
        int32 OtherBodyIndex, 
        bool bFromSweep, 
        const FHitResult& SweepResult);


    UFUNCTION(BlueprintImplementableEvent, Category = "Water|Wave")
    void DrawRippleToRT(
        UTextureRenderTarget2D* Target,
        UMaterialInterface* WriterMaterial,
        const FVector2D& HitUV,
        float Intensity,
        float Radius
    );

#pragma endregion
};
