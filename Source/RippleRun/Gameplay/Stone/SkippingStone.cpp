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
    StoneMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

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
    float NewRadius = FMath::FRandRange(0.03f, 0.08f);
    float NewThickness = FMath::FRandRange(0.01f, 0.035f);

    SetRadius(NewRadius);
    SetThickness(NewThickness);

    float Volume = PI * Radius * Radius * Thickness;
    float NewMass = Density * Volume;

    SetMass(NewMass);
}

void ASkippingStone::SetRadius(float NewRadius)
{
    Radius = NewRadius;

    FVector Scale = StoneMeshComp->GetRelativeScale3D();

    float RadiusScale = Radius / BaseRadius * 100.f; // Convert m to cm

    Scale.X = RadiusScale;
    Scale.Y = RadiusScale;

    StoneMeshComp->SetRelativeScale3D(Scale);
}

void ASkippingStone::SetThickness(float NewThickness)
{
    Thickness = NewThickness;

    FVector Scale = StoneMeshComp->GetRelativeScale3D();

	float ThicknessScale = Thickness / BaseThickness * 100.f; // Convert m to cm

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

    FRotator CurrentRotation = StoneMeshComp->GetRelativeRotation();
    TiltPitch = CurrentRotation.Pitch;
    TiltRoll = CurrentRotation.Roll;

    // Throwing direction
    FRotator ThrowRot = GetActorRotation();
    ThrowRot.Pitch += ThrowAngle;
    FVector Direction = ThrowRot.Vector().GetSafeNormal();

    float Accel = ThrowForce / Mass *100.f;
    float ImpulseTime = 0.01f;

    Velocity = Direction * (Accel * ImpulseTime);

    SpinAxis = StoneMeshComp->GetUpVector().GetSafeNormal();

    SetStoneState(EStoneState::Airborne);
    SetActorTickEnabled(true);

    bHasThrown = true;

    UE_LOG(LogTemp, Log,
        TEXT("Stone Thrown | Force=%.1f | Mass=%.3f | InitV=%.1f cm/s"),
        ThrowForce, Mass, Velocity.Size());
}


void ASkippingStone::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bHasThrown) return;

    if (GetWorld()->IsPlayingReplay())
    {
        return;
    }

    PreviousLocation = GetActorLocation();
    
    TimeElapsed += DeltaTime;

    ForwardDistance =
        FVector::DotProduct(GetActorLocation() - ThrowStartLocation, ThrowDirection)/100;

    ApplyPhysics(DeltaTime);

    // Update position
    SetActorLocation(GetActorLocation() + Velocity * DeltaTime);

    // Simple yaw spin
    AddActorLocalRotation(FRotator(0.f, 100*SpinRate * DeltaTime, 0.f));
    
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
        SetStoneState(EStoneState::Sunk);
        return;
    }

    if (Velocity.Z < 0.f&& LastWater!=nullptr) 
    {
        float WaterZ = LastWater ? LastWater->GetActorLocation().Z : ContactPoint.Z;

        if (GetActorLocation().Z <= WaterZ + 5.f)
        {
            SetStoneState(EStoneState::Sunk);
            FinalDistance = ForwardDistance;
            return;
        }
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

    // v' = v - (1+e)(v��n)n
    float Vn = FVector::DotProduct(Velocity, Normal);
    float Restitution = EnergyRetention; // 0~1

    FVector Reflected = Velocity - (1.f + Restitution) * Vn * Normal;

    const float TangentialDamping = 0.9f;
    const FVector VnVec = FVector::DotProduct(Reflected, Normal) * Normal;
    const FVector VtVec = Reflected - VnVec;

    Reflected = VtVec * TangentialDamping + VnVec;

    // Magnus Lift 
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

    float CurveAngle = TiltRoll * CurveStrength;
    Velocity = Velocity.RotateAngleAxis(CurveAngle, FVector::UpVector);

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

    FVector WaterDragForce = ComputeDragForce(EFluidType::Water); // ���� �巡�׿� ����� �� + ����� ũ��

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

    // ���� ���� �ӹ������� �ʹ� �������� Yaw/�� ���� �ٲ㼭 ����ȭ��Ű�� �͵� ���� (����)

    if (!IsFiniteVector(Velocity))
    {
        UE_LOG(LogTemp, Error, TEXT("Velocity became non-finite in Glide! V=%s"), *Velocity.ToString());
        Velocity = FVector::ZeroVector;
        SetStoneState(EStoneState::Sunk);
        return;
    }

    // ������ ������ ����
    SpinRate -= SpinRate * (AngularDrag * 0.5f) * DeltaTime;

    UpdateTilt(DeltaTime);

    if (Velocity.Size() < MinGlideSpeed || abs(SpinRate) < MinGlideSpin)
    {
        SetStoneState(EStoneState::Sunk);
        FinalDistance = ForwardDistance;
        return;
    }

    if (Velocity.Z > GlideMinVz && GetActorLocation().Z >= ContactPoint.Z + GlideThreshold)
    {
        SetStoneState(EStoneState::Airborne);
        return;
    }
}

void ASkippingStone::TickSunk(float DeltaTime)
{
    const float g = 980.f;

    FVector GravityForce = FVector(0.f, 0.f, -Mass * g);
    FVector DragForce = ComputeDragForce(EFluidType::Water);

    // ���� ������ �η� �ٻ�: ���� ������ ���ٰ� ���� ������ ����
    float SubmergedRatio = 1.0f; // ���߿� ����/�β��� ��� ����
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
    // ���鿡 ��� �ִ� ���¿����� ����
    if (State != EStoneState::Bouncing && State != EStoneState::Glide)
        return;

    // ��ǥ: �ణ nose-up (��: 5��), ���� 0���� ������ ����
    const float TargetPitch = 5.f;
    const float TargetRoll = 0.f;

    const float PitchStiffness = 20.f; // Ŭ���� ������ ����
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

    // ���� �� (������ UpVector �����ε�, ���߿� Tilt �ݿ� ����)
    FVector Omega = SpinAxis * SpinRate;  // "ȸ�� ����"

    // Magnus: FL �� �� �� v
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

    // "�Ӱ� ����"�� �ӵ�/������ ���� ���� ���� (��: �������� ���� �� Ŀ�� Ƥ��)
    float DynamicCriticalAngle = CriticalBounceAngle + 5.f * FMath::Loge(Speed / 500.f + 1.f);

    float PitchPenalty = 0.f;
    float RollPenalty = 0.f;

    // ����: Pitch�� Ŭ����(���� ���� ��Ȱų� �������ų�) ���� ���Ƽ
    PitchPenalty = FMath::Abs(TiltPitch) * 0.3f;

    // Roll�� �¿� ���밪�� ��� (������ �� Ʀ)
    RollPenalty = FMath::Abs(TiltRoll) * 0.5f;

    float EffectiveAngle = Angle + PitchPenalty + RollPenalty;

    bool bBounce =
        EffectiveAngle < DynamicCriticalAngle &&
        Speed > MinBounceSpeed &&
        abs(SpinRate) > MinBounceSpin &&
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

    FVector Prev = PreviousLocation;
    FVector Curr = GetActorLocation();

    const float WaterZ = Water->GetActorLocation().Z;

    float Alpha = (WaterZ - Prev.Z) / (Curr.Z - Prev.Z);
    Alpha = FMath::Clamp(Alpha, 0.f, 1.f);

    FVector Hit = Prev + (Curr - Prev) * Alpha;
    Hit.Z = WaterZ;

    Water->GenerateWave(Hit, SpinRate);

    ContactPoint = Hit;

	if (State == EStoneState::Airborne)
    {
        SetStoneState(EStoneState::WaterContact);

    }
}

void ASkippingStone::ForceEndSkipping()
{
	Velocity = FVector::ZeroVector;
	SetActorTickEnabled(false);

    FinalDistance = ForwardDistance;

    OnStoneFinished.Broadcast(this);
}

#pragma endregion
