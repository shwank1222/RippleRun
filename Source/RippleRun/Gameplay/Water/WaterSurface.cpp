#include "WaterSurface.h"
#include "Gameplay/Stone/SkippingStone.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Materials/MaterialInstanceDynamic.h"

AWaterSurface::AWaterSurface()
{
    PrimaryActorTick.bCanEverTick = false;

    WaterMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WaterMesh"));
    RootComponent = WaterMesh;

    WaterMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    WaterMesh->SetCollisionResponseToAllChannels(ECR_Overlap);
	WaterMesh->SetGenerateOverlapEvents(true);
    WaterMesh->SetCastShadow(false);

    WaterMesh->OnComponentBeginOverlap.AddDynamic(this, &AWaterSurface::HandleStoneOverlap);

    KillZone = CreateDefaultSubobject<UBoxComponent>(TEXT("KillZone"));
    KillZone->SetupAttachment(WaterMesh);

	//Set Z to be below water surface
	KillZone->SetRelativeLocation(FVector(0.f, 0.f, -500.f));

    KillZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    KillZone->SetCollisionResponseToAllChannels(ECR_Overlap);
    KillZone->SetGenerateOverlapEvents(true);
    KillZone->OnComponentBeginOverlap.AddDynamic(this, &AWaterSurface::HandleKillZoneOverlap);
}

void AWaterSurface::BeginPlay()
{

}

void AWaterSurface::HandleStoneOverlap(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult
)
{
    ASkippingStone* Stone = Cast<ASkippingStone>(OtherActor);
    if (Stone)
    {
		UE_LOG(LogTemp, Log, TEXT("Water Surface: Stone Overlapped"));
        Stone->HandleWaterContact(this, SweepResult.ImpactPoint);

        const FVector Hit = Stone->GetContactPoint();

        if (SplashEffect)
        {
            UGameplayStatics::SpawnEmitterAtLocation(
                GetWorld(),
                SplashEffect,
                Hit
            );
        }

        if (SplashSound)
        {
			float BounceCount = Stone->GetBounceCount();
            float Strength = WaveStrength * FMath::Pow(0.8f, BounceCount);

            UGameplayStatics::PlaySoundAtLocation(
                GetWorld(),
                SplashSound,
                Hit,
                Strength
            );


        }

    }
}

void AWaterSurface::SetOverlapEnabled(bool bEnabled)
{
	WaterMesh->SetGenerateOverlapEvents(bEnabled);
	KillZone->SetGenerateOverlapEvents(bEnabled);
}

void AWaterSurface::GenerateWave(const FVector& HitLocation, float ImpactStrength)
{

}

void AWaterSurface::HandleKillZoneOverlap(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult
)
{
    if (ASkippingStone* Stone = Cast<ASkippingStone>(OtherActor))
    {
        UE_LOG(LogTemp, Warning, TEXT("KillZone: Stone forced end"));
        Stone->ForceEndSkipping(); // �� �Լ� �ϳ� �����
    }
}
