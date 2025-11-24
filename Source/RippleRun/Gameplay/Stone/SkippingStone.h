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

#pragma region Components
public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stone|Component")
    UStaticMeshComponent* StoneMeshComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stone|Component")
    UArrowComponent* ArrowComp;
#pragma endregion

#pragma region Setters
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

#pragma endregion
	
#pragma region Getters
	
	UFUNCTION(BlueprintPure)
	FORCEINLINE float GetMass() const { return Mass; }
	
	UFUNCTION(BlueprintPure)
	FORCEINLINE float GetRadius() const { return Radius; }
	
	UFUNCTION(BlueprintPure)
	FORCEINLINE float GetThickness() const { return Thickness; }
	
#pragma endregion

#pragma region Parameters
protected:

    // Throw
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone|Throw")
    float InitialSpeed = 800.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone|Throw")
    float ThrowAngle = 45.f;

    // Physical
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone|Physic")
	float BaseRadius = 5.f; // Standard mesh radius

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone|Physic")
	float BaseThickness = 2.f; // Standard mesh thickness

    // Spin
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone|Spin")
    float SpinRate = 20.f;  // rad/s

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone|Spin")
    float AngularDrag = 0.1f;

    // Drag
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone|Resistance")
    float LinearDrag = 0.05f;

    // Bounce / skim
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone|Bounce")
    float EnergyRetention = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone|Bounce")
    float SpinDamping = 0.9f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone|Bounce")
    float CriticalBounceAngle = 25.f;

    // Lift tuning
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone|Lift")
    float LiftScale = 0.0005f;

	// Finish conditions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone|Finish")
    float EndSpeedThreshold = 70.f;     

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone|Finish")
    float EndSinkDelay = 1.0f;


private:
    float Mass = 0.15f;
    float Radius = 0.03f;
    float Thickness = 0.01f;

	float TiltPitch = 0.f;
	float TiltRoll = 0.f;

#pragma endregion

#pragma region Stone Logic
public:
    UFUNCTION(BlueprintCallable)
    void ThrowStone();

    void EnterWaterContact(AWaterSurface* Water, const FVector& HitPoint);

private:
    void ApplyPhysics(float DeltaTime);
    void TickAirborne(float DeltaTime);
    void TickBouncing(float DeltaTime);
    void TickGlide(float DeltaTime);

    bool ShouldBounce() const;
    float ComputeIncidenceAngle() const;
    float ComputeLift() const;
    
    void SetStoneState(EStoneState NewState);

#pragma endregion

#pragma region Runtime Variables
private:
    FVector Velocity;
    FVector SpinAxis;
    EStoneState State = EStoneState::None;

    // For WaterContact
    AWaterSurface* LastWater = nullptr;
    FVector ContactPoint;

    // Internal
	int32 BounceCount = 0;
    int32 BounceFrameCounter = 0;

    float SunkElapsed = 0.f;

	bool bHasThrown = false;

#pragma endregion

#pragma region Delegates
    UPROPERTY(BlueprintAssignable, Category = "Stone|Event")
    FOnStoneFinished OnStoneFinished;
	UPROPERTY(BlueprintAssignable, Category = "Stone|Event")
	FOnStoneBounced OnStoneBounced;

#pragma endregion
};
