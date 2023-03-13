#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "VRShooter/Interfaces/BulletHitInterface.h"
#include "Enemy.generated.h"

UCLASS()
class VRSHOOTER_API AEnemy : public ACharacter, public IBulletHitInterface
{
	GENERATED_BODY()

public:
	AEnemy();

	void BreakingBones(FVector Impulse, FVector HitLocation, FName Bone);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void AddHitReactImpulse(FVector Impulse, FVector HitLocation, FName Bone, bool bAddImpulse);

	UFUNCTION(BlueprintCallable)
	void SetStunned(bool Stunned);

private:
	// OverlapShpere for when the enemy becomes hostile
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Initialization", meta = (AllowPrivateAccess = "true"))
	class USphereComponent* AgroSphere;

	// Sphere for CombatRange
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Initialization", meta = (AllowPrivateAccess = "true"))
	class USphereComponent* CombatRangeSphere;

	// HealthBar
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Initialization", meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* HealthBar;

	void RotateWidgetToPlayer(UWidgetComponent* Widget, FVector PlayerLocation);
	bool bShouldRotateTheHealthBar = false;
	class AVRShooterCharacter* VRShooterCharacter;

protected:
	virtual void BeginPlay() override;

	void Die();
	void PlayHitMontage(FName Section, float PlayRate = 1.0f);
	void ResetHitReactTimer();
	void ResetCanAttack();

	UFUNCTION(BlueprintCallable)
	void FinishDeath();

	UFUNCTION()
	void DestroyEnemy();

	UFUNCTION(BlueprintCallable)
	void ToggleRotateToPlayer(bool bRotate);

	void RotateToPlayer(float DeltaTime);

	UFUNCTION(BlueprintCallable)
	void PlayAttackMontage(FName Section, float PlayRate = 1.0f);

	UFUNCTION(BlueprintPure)
	FName GetAttackSectionName();

	void DoDamage(class AVRShooterCharacter* Victim);
	void SpawnBloodParticles(AVRShooterCharacter* Victim, FName SocketName);

private:
	void ShowHealthBar();
	void HideHealthBar();
	
	// Called when something overlaps with the agro sphere
	UFUNCTION()
	void OnAgroSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bfromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void OnCombatRangeSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bfromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void OnCombatRangeSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

	UFUNCTION()
	void OnLeftWeaponOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bfromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void OnRightWeaponOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bfromSweep,
		const FHitResult& SweepResult
	);

	// Activate/Deactivate collision for weapon boxes
	UFUNCTION(BlueprintCallable)
	void ActivateLeftWeapon();

	UFUNCTION(BlueprintCallable)
	void DeactivateLeftWeapon();

	UFUNCTION(BlueprintCallable)
	void ActivateRightWeapon();

	UFUNCTION(BlueprintCallable)
	void DeactivateRightWeapon();

	// Base damage for enemy
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float BaseDamage = 20.f;

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

	// Montage containing different Attacks
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* AttackMontage;

	// Montage section names
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	FName AttackLFast = TEXT("AttackLFast");
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	FName AttackRFast = TEXT("AttackRFast");
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	FName AttackRSword = TEXT("AttackRSword");
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	FName AttackLSword = TEXT("AttackLSword");

	// Collision volume for the left weapon
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* LeftWeaponCollision;

	// Collision volume for the right weapon
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UBoxComponent* RightWeaponCollision;

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

	// Behavior tree for the AI Character
	UPROPERTY(EditAnywhere, Category = Initialization, meta = (AllowPrivateAccess = "True"))
	class UBehaviorTree* BehaviorTree;

	// Point for the enemy to move to
	UPROPERTY(EditAnywhere, Category = Initialization, meta = (AllowPrivateAccess = "True", MakeEditWidget = "true"))
	FVector PatrolPoint;

	// Second Point for the enemy to move to
	UPROPERTY(EditAnywhere, Category = Initialization, meta = (AllowPrivateAccess = "True", MakeEditWidget = "true"))
	FVector PatrolPoint2;

	class AEnemyController* EnemyController;

	// True when playing the get hit animation
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "True"))
	bool bStunned = false;

	// Chance of being stunned. 0: no stun chance, 1: 100% stun chance 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Initialization, meta = (AllowPrivateAccess = "True"))
	float StunChance = .5f;

	// True when in attack range -> Time to Attack!!!
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "True"))
	bool bInAttackRange = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* BloodParticles;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UNiagaraSystem* BodyBloodNiagara;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UNiagaraSystem* HeadBloodNiagara;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UNiagaraSystem* DeathNiagara;

	UNiagaraSystem* BloodNiagara;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	FName LeftWeaponSocket = TEXT("LeftWeaponBone");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	FName RightWeaponSocket = TEXT("RightWeaponBone");

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "True"))
	bool bCanAttack = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float RotationSpeed = 10.f;

	bool bShouldRotateToPlayer = false;

	FTimerHandle AttackWaitTimer;

	// Minimum wait time between attacks
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float AttackWaitTime = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* DeathMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bRagdollOnDeath = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UCameraShakeBase> DeathCameraShake;

	bool bDying = false;

	FTimerHandle DeathTimer;
	// Time after death until Destroy
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float DeathTime = 2.0f;

	void UpdatePlayerKillCounter(APawn* Shooter);
	bool bKillCounterUpdated = false;

	// Ragdoll
	void RagdollStart();
	void RagdollEnd();

	// SlowMotion
	FTimerHandle SlowMotionResetTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float SlowMotionResetTime = .25f;

	void StartSlowMotion();

	UFUNCTION()
	void StopSlowMotion();

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void BulletHit_Implementation(FHitResult HitResult, AActor * Shooter, AController* ShooterController) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	
	void ShowHitNumber(class AVRShooterCharacter* Causer, int32 Damage, FVector HitLocation, bool bHeadShot);
	void SwitchBloodParticles(bool bIsHeadshot);

	FORCEINLINE FString GetHeadBone() const { return HeadBone; }
	FORCEINLINE UBehaviorTree* GetBehaviorTree() const { return BehaviorTree; }
	FORCEINLINE UParticleSystem* GetBloodParticles() const { return BloodParticles; }
};
