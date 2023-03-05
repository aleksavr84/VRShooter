#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BulletHitInterface.h"
#include "Enemy.generated.h"

UCLASS()
class VRSHOOTER_API AEnemy : public ACharacter, public IBulletHitInterface
{
	GENERATED_BODY()

public:
	AEnemy();

private:
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	//class USphereComponent* AreaSphere;

	// HealthBar
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* HealthBar;

	void RotateWidgetToPlayer(UWidgetComponent* Widget, FVector PlayerLocation);
	bool bShouldRotateTheHealthBar = false;
	class AVRShooterCharacter* VRShooterCharacter;

protected:
	virtual void BeginPlay() override;

	void Die();
	void PlayHitMontage(FName Section, float PlayRate = 1.0f);
	void ResetHitReactTimer();

private:
	void ShowHealthBar();
	void HideHealthBar();
	
	/*UFUNCTION()
	void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bfromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);*/

	// Particles to spawn when hit by bullets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UParticleSystem* ImpactParticles;

	// Sound to play when hit by bullets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class USoundCue* ImpactSound;

	// Current health of the enemy
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float Health = 100.f;

	// Maximum health of the enemy
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float MaxHealth = 100.f;

	// Name of the head bone
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	FString HeadBone = "head";

	FTimerHandle HealthBarTimer;

	// Time to display healthBar once shot
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float HealthBarDisplayTime = 4.f;

	// Montage containing Hit and Death animation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* HitMontage;

	FTimerHandle HitReactTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float HitReactTimeMin = 0.5f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float HitReactTimeMax = .75f;
	
	bool bCanHitReact = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Initialization, meta = (AllowPrivateAccess = "True"))
	TSubclassOf<class AWidgetActor> WidgetActorClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Initialization, meta = (AllowPrivateAccess = "True"))
	TMap<AWidgetActor*, FVector> HitNumberActors;

	// Time before a HitNumber is removed from the screen
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Initialization, meta = (AllowPrivateAccess = "True"))
	float HitNumberDestroyTime = 1.5f;
;
	void StoreHitNumber(AWidgetActor* HitNumber, FVector Location);

	UFUNCTION()
	void DestroyHitNumber(AWidgetActor* HitNumber);

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void BulletHit_Implementation(FHitResult HitResult, AVRShooterCharacter* CauserCharacter) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	
	void ShowHitNumber(class AVRShooterCharacter* Causer, int32 Damage, FVector HitLocation, bool bHeadShot);

	FORCEINLINE FString GetHeadBone() const { return HeadBone; }
};
