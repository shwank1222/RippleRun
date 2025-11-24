#pragma once

#include "GameFramework/Actor.h"
#include "WaterSurface.generated.h"

class UBoxComponent;
class ASkippingStone;

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water|Collision")
    UBoxComponent* WaterCollision;

#pragma endregion

#pragma region Parameters
public:

    // 물 저항 (글라이드, 접촉 시 감쇠에 사용)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water|Physics")
    float Viscosity = 0.2f;

    // 스플래시 / 파동 세기
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water|Effect")
    float WaveStrength = 10.f;

    // 이펙트
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water|Effect")
    UParticleSystem* SplashEffect;

#pragma endregion

#pragma region Functions
public:

    UFUNCTION(BlueprintCallable)
    void GenerateWave(const FVector& HitLocation, float ImpactStrength);

private:
    UFUNCTION()
    void HandleStoneOverlap(
        UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult
    );

#pragma endregion
};
