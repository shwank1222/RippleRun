#pragma once

#include "GameFramework/Actor.h"
#include "SkippingStone.generated.h"

class AWaterSurface;
class UArrowComponent;

UENUM(BlueprintType)
enum class EStoneState : uint8
{
    None,
    Airborne,
    WaterContact,
    Bouncing,
    Glide,
    Sunk
};

UENUM(BlueprintType)
enum class EFluidType : uint8
{
    Air,
    Water
};

namespace
{
    bool IsFiniteVector(const FVector& V)
    {
        return FMath::IsFinite(V.X) && FMath::IsFinite(V.Y) && FMath::IsFinite(V.Z);
    }

    float SafePositive(float Value, float Default = 1.0f)
    {
        return (Value > KINDA_SMALL_NUMBER) ? Value : Default;
    }
}


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStoneFinished, ASkippingStone*, Stone);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStoneBounced, ASkippingStone*, Stone, int32, Count);

UCLASS()
class RIPPLERUN_API ASkippingStone : public AActor
{
    GENERATED_BODY()

public:
    ASkippingStone();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

#pragma region Components
public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stone|Component")
    UStaticMeshComponent* StoneMeshComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stone|Component")
    UArrowComponent* ArrowComp;

#pragma endregion

#pragma region Setters & Getters
public:
	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetThrowAngle(float NewAngle) { ThrowAngle = NewAngle; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetSpinRate(float NewSpinRate) { SpinRate = NewSpinRate; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetInitialSpeed(float NewSpeed) { InitialSpeed = NewSpeed; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetMass(float NewMass) { Mass = NewMass; }

	UFUNCTION(BlueprintCallable)
    void SetRadius(float NewRadius);
	UFUNCTION(BlueprintCallable)
	void SetThickness(float NewThickness);
    UFUNCTION(BlueprintCallable)
    void SetTilt(float NewPitch, float NewRoll);

	// Getters
    UFUNCTION(BlueprintCallable, Category = "Stone|Stat")
	FORCEINLINE float GetRadius() const { return Radius; }

	UFUNCTION(BlueprintCallable, Category = "Stone|Stat")
	FORCEINLINE float GetThickness() const { return Thickness; }

	UFUNCTION(BlueprintCallable, Category = "Stone|Stat")
	FORCEINLINE float GetMass() const { return Mass; }


	UFUNCTION(BlueprintCallable, Category = "Stone|Runtime")
	FORCEINLINE float GetTimeElapsed() const { return TimeElapsed; }

	UFUNCTION(BlueprintCallable, Category = "Stone|Runtime")
	FORCEINLINE int32 GetBounceCount() const { return BounceCount; }

	UFUNCTION(BlueprintCallable, Category = "Stone|Runtime")
	FORCEINLINE float GetForwardDistance() const { return ForwardDistance; }

	UFUNCTION(BlueprintCallable, Category = "Stone|Runtime")
	FORCEINLINE float GetFinalDistance() const { return FinalDistance; }

    UFUNCTION(BlueprintCallable, Category = "Stone|Runtime")
	FORCEINLINE float GetSpinRate() const { return SpinRate; }

    UFUNCTION(BlueprintCallable, Category = "Stone|Runtime")
    FORCEINLINE FVector GetContactPoint() const { return ContactPoint; }

#pragma endregion

#pragma region Parameters
protected:

    // Throw Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone|Throw")
    float InitialSpeed = 800.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone|Throw")
    float ThrowAngle = 45.f;


    // Physical Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone|Physic")
    float Mass = 0.15f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone|Physic")
    float Radius = 0.03f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone|Physic")
    float Thickness = 0.01f;

    // 기준 메쉬 사이즈 (스케일링용)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone|Physic")
    float BaseRadius = 5.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone|Physic")
    float BaseThickness = 2.f;

    // Fluid Medium Properties
    UPROPERTY(EditAnywhere, Category = "Stone|Fluid")
    float AirDensity = 0.0012f;

    UPROPERTY(EditAnywhere, Category = "Stone|Fluid")
    float WaterDensity = 0.0010f;


    // Glide Conditions
    UPROPERTY(EditAnywhere, Category = "Stone|Glide")
    float MinGlideSpeed = 700.f;

    UPROPERTY(EditAnywhere, Category = "Stone|Glide")
    float MinGlideSpin = 12.f;

    UPROPERTY(EditAnywhere, Category = "Stone|Glide")
    float GlideThreshold = 5.f;

    UPROPERTY(EditAnywhere, Category = "Stone|Glide")
    float GlideMinVz = 20.f;


    // Drag & Lift
    UPROPERTY(EditAnywhere, Category = "Stone|Aerodynamics")
    float DragCoeffAir = 0.5f;

    UPROPERTY(EditAnywhere, Category = "Stone|Aerodynamics")
    float DragCoeffWater = 1.0f;

    UPROPERTY(EditAnywhere, Category = "Stone|Aerodynamics")
    float LiftCoeffMagnus = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone|Aerodynamics")
    float LiftScale = 0.0005f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone|Curve")
	float CurveStrength = 0.15f;

    // Spin Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone|Spin")
    float SpinRate = 20.f;       // rad/s

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone|Spin")
    float AngularDrag = 0.1f;


    // Resistance (linear drag)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone|Resistance")
    float LinearDrag = 0.05f;


	// Bounce Properties and Conditions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone|Bounce")
    float EnergyRetention = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone|Bounce")
    float SpinDamping = 0.9f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone|Bounce|Conditions")
    float CriticalBounceAngle = 25.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stone|Bounce|Conditions")
	float MinBounceSpin = 8.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stone|Bounce|Conditions")
	float MinBounceSpeed = 900.f;

    // Finish / End Conditions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone|Finish")
    float EndSpeedThreshold = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone|Finish")
    float EndSinkDelay = 1.0f;


private:
    float TiltPitch = 0.f;
    float TiltRoll = 0.f;

#pragma endregion

#pragma region Stone Logic
public:
    UFUNCTION(BlueprintCallable, Category = "Stone|Physics")
    void ThrowStone();

    void HandleWaterContact(AWaterSurface* Water, const FVector& HitPoint);

    void ForceEndSkipping();

private:
    void ApplyPhysics(float DeltaTime);

    void TickAirborne(float DeltaTime);
    void TickBouncing(float DeltaTime);
    void TickGlide(float DeltaTime);
	void TickSunk(float DeltaTime);
    void UpdateTilt(float DeltaTime);

	FVector ComputeDragForce(EFluidType Fluid) const;
    FVector ComputeMagnusLiftForce() const;

    float ComputeIncidenceAngle() const;

    bool ShouldBounce() const;

    void SetStoneState(EStoneState NewState);

#pragma endregion

#pragma region Utility Functions
public:
	UFUNCTION(BlueprintCallable, Category = "Stone|Utility")
    void MakeRandomStats();

private:
    float GetArea() const;

#pragma region Runtime Variables

private:
    FVector Velocity;
    FVector SpinAxis;

    EStoneState State = EStoneState::None;

    // For WaterContact
    AWaterSurface* LastWater = nullptr;
    FVector ContactPoint;

    // Internal
	UPROPERTY(Replicated)
	int32 BounceCount = 0;
    int32 BounceFrameCounter = 0;

	UPROPERTY(Replicated)
    float TimeElapsed = 0.f;
    float SunkElapsed = 0.f;
    
    FVector ThrowStartLocation;
    FVector ThrowDirection;

	UPROPERTY(Replicated)
    float FinalDistance = 0.f;
	UPROPERTY(Replicated)
    float ForwardDistance = 0.f;

    FVector PreviousLocation;

    bool bHasThrown = false;

#pragma endregion

#pragma region Delegates

    UPROPERTY(BlueprintAssignable, Category = "Stone|Event")
    FOnStoneFinished OnStoneFinished;
	UPROPERTY(BlueprintAssignable, Category = "Stone|Event")
	FOnStoneBounced OnStoneBounced;

#pragma endregion
};
