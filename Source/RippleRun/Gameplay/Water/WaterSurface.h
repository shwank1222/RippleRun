#pragma once

#include "GameFramework/Actor.h"
#include "WaterSurface.generated.h"

class UBoxComponent;

UCLASS()
class RIPPLERUN_API AWaterSurface : public AActor
{
	GENERATED_BODY()
	
#pragma region Constructor and Overrides
public:
    AWaterSurface();

protected:
    virtual void BeginPlay() override;

#pragma endregion

#pragma region Components
protected:
    // === Visual ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water|Visual")
    UStaticMeshComponent* WaterMesh;

    // === Collision ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water|Collision")
    UBoxComponent* WaterCollision;

#pragma endregion

#pragma region Water Parameters
    // === 물리적 파라미터: 튕김 관련 ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water|Physics|Bounce")
    float SurfaceTension = 0.5f;           // 튕김 난이도에 영향을 줌 (0~1)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water|Physics|Bounce")
    float EnergyAbsorption = 0.3f;         // 돌 속도 감소율 (0~1)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water|Physics|Bounce")
    float SpinDamping = 0.8f;              // 돌 스핀 감소율 (0~1)

    // === 물리적 파라미터: 물결/저항 관련 ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water|Physics|Wave")
    float Viscosity = 0.2f;                // 물 저항력

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water|Physics|Wave")
    float WaveHeight = 10.0f;              // 기본 물결 높이

    // === 효과/이펙트 ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water|Effect")
    UParticleSystem* SplashEffect;         // 충돌 시 생성할 스플래시 이펙트

#pragma endregion

#pragma region Water Functions
    // === 함수 ===
    UFUNCTION(BlueprintCallable, Category = "Water|Effect")
    void GenerateWave(const FVector& HitLocation, float ImpactStrength);
#pragma endregion
};
