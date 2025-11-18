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
        ProjectileMovementComp->InitialSpeed = 3000.0f;
        ProjectileMovementComp->MaxSpeed = 3000.0f;
        ProjectileMovementComp->bRotationFollowsVelocity = true;
        ProjectileMovementComp->bShouldBounce = true;
        ProjectileMovementComp->Bounciness = 0.3f;
        ProjectileMovementComp->ProjectileGravityScale = 0.0f;
    }

    if(!ArrowComp)
    {
        ArrowComp = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComp"));
        ArrowComp->SetupAttachment(RootComponent);
        ArrowComp->SetHiddenInGame(true);
	}
    
}

// Called when the game starts or when spawned
void ASkippingStone::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASkippingStone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

