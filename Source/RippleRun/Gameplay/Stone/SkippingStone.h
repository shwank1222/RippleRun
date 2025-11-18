#pragma once

#include "GameFramework/Actor.h"
#include "SkippingStone.generated.h"

class UProjectileMovementComponent;
class UArrowComponent;
UCLASS()
class RIPPLERUN_API ASkippingStone : public AActor
{
	GENERATED_BODY()
	
public:	
	ASkippingStone();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Appearance")
	UStaticMeshComponent* StoneMeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	UProjectileMovementComponent* ProjectileMovementComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	UArrowComponent* ArrowComp;

public:	
	virtual void Tick(float DeltaTime) override;

};
