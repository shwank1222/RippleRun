#include "Gameplay/Stone/SkippingStone.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/ArrowComponent.h"

ASkippingStone::ASkippingStone()
{
	PrimaryActorTick.bCanEverTick = true;

    if (!RootComponent)
    {
        RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("ProjectileSceneComponent"));
    }

    if (!StoneMeshComp)
    {
        StoneMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StoneMeshComp"));
        StoneMeshComp->SetupAttachment(RootComponent);
        StoneMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        StoneMeshComp->SetCollisionObjectType(ECC_PhysicsBody);
        StoneMeshComp->SetCollisionResponseToAllChannels(ECR_Block);
    }

    if (!ProjectileMovementComp)
    {   
        ProjectileMovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
        ProjectileMovementComp->SetUpdatedComponent(StoneMeshComp);

        ProjectileMovementComp->bRotationFollowsVelocity = true;

        ProjectileMovementComp->bShouldBounce = true;
        ProjectileMovementComp->Bounciness = Bounciness;
        ProjectileMovementComp->ProjectileGravityScale = GravityScale;

        ProjectileMovementComp->bAutoActivate = false;
    }

    if(!ArrowComp)
    {
        ArrowComp = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComp"));
        ArrowComp->SetupAttachment(RootComponent);
        ArrowComp->SetHiddenInGame(true);
	}
    
}

void ASkippingStone::BeginPlay()
{
	Super::BeginPlay();

    ProjectileMovementComp->InitialSpeed = InitialSpeed;
    ProjectileMovementComp->MaxSpeed = InitialSpeed * 1.5f;
}

void ASkippingStone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASkippingStone::SetRadius(float NewRadius)
{
	Radius = NewRadius;
	// Update the scale of the stone mesh based on new radius
}

void ASkippingStone::SetThickness(float NewThickness)
{
	Thickness = NewThickness;
	// Update the scale of the stone mesh based on new thickness
}

void ASkippingStone::ThrowStone()
{
    ProjectileMovementComp->Activate(true);
	UE_LOG(LogTemp, Warning, TEXT("Stone Thrown with Initial Speed: %f"), ProjectileMovementComp->InitialSpeed);
}

