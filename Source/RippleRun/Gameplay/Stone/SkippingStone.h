#pragma once

#include "GameFramework/Actor.h"
#include "SkippingStone.generated.h"

class AWaterSurface;
class UArrowComponent;

UENUM(BlueprintType)
enum class EStoneState : uint8
{
    Airborne,
    WaterContact,
    Bouncing,   // 1 frame only
    Glide,
    Sunk
};

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

#pragma region Parameters
protected:

    // Throw
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone|Throw")
    float InitialSpeed = 800.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone|Throw")
    float ThrowAngle = 45.f;

    // Physical
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone|Physic")
    float Mass = 0.15f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone|Physic")
    float Radius = 0.03f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stone|Physic")
    float Thickness = 0.01f;

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
    EStoneState State = EStoneState::Airborne;

    // For WaterContact
    AWaterSurface* LastWater = nullptr;
    FVector ContactPoint;

    // Internal
    int32 BounceFrameCounter = 0;

#pragma endregion
};
