#include "SkippingStone.h"
#include "Gameplay/Water/WaterSurface.h"
#include "Components/StaticMeshComponent.h"
#include "Components/ArrowComponent.h"

ASkippingStone::ASkippingStone()
{
    PrimaryActorTick.bCanEverTick = true;

    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

    StoneMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StoneMeshComp"));
    StoneMeshComp->SetupAttachment(RootComponent);
    StoneMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    ArrowComp = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComp"));
    ArrowComp->SetupAttachment(RootComponent);
    ArrowComp->SetHiddenInGame(true);

    SpinAxis = FVector::UpVector;
}

void ASkippingStone::BeginPlay()
{
    Super::BeginPlay();
    SetActorTickEnabled(false);
}

void ASkippingStone::ThrowStone()
{
    FRotator ThrowRot = GetActorRotation();
    ThrowRot.Pitch += ThrowAngle;
    Velocity = ThrowRot.Vector() * InitialSpeed;

    SetStoneState(EStoneState::Airborne);
    SetActorTickEnabled(true);

    UE_LOG(LogTemp, Log, TEXT("Stone Thrown"));
}

void ASkippingStone::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    ApplyPhysics(DeltaTime);

    // Update position
    SetActorLocation(GetActorLocation() + Velocity * DeltaTime);

    // Simple yaw spin
    AddActorLocalRotation(FRotator(0.f, SpinRate * DeltaTime, 0.f));
}

#pragma region Physics

void ASkippingStone::ApplyPhysics(float DeltaTime)
{
    switch (State)
    {
    case EStoneState::Airborne:
        TickAirborne(DeltaTime);
        break;

    case EStoneState::WaterContact:
        if (ShouldBounce())
        {
            SetStoneState(EStoneState::Bouncing);
            BounceFrameCounter = 1;
        }
        else
        {
            SetStoneState(EStoneState::Sunk);
        }
        break;

    case EStoneState::Bouncing:
        TickBouncing(DeltaTime);
        break;

    case EStoneState::Glide:
        TickGlide(DeltaTime);
        break;

    case EStoneState::Sunk:
        float SinkGravity = -300.f;

        Velocity.Z += SinkGravity * DeltaTime;

        float WaterDrag = 3.0f;
        Velocity -= Velocity * WaterDrag * DeltaTime;

        SpinRate *= 0.95f;

        break;;
    }
}

void ASkippingStone::TickAirborne(float DeltaTime)
{
    // Gravity
    Velocity.Z -= 980.f * DeltaTime;

    // Drag
    Velocity -= Velocity * LinearDrag * DeltaTime;
    SpinRate -= SpinRate * AngularDrag * DeltaTime;
}

float ASkippingStone::ComputeIncidenceAngle() const
{
    FVector V = Velocity.GetSafeNormal();

    float sinTheta = FMath::Abs(V.Z);
    return FMath::RadiansToDegrees(FMath::Asin(sinTheta));
}

float ASkippingStone::ComputeLift() const
{
    return SpinRate * Velocity.Size() * LiftScale;
}

void ASkippingStone::SetStoneState(EStoneState NewState)
{
    if (State != NewState)
    {
        UE_LOG(LogTemp, Log, TEXT("[StoneState] %s -> %s"),
            *UEnum::GetValueAsString(State),
            *UEnum::GetValueAsString(NewState)
        );
        State = NewState;
    }
}

bool ASkippingStone::ShouldBounce() const
{
    float Angle = ComputeIncidenceAngle();

    // May need to be adjusted based on testing
    bool bBounce =
		Angle < CriticalBounceAngle && 
        SpinRate > 2.f &&
        Velocity.Z > -1000.f;

    UE_LOG(LogTemp, Log,
        TEXT("[ShouldBounce] Result: %s  | Angle=%.2f (Critical=%.2f) | Spin=%.2f | Vz=%.2f"),
        bBounce ? TEXT("TRUE") : TEXT("FALSE"),
        Angle, CriticalBounceAngle,
        SpinRate,
        Velocity.Z
    );

    return bBounce;
}

void ASkippingStone::TickBouncing(float DeltaTime)
{
    if (BounceFrameCounter > 0)
    {
        FVector Normal = FVector::UpVector;

        // 기본 반사
        FVector Ref = Velocity.MirrorByVector(Normal);

        // 에너지 유지
        Ref *= EnergyRetention;

        // Lift 적용
        Ref.Z += ComputeLift();

        Velocity = Ref;

        // 스핀 감소
        SpinRate *= SpinDamping;

        BounceFrameCounter--;
        if (BounceFrameCounter == 0)
        {
            SetStoneState(EStoneState::Glide);
        }
    }
}

void ASkippingStone::TickGlide(float DeltaTime)
{
    // 미세한 Lift → 수면 위 스침
    Velocity.Z += ComputeLift() * DeltaTime;

    // 물 저항
    Velocity -= Velocity * (LinearDrag * 2.f) * DeltaTime;

    // Spin 감소
    SpinRate -= SpinRate * (AngularDrag * 0.5f) * DeltaTime;

    // Z 속도가 양수가 되면 다시 Airborne
    if (Velocity.Z > 0)
    {
        SetStoneState(EStoneState::Airborne);
    }
}

#pragma endregion

#pragma region Water Contact

void ASkippingStone::EnterWaterContact(AWaterSurface* Water, const FVector& HitPoint)
{
    LastWater = Water;
    ContactPoint = HitPoint;

    if (State == EStoneState::Airborne)
    {
        SetStoneState(EStoneState::WaterContact);
    }
}

#pragma endregion
