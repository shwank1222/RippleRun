#include "SkippingStone.h"
#include "Gameplay/Water/WaterSurface.h"
#include "Components/StaticMeshComponent.h"
#include "Components/ArrowComponent.h"
#include "Net/UnrealNetwork.h"

ASkippingStone::ASkippingStone()
{
    PrimaryActorTick.bCanEverTick = true;
    
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

    StoneMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StoneMeshComp"));
    StoneMeshComp->SetupAttachment(RootComponent);
    StoneMeshComp->SetCollisionProfileName("OverlapAll");
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

void ASkippingStone::MakeRandomStats()
{
    float NewRadius = FMath::FRandRange(3.f, 10.f);
    SetRadius(NewRadius);

    float NewThickness = FMath::FRandRange(1.f, 3.5f);
    SetThickness(NewThickness);

	float NewMass = FMath::FRandRange(0.05f, 0.3f);
	SetMass(NewMass);
}

void ASkippingStone::SetRadius(float NewRadius)
{
    Radius = NewRadius;

    FVector Scale = StoneMeshComp->GetRelativeScale3D();

    float RadiusScale = Radius / BaseRadius;

    Scale.X = RadiusScale;
    Scale.Y = RadiusScale;

    StoneMeshComp->SetRelativeScale3D(Scale);
}

void ASkippingStone::SetThickness(float NewThickness)
{
    Thickness = NewThickness;

    FVector Scale = StoneMeshComp->GetRelativeScale3D();

    float ThicknessScale = Thickness / BaseThickness;

    Scale.Z = ThicknessScale;

    StoneMeshComp->SetRelativeScale3D(Scale);
}

void ASkippingStone::SetTilt(float NewPitch, float NewRoll)
{
    FRotator NewRot = StoneMeshComp->GetRelativeRotation();
    NewRot.Pitch = FMath::Clamp(NewRot.Pitch + NewPitch, -5.0f, 30.0f);
    NewRot.Roll = FMath::Clamp(NewRot.Roll + NewRoll, -20.0f, 20.0f);
    
    TiltPitch = NewRot.Pitch;
	TiltRoll = NewRot.Roll;

    StoneMeshComp->SetRelativeRotation(NewRot);
}

void ASkippingStone::ThrowStone()
{
    if (State != EStoneState::None) { return; }

    TimeElapsed = 0.f;
    ForwardDistance = 0.f;

    ThrowStartLocation = GetActorLocation();
    ThrowDirection = GetActorForwardVector().GetSafeNormal();

	FRotator CurrnetRotation = StoneMeshComp->GetRelativeRotation();
	TiltPitch = CurrnetRotation.Pitch;
	TiltRoll = CurrnetRotation.Roll;

    FRotator ThrowRot = GetActorRotation();
    ThrowRot.Pitch += ThrowAngle;
    Velocity = ThrowRot.Vector() * InitialSpeed;

    SpinAxis = StoneMeshComp->GetUpVector().GetSafeNormal();

    SetStoneState(EStoneState::Airborne);
    SetActorTickEnabled(true);

    bHasThrown = true;
    UE_LOG(LogTemp, Log, TEXT("Stone Thrown"));
}

void ASkippingStone::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bHasThrown) return;

    if (GetWorld()->IsPlayingReplay())
    {
        return;
    }
    
    TimeElapsed += DeltaTime;

    ForwardDistance =
        FVector::DotProduct(GetActorLocation() - ThrowStartLocation, ThrowDirection);

    ApplyPhysics(DeltaTime);

    // Update position
    SetActorLocation(GetActorLocation() + Velocity * DeltaTime);

    // Simple yaw spin
    AddActorLocalRotation(FRotator(0.f, SpinRate * DeltaTime, 0.f));
    
    ForceNetUpdate();
}

void ASkippingStone::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(ASkippingStone, BounceCount);
    DOREPLIFETIME(ASkippingStone, TimeElapsed);
    DOREPLIFETIME(ASkippingStone, FinalDistance);
    DOREPLIFETIME(ASkippingStone, ForwardDistance);
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
			BounceCount += 1;
			OnStoneBounced.Broadcast(this, BounceCount);

            BounceFrameCounter = 1;
        }
        else
        {
            SetStoneState(EStoneState::Sunk);
			FinalDistance = ForwardDistance;
        }
        break;

    case EStoneState::Bouncing:
        TickBouncing(DeltaTime);
        break;

    case EStoneState::Glide:
        TickGlide(DeltaTime);
        break;

    case EStoneState::Sunk:
		TickSunk(DeltaTime);
        break;;
    }
}

void ASkippingStone::TickAirborne(float DeltaTime)
{
    const float g = 980.f; // cm/s^2

    FVector GravityForce = FVector(0.f, 0.f, -Mass * g);
    FVector DragForce = ComputeDragForce(EFluidType::Air);
    FVector LiftForce = ComputeMagnusLiftForce();

    FVector TotalForce = GravityForce + DragForce + LiftForce;

    if (!IsFiniteVector(TotalForce))
    {
        UE_LOG(LogTemp, Error, TEXT("TotalForce in Airborne is non-finite: %s"), *TotalForce.ToString());
        TotalForce = FVector::ZeroVector;
    }

    FVector Accel = TotalForce / Mass;

    if (!IsFiniteVector(Accel))
    {
        UE_LOG(LogTemp, Error, TEXT("Accel in Airborne is non-finite: %s"), *Accel.ToString());
        Accel = FVector::ZeroVector;
    }

    Velocity += Accel * DeltaTime;

    if (!IsFiniteVector(Velocity))
    {
        UE_LOG(LogTemp, Error, TEXT("Velocity became non-finite in Airborne! V=%s"), *Velocity.ToString());
        Velocity = FVector::ZeroVector;
        SetStoneState(EStoneState::Sunk); // 안전하게 종료
        return;
    }
}


void ASkippingStone::TickBouncing(float DeltaTime)
{
    if (BounceFrameCounter <= 0)
    {
        SetStoneState(EStoneState::Glide);
        return;
    }

    const FVector Normal = FVector::UpVector;

    // v' = v - (1+e)(v·n)n
    float Vn = FVector::DotProduct(Velocity, Normal);
    float Restitution = EnergyRetention; // 0~1

    FVector Reflected = Velocity - (1.f + Restitution) * Vn * Normal;

    // 수면과 접촉하며 수평 속도도 약간 줄어듦 (물 저항)
    const float TangentialDamping = 0.9f;
    const FVector VnVec = FVector::DotProduct(Reflected, Normal) * Normal;
    const FVector VtVec = Reflected - VnVec;

    Reflected = VtVec * TangentialDamping + VnVec;

    // 리프트(마그누스) 보정
    const FVector LiftForce = ComputeMagnusLiftForce();
    FVector LiftAccel = LiftForce / Mass;

    if (!IsFiniteVector(LiftAccel))
    {
        LiftAccel = FVector::ZeroVector;
    }

    Reflected += LiftAccel * DeltaTime;

    if (!IsFiniteVector(Reflected))
    {
        UE_LOG(LogTemp, Error, TEXT("Reflected velocity is non-finite in Bouncing!"));
        Reflected = FVector::ZeroVector;
        SetStoneState(EStoneState::Sunk);
        return;
    }

    Velocity = Reflected;
    SpinRate *= SpinDamping;

    BounceFrameCounter--;

    UpdateTilt(DeltaTime);

    if (BounceFrameCounter <= 0)
    {
        SetStoneState(EStoneState::Glide);
    }
}

void ASkippingStone::TickGlide(float DeltaTime)
{
    const float g = 980.f;

    FVector GravityForce = FVector(0.f, 0.f, -Mass * g);
    FVector LiftForce = ComputeMagnusLiftForce();

    // 물 위 드래그 (공기보다 훨씬 큼)
    FVector WaterDragForce = ComputeDragForce(EFluidType::Water); // 공기 드래그와 비슷한 식 + 계수만 크게

    FVector TotalForce = GravityForce + LiftForce + WaterDragForce;

    if (!IsFiniteVector(TotalForce))
    {
        UE_LOG(LogTemp, Error, TEXT("TotalForce in Glide is non-finite: %s"), *TotalForce.ToString());
        TotalForce = FVector::ZeroVector;
    }

    FVector Accel = TotalForce / Mass;

    if (!IsFiniteVector(Accel))
    {
        UE_LOG(LogTemp, Error, TEXT("Accel in Glide is non-finite: %s"), *Accel.ToString());
        Accel = FVector::ZeroVector;
    }

    Velocity += Accel * DeltaTime;

    // 수면 위에 머무르도록 너무 내려가면 Yaw/롤 조금 바꿔서 안정화시키는 것도 가능 (추후)

    if (!IsFiniteVector(Velocity))
    {
        UE_LOG(LogTemp, Error, TEXT("Velocity became non-finite in Glide! V=%s"), *Velocity.ToString());
        Velocity = FVector::ZeroVector;
        SetStoneState(EStoneState::Sunk);
        return;
    }

    // 스핀은 서서히 감소
    SpinRate -= SpinRate * (AngularDrag * 0.5f) * DeltaTime;

    UpdateTilt(DeltaTime);

    if (Velocity.Size() < MinGlideSpeed || SpinRate < MinGlideSpin)
    {
        SetStoneState(EStoneState::Sunk);
        FinalDistance = ForwardDistance;
        UE_LOG(LogTemp, Log, TEXT("[Glide] Speed/Spin too low -> Sunk"));
        return;
    }

    if (Velocity.Z > GlideMinVz && GetActorLocation().Z >= ContactPoint.Z + GlideThreshold)
    {
        SetStoneState(EStoneState::Airborne);
        UE_LOG(LogTemp, Log, TEXT("[Glide] Lifted up -> Airborne"));
        return;
    }
}

void ASkippingStone::TickSunk(float DeltaTime)
{
    const float g = 980.f;

    FVector GravityForce = FVector(0.f, 0.f, -Mass * g);
    FVector DragForce = ComputeDragForce(EFluidType::Water);

    // 아주 간단한 부력 근사: 거의 완전히 잠겼다고 보고 비율로 조정
    float SubmergedRatio = 1.0f; // 나중에 높이/두께로 계산 가능
    float VolumeApprox = PI * Radius * Radius * Thickness; // cm^3
    float BuoyancyMagnitude = WaterDensity * g * VolumeApprox * SubmergedRatio;
    FVector BuoyancyForce(0.f, 0.f, BuoyancyMagnitude);

    FVector TotalForce = GravityForce + DragForce + BuoyancyForce;
    FVector Accel = TotalForce / Mass;

    Velocity += Accel * DeltaTime;

    SpinRate *= 0.95f;

    if (Velocity.Size() < EndSpeedThreshold)
    {
        OnStoneFinished.Broadcast(this);
    }
}

void ASkippingStone::UpdateTilt(float DeltaTime)
{
    // 수면에 닿아 있는 상태에서만 보정
    if (State != EStoneState::Bouncing && State != EStoneState::Glide)
        return;

    // 목표: 약간 nose-up (예: 5도), 롤은 0으로 서서히 복원
    const float TargetPitch = 5.f;
    const float TargetRoll = 0.f;

    const float PitchStiffness = 20.f; // 클수록 빠르게 따라감
    const float RollStiffness = 18.f;

    TiltPitch = FMath::FInterpTo(TiltPitch, TargetPitch, DeltaTime, PitchStiffness);
    TiltRoll = FMath::FInterpTo(TiltRoll, TargetRoll, DeltaTime, RollStiffness);

    FRotator NewRot = StoneMeshComp->GetRelativeRotation();
    NewRot.Pitch = TiltPitch;
    NewRot.Roll = TiltRoll;
    StoneMeshComp->SetRelativeRotation(NewRot);

    SpinAxis = StoneMeshComp->GetUpVector().GetSafeNormal();
}

FVector ASkippingStone::ComputeDragForce(EFluidType Fluid) const
{
	float DragCoeff = (Fluid == EFluidType::Air) ? DragCoeffAir : DragCoeffWater;

	float Speed = Velocity.Size();

    if (Speed < KINDA_SMALL_NUMBER)
    {
        return FVector::ZeroVector;
    }

	FVector Dir = Velocity / Speed;
	float Area = GetArea();

	// Fd = 0.5 * rho * Cd * A * v^2
	float Magnitude = 0.5f * AirDensity * DragCoeff * Area * Speed * Speed;

    if (!FMath::IsFinite(Magnitude))
    {
        return FVector::ZeroVector;
    }

	return -Magnitude * Dir;
}

FVector ASkippingStone::ComputeMagnusLiftForce() const
{
    float Speed = Velocity.Size();

    if (Speed < KINDA_SMALL_NUMBER)
    {
        return FVector::ZeroVector;
    }

    if (FMath::IsNearlyZero(SpinRate) || SpinAxis.IsNearlyZero())
    {
        return FVector::ZeroVector;
    }

    // 스핀 축 (지금은 UpVector 고정인데, 나중에 Tilt 반영 가능)
    FVector Omega = SpinAxis * SpinRate;  // "회전 벡터"

    // Magnus: FL ∝ ω × v
    FVector LiftDir = Omega ^ Velocity; // Cross product

    if (LiftDir.IsNearlyZero())
    {
        return FVector::ZeroVector;
    }

    if (!LiftDir.Normalize())
    {
        return FVector::ZeroVector;
    }

    if (LiftDir.IsNearlyZero())
        return FVector::ZeroVector;

    LiftDir.Normalize();

    float Area = GetArea();
    float Magnitude = 0.5f * AirDensity * LiftCoeffMagnus * Area * Speed * Speed;

    if (!FMath::IsFinite(Magnitude))
    {
        return FVector::ZeroVector;
    }

    return LiftDir * Magnitude;
}

float ASkippingStone::ComputeIncidenceAngle() const
{
    FVector V = Velocity.GetSafeNormal();

    float sinTheta = FMath::Abs(V.Z);
    return FMath::RadiansToDegrees(FMath::Asin(sinTheta));
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
    float Speed = Velocity.Size();

    // "임계 각도"를 속도/질량에 따라 조정 가능 (예: 빠를수록 조금 더 커도 튄다)
    float DynamicCriticalAngle = CriticalBounceAngle + 5.f * FMath::Loge(Speed / 500.f + 1.f);

    const float MinSpeed = 500.f;

    float PitchPenalty = 0.f;
    float RollPenalty = 0.f;

    // 예시: Pitch가 클수록(앞이 많이 들렸거나 숙여졌거나) 조금 페널티
    PitchPenalty = FMath::Abs(TiltPitch) * 0.3f;

    // Roll은 좌우 절대값만 사용 (기울수록 안 튐)
    RollPenalty = FMath::Abs(TiltRoll) * 0.5f;

    float EffectiveAngle = Angle + PitchPenalty + RollPenalty;

    bool bBounce =
        EffectiveAngle < DynamicCriticalAngle &&
        Speed > MinSpeed &&
        SpinRate > 5.f &&
        Velocity.Z > -1000.f;

    UE_LOG(LogTemp, Log,
        TEXT("[ShouldBounce] %s | Angle=%.2f (Eff=%.2f) | Speed=%.2f | Spin=%.2f | Vz=%.2f | Pitch=%.2f | Roll=%.2f"),
        bBounce ? TEXT("TRUE") : TEXT("FALSE"),
        Angle, EffectiveAngle, Speed, SpinRate, Velocity.Z, TiltPitch, TiltRoll);

    return bBounce;
}

#pragma endregion

float ASkippingStone::GetArea() const
{
	return PI * Radius * Radius;    
}

#pragma region Water Contact

void ASkippingStone::HandleWaterContact(AWaterSurface* Water, const FVector& HitPoint)
{
    LastWater = Water;
    ContactPoint = HitPoint;

    if (State == EStoneState::Airborne)
    {
        SetStoneState(EStoneState::WaterContact);
    }
}

#pragma endregion
