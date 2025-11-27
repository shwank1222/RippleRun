#include "WaterSurface.h"
#include "Gameplay/Stone/SkippingStone.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"

AWaterSurface::AWaterSurface()
{
    PrimaryActorTick.bCanEverTick = false;

    // === WaterMesh를 Root로 설정 ===
    WaterMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WaterMesh"));
    RootComponent = WaterMesh;

    WaterMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    WaterMesh->SetCastShadow(false);

    // === Collision은 Mesh의 자식으로 배치 ===
    WaterCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("WaterCollision"));
    WaterCollision->SetupAttachment(WaterMesh);
    WaterCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    WaterCollision->SetCollisionObjectType(ECC_WorldStatic);
    WaterCollision->SetCollisionResponseToAllChannels(ECR_Overlap);

    // === 오버랩 이벤트 ===
    WaterCollision->OnComponentBeginOverlap.AddDynamic(
        this, &AWaterSurface::HandleStoneOverlap
    );
}

void AWaterSurface::BeginPlay()
{
    Super::BeginPlay();
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

        if (SplashEffect)
        {
            UGameplayStatics::SpawnEmitterAtLocation(
                GetWorld(),
                SplashEffect,
                SweepResult.ImpactPoint,
                FRotator::ZeroRotator
            );
        }
    }
}

void AWaterSurface::GenerateWave(const FVector& HitLocation, float ImpactStrength)
{
    // TODO: 나중에 실제 물결 시뮬레이션을 구현할 예정
}
