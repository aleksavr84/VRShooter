#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BulletHitInterface.h"
#include "Explosive.generated.h"

UCLASS()
class VRSHOOTER_API AExplosive : public AActor, public IBulletHitInterface
{
	GENERATED_BODY()
	
public:	
	AExplosive();

protected:
	virtual void BeginPlay() override;

private:
	// Explosion when hit by bullets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UParticleSystem* ExplodeParticles;

	// Sound to play when hit by bullets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class USoundCue* ImpactSound;

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void BulletHit_Implementation(FHitResult HitResult, class AVRShooterCharacter* CauserCharacter) override;

};
