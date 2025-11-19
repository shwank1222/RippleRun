#pragma once

#include "GameFramework/Actor.h"
#include "SkippingStone.generated.h"

class UProjectileMovementComponent;
class UArrowComponent;

UCLASS()
class RIPPLERUN_API ASkippingStone : public AActor
{
	GENERATED_BODY()
	
#pragma region Constructor and Overrides
public:	
	ASkippingStone();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

#pragma endregion

#pragma region Components

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stone|Component")
	UStaticMeshComponent* StoneMeshComp;

	//for prototype
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stone|Component")
	UProjectileMovementComponent* ProjectileMovementComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stone|Component")
	UArrowComponent* ArrowComp;

#pragma endregion

#pragma region Stone Parameters
protected:
	// --- Initial Throw ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone|Parameters|Throw")
	float InitialSpeed = 3000.0f;          // 던질 때 초기 속도

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone|Parameters|Throw")
	float ThrowAngle = 45.0f;              // 투척 각도 (Pitch 기준)

	// --- Physical Characteristic  ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone|Parameters|Physics")
	float Mass = 0.15f;                     // 돌 질량 (kg)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone|Parameters|Physics")
	float Radius = 0.03f;                   // 돌 반지름 (m)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone|Parameters|Physics")
	float Thickness = 0.01f;                // 돌 두께 (m)

	// --- Rotation / Spin ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone|Parameters|Spin")
	float SpinRate = 20.0f;                 // 회전 속도 (rad/s)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone|Parameters|Spin")
	float LinearDrag = 0.05f;               // 선속도 감쇠

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone|Parameters|Spin")
	float AngularDrag = 0.1f;               // 회전 감쇠

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone|Parameters|Bounce")
	float EnergyRetention = 0.7f;           // 튕긴 후 속도 유지 비율 (0~1)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone|Parameters|Bounce")
	float SpinDamping = 0.9f;               // 튕긴 후 스핀 유지 비율 (0~1)

	// --- Bounce / Resistance ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone|Parameters|Resistance")
	float CriticalBounceAngle = 60.0f;      // 임계 입사각(도), 이하면 튕김

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone|Parameters|Resistance")
	float CurrentMedium = 0.0f;              // 현재 매질 (0: 공기, 1: 물)
	
	// for projectile component prototype
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone|Parameters")
	float Bounciness = 0.3f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone|Parameters")
	float GravityScale = 1.0f;

#pragma endregion

#pragma region Setters and Getters
public:
	UFUNCTION(BlueprintCallable, Category = "Stone|Parameters")
	FORCEINLINE void SetInitialSpeed(float NewSpeed) { InitialSpeed = NewSpeed; }
	UFUNCTION(BlueprintCallable, Category = "Stone|Parameters")
	FORCEINLINE void SetThrowAngle(float NewAngle) { ThrowAngle = NewAngle; }
	UFUNCTION(BlueprintCallable, Category = "Stone|Parameters")
	FORCEINLINE void SetSpinRate(float NewSpinRate) { SpinRate = NewSpinRate; }

	UFUNCTION(BlueprintCallable, Category = "Stone|Parameters")
	FORCEINLINE void SetMass(float NewMass) { Mass = NewMass; }

	UFUNCTION(BlueprintCallable, Category = "Stone|Parameters")
	void SetRadius(float NewMass);
	UFUNCTION(BlueprintCallable, Category = "Stone|Parameters")
	void SetThickness(float NewMass);

#pragma endregion

#pragma region Stone Functionality
	public:
	UFUNCTION(BlueprintCallable, Category = "Stone|Functionality")
	void ThrowStone();
};
