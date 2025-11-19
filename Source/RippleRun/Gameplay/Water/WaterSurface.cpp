#include "Gameplay/Water/WaterSurface.h"
#include "Components/BoxComponent.h"

AWaterSurface::AWaterSurface()
{
	PrimaryActorTick.bCanEverTick = false;

    if (!RootComponent)
    {
        RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
    }

    if(!WaterMesh)
    {
        WaterMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WaterMesh"));
        WaterMesh->SetupAttachment(RootComponent);
        WaterMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        WaterMesh->SetCastShadow(false);
	}

    if(!WaterCollision)
    {
        WaterCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("WaterCollision"));
        WaterCollision->SetupAttachment(RootComponent);
        WaterCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        WaterCollision->SetCollisionObjectType(ECC_WorldStatic);
        WaterCollision->SetCollisionResponseToAllChannels(ECR_Overlap);
	}
}

void AWaterSurface::GenerateWave(const FVector& HitLocation, float ImpactStrength)
{
}

void AWaterSurface::BeginPlay()
{
	Super::BeginPlay();
	
}

//void AWaterSurface::Tick(float DeltaTime)
//{
//	Super::Tick(DeltaTime);
//
//}

