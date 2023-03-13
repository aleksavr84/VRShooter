#include "CombatComponent.h"
#include "VRShooter/Character/VRShooterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Sound/SoundCue.h"
#include "Engine/SkeletalMeshSocket.h"
#include "DrawDebugHelpers.h"
#include "Particles/ParticleSystemComponent.h"
#include "VRShooter/Weapon/Weapon.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Camera/CameraComponent.h"
#include "VRShooter/Pickups/Ammo.h"
#include "VRShooter/Interfaces/BulletHitInterface.h"
#include "VRShooter/Enemy/Enemy.h"
#include "TimerManager.h"
#include "VRShooter/Actors/WidgetActor.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "VRShooter/Weapon/Types.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	HandSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("HandSceneComponent"));
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	SpawnKillCounterWidget();
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (Character->GetIsFightingForLife())
	{
		FightForYourLifeTimeRemaining = Character->GetWorldTimerManager().GetTimerRemaining(FightForYourLifeTimer);
		UE_LOG(LogTemp, Error, TEXT("%f"), FightForYourLifeTimeRemaining);
		UE_LOG(LogTemp, Error, TEXT("%f"), (FightForYourLifeTimeRemaining / FightForYourLifeTime) );
	}
}

void UCombatComponent::SpawnKillCounterWidget()
{
	if (KillCounterWidgetActorClass &&
		Character &&
		Character->GetCameraComponent())
	{
		//// Spawning KillCounterWidget
		FActorSpawnParameters SpawnParam;
		SpawnParam.Instigator = Character;
		KillCounterWidgetActor = Character->GetWorld()->SpawnActor<AWidgetActor>(KillCounterWidgetActorClass, SpawnParam);
		KillCounterWidgetActor->AttachToComponent(Character->GetCameraComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		KillCounterWidgetActor->SetOwner(Character);
		KillCounterWidgetActor->SetActorHiddenInGame(true);
		KillCounterWidgetActor->AddActorWorldOffset(KillCounterLocationOffset);
	}
}

AWeapon* UCombatComponent::SpawnDefaultWeapon()
{
	// Check the TSubclassOf variable
	if (DefaultWeaponClass)
	{
		bIsEquipped = true;
		// Spawn the Weapon
		return GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass);
	}

	bIsEquipped = false;

	return nullptr;
}

void UCombatComponent::EquipWeapon(class AWeapon* WeaponToEquip, bool bSwapping)
{
	if (WeaponToEquip)
	{
		// Get the HandMesh and the HandSocket
		USkeletalMeshComponent* HandMesh = Character->GetBodyMesh(); //Character->GetRightHandController()->GetHandMesh();
		const USkeletalMeshSocket* HandSocket = HandMesh->GetSocketByName(WeaponToEquip->GetHandSocketName());

		if (HandSocket)
		{
			// Attach the Weapon to the HandSocket
			HandSocket->AttachActor(WeaponToEquip, HandMesh);

			if (EquippedWeapon == nullptr)
			{
				// -1 == no EquippedWeapon yet. No need to reverse the icon Animation
				EquipItemDelegate.Broadcast(-1, WeaponToEquip->GetSlotIndex());
			}
			else if (!bSwapping)
			{
				EquipItemDelegate.Broadcast(EquippedWeapon->GetSlotIndex(), WeaponToEquip->GetSlotIndex());
			}

			EquippedWeapon = WeaponToEquip;
			//EquippedWeapon->GetItemMesh()->SetWorldRotation(EquippedWeapon->GetWeaponRotation());
			EquippedWeapon->SetItemState(EItemState::EIS_Equipped);

			bIsEquipped = true;

			Character->ShowWeaponHUD();
		}
	}
}

void UCombatComponent::DropWeapon()
{
	if (EquippedWeapon)
	{
		FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld, true);
		EquippedWeapon->GetItemMesh()->DetachFromComponent(DetachmentTransformRules);

		EquippedWeapon->SetItemState(EItemState::EIS_Falling);
		EquippedWeapon->ThrowWeapon();

		bIsEquipped = false;
	}
}

void UCombatComponent::SwapWeapon(AWeapon* WeaponToSwap)
{
	if (WeaponToSwap)
	{
		if (Inventory.Num() - 1 >= EquippedWeapon->GetSlotIndex())
		{
			Inventory[EquippedWeapon->GetSlotIndex()] = WeaponToSwap;
			WeaponToSwap->SetSlotIndex(EquippedWeapon->GetSlotIndex());
		}

		DropWeapon();
		EquipWeapon(WeaponToSwap, true);
		TraceHitItem = nullptr;
		TraceHitItemLastFrame = nullptr;
	}
}

void UCombatComponent::ExchangeInventoryItems(int32 CurrentItemIndex, int32 NewItemIndex)
{
	const bool bCanExchangeItems = 
		(CurrentItemIndex != NewItemIndex) &&
		(NewItemIndex < Inventory.Num()) &&
		(CombatState == ECombatState::ECS_Unoccupied || 
			CombatState == ECombatState::ECS_Equipping);

	if (bCanExchangeItems)
	{
		auto OldEquippedWeapon = EquippedWeapon;
		auto NewWeapon = Cast<AWeapon>(Inventory[NewItemIndex]);

		EquipWeapon(NewWeapon);

		OldEquippedWeapon->SetItemState(EItemState::EIS_PickedUp);
		NewWeapon->SetItemState(EItemState::EIS_Equipped);

		CombatState = ECombatState::ECS_Equipping;

		if (Character)
		{
			UAnimInstance* AnimInstance = Character->GetBodyMesh()->GetAnimInstance();

			if (AnimInstance &&
				EquipMontage)
			{
				AnimInstance->Montage_Play(EquipMontage, 1.0f);
				AnimInstance->Montage_JumpToSection(FName("Equip"));
			}
			else
			{
				FinishEquipping();
			}

			NewWeapon->PlayEquipSound(true);
		}
	}
}

void UCombatComponent::HighlightInventorySlot()
{
	const int32 EmptySlot{ GetEmptyInventorySlot() };
	HighlightIconDelegate.Broadcast(EmptySlot, true);
	HighlightedSlot = EmptySlot;
}

void UCombatComponent::UnHighlightInventorySlot()
{
	HighlightIconDelegate.Broadcast(HighlightedSlot, false);
	HighlightedSlot = -1;
}

int32 UCombatComponent::GetEmptyInventorySlot()
{
	for (int32 i = 0; i < Inventory.Num(); i++)
	{
		if (Inventory[i] == nullptr)
		{
			return i;
		}
	}

	if (Inventory.Num() < INVENTORY_CAPACITY)
	{
		return Inventory.Num();
	}

	return -1; // Inventory is full!
}

void UCombatComponent::FinishEquipping()
{
	CombatState = ECombatState::ECS_Unoccupied;
}

void UCombatComponent::InitializeAmmoMap()
{
	AmmoMap.Add(EAmmoType::EAT_9mm, Starting9mmAmmo);
	AmmoMap.Add(EAmmoType::EAT_AR, StartingARAmmo);
	AmmoMap.Add(EAmmoType::EAT_Grenade, StartingGrenadeAmmo);
}

void UCombatComponent::PickupAmmo(AAmmo* Ammo)
{
	
	if (EquippedWeapon)
	{
		// Check to see if AmmoMap contains Ammo's AmmoType
		if (AmmoMap.Find(Ammo->GetAmmoType()))
		{
			// Get amount of ammo in our AmmoMap for Ammo's type
			int32 AmmoCount{ AmmoMap[Ammo->GetAmmoType()] };
			AmmoCount += Ammo->GetItemCount();
			// Set the amount of ammo in the Map for this type
			AmmoMap[Ammo->GetAmmoType()] = AmmoCount;
		}

		if (EquippedWeapon->GetAmmoType() == Ammo->GetAmmoType())
		{
			// Check to see if the gun is empty
			if (EquippedWeapon->GetAmmo() == 0)
			{
				ReloadWeapon();
			}
		}

		Ammo->Destroy();
	
	}
}

void UCombatComponent::FireButtonPressed()
{
	bFireButtonPressed = true;
	FireWeapon();
}

void UCombatComponent::FireButtonReleased()
{
	bFireButtonPressed = false;
}

void UCombatComponent::FireWeapon()
{
	if (EquippedWeapon)
	{
		if (CombatState != ECombatState::ECS_Unoccupied) return;

		if (WeaponHasAmmo())
		{
			PlayFireSound();
			SendBullet();
			PlayGunFireMontage();
			PlayHapticEffect();
			EquippedWeapon->DecrementAmmo();

			StartFireTimer();
		}
	}
}

void UCombatComponent::PlayFireSound()
{
	// Play fire sound
	if (EquippedWeapon &&
		EquippedWeapon->GetFireSound())
	{
		UGameplayStatics::PlaySound2D(this, EquippedWeapon->GetFireSound());
	}
}

void UCombatComponent::SendBullet()
{
	if (EquippedWeapon->GetWeaponType() == EWeaponType::EWT_GrenadeLauncher) return;

	StartHitMultiplierTimer();
	
	// Send bullet
	const USkeletalMeshComponent* WeaponMesh = EquippedWeapon->GetItemMesh();

	if (WeaponMesh)
	{
		const USkeletalMeshSocket* BarrelSocket = WeaponMesh->GetSocketByName(EquippedWeapon->GetMuzzleFlashSocketName());

		if (BarrelSocket)
		{
			const FTransform SocketTransform = BarrelSocket->GetSocketTransform(WeaponMesh);

			if (EquippedWeapon->GetMuzzleFlash())
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), EquippedWeapon->GetMuzzleFlash(), SocketTransform);
			}

			if (EquippedWeapon->GetMuzzleFlashNiagara())
			{
				UNiagaraComponent* MuzzleFlashN = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
					this,
					EquippedWeapon->GetMuzzleFlashNiagara(),
					SocketTransform.GetLocation(),
					EquippedWeapon->GetActorRotation()
				);
				MuzzleFlashN->SetWorldRotation(SocketTransform.Rotator());
			}

			FHitResult FireHit;
			const FVector Start{ SocketTransform.GetLocation() };
			const FQuat Rotation{ SocketTransform.GetRotation() };
			const FVector RotationAxis{ Rotation.GetAxisX() };
			const FVector End{ Start + RotationAxis * 50'000.f };
			const FVector BoneBreakImpulse{ Start + RotationAxis * 100.f };

			BeamEndPoint = End ;

			GetWorld()->LineTraceSingleByChannel(
				FireHit,
				Start,
				End,
				ECollisionChannel::ECC_Visibility
			);

			/*if (EquippedWeapon->GetFireType() == EFireType::EFT_Projectile)
			{
				EquippedWeapon->Fire(FireHit.Location);
			}*/

			if (FireHit.bBlockingHit)
			{
				/*DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 2.f);
				DrawDebugPoint(GetWorld(), FireHit.Location, 5.f, FColor::Red, false, 2.f);*/
				
				IBulletHitInterface* BulletHitInterface = Cast<IBulletHitInterface>(FireHit.GetActor());

				if (BulletHitInterface)
				{
					BulletHitInterface->BulletHit_Implementation(FireHit, Character, Character->GetController());
				}

				BeamEndPoint = FireHit.Location;

				// Does hit Actor implement BulletHitInterface?
				if (FireHit.GetActor())
				{
					AEnemy* HitEnemy = Cast<AEnemy>(FireHit.GetActor());

					if (HitEnemy)
					{
						int32 Damage{};

						if (FireHit.BoneName.ToString() == HitEnemy->GetHeadBone())
						{
							// Head shot
							Damage = EquippedWeapon->GetHeadShotDamage();
							//HitEnemy->SwitchBloodParticles(true);

							UGameplayStatics::ApplyDamage(
								FireHit.GetActor(),
								Damage,
								Character->GetController(),
								Character,
								UDamageType::StaticClass()
							);

							HitEnemy->ShowHitNumber(Character, Damage, FireHit.Location, true);
							HitEnemy->BreakingBones(BoneBreakImpulse, FireHit.Location, FireHit.BoneName);

							// Update HitCounter and ScoreMultiplier
							UpdateHitMultiplier(GetHitMultiplier() + 1);
							UpdateHitCounter(Damage);
						}
						else
						{
							// Body shot
							Damage = EquippedWeapon->GetDamage();

							UGameplayStatics::ApplyDamage(
								FireHit.GetActor(),
								Damage,
								Character->GetController(),
								Character,
								UDamageType::StaticClass()
							);

							HitEnemy->ShowHitNumber(Character, Damage, FireHit.Location, false);
							HitEnemy->BreakingBones(BoneBreakImpulse, FireHit.Location, FireHit.BoneName);
						
							// Update HitCounter and ScoreMultiplier
							UpdateHitCounter(Damage);
						}

						// Reset the timers if we make a hit
						StartHitMultiplierTimer();
						StartKillCounterTimer();
					}
				}
				else
				{
					// Spawn default particles
					if (EquippedWeapon->GetImpactParticles())
					{
						UGameplayStatics::SpawnEmitterAtLocation(
							GetWorld(),
							EquippedWeapon->GetImpactParticles(),
							FireHit.Location
						);
					}
				}
			}

			if (EquippedWeapon->GetBeamParticles())
			{
				UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
					GetWorld(),
					EquippedWeapon->GetBeamParticles(),
					SocketTransform
				);

				if (Beam)
				{
					Beam->SetVectorParameter(FName("Target"), BeamEndPoint);
				}
			}
		}
	}
}

// Called from the Enemy Class
void UCombatComponent::UpdateKillCounter(int32 KillsToAdd)
{
	KillCounter += KillsToAdd;
	GenerateKillCounterText(KillCounter);
	StartKillCounterTimer();

	if (Character->GetIsFightingForLife())
	{
		FightForYourLifeReset();
	}
}

void UCombatComponent::GenerateKillCounterText(int32 Kills)
{
	int32 KillTextNumber = 0;

	if (KillTexts.Num() >= 0 &&
		KillCounterWidgetActorClass &&
		KillCounterWidgetActor)
	{
		if (std::fmod(Kills / KillTextSeps, 1.0) == 0)
		{
			KillTextNumber = (Kills / KillTextSeps);

			if (KillTextNumber > KillTexts.Num() - 1)
			{
				KillTextNumber = KillTexts.Num() - 1;
			}
			
			KillCounterWidgetActor->SetTextAndStartAnimation(KillTexts[KillTextNumber], true);
			ShowHideKillCounterWidget(true);
		}
	}
}

void UCombatComponent::ShowHideKillCounterWidget(bool ShouldShow)
{
	if (KillCounterWidgetActor &&
		ShouldShow)
	{
		KillCounterWidgetActor->SetActorHiddenInGame(false);
	}
	else if (KillCounterWidgetActor && !ShouldShow)
	{
		KillCounterWidgetActor->SetActorHiddenInGame(true);
	}
}

void UCombatComponent::StartKillCounterTimer()
{
	if (Character)
	{
		Character->GetWorldTimerManager().SetTimer(
			KillCounterResetTimer,
			this,
			&UCombatComponent::ResetKillCounter,
			KillCounterResetTime
		);
	}
}

void UCombatComponent::ResetKillCounter()
{
	KillCounter = 0;
	ShowHideKillCounterWidget(false);
}

void UCombatComponent::StartHitMultiplierTimer()
{
	if (Character)
	{
		Character->GetWorldTimerManager().SetTimer(
			HitMultiplierResetTimer,
			this,
			&UCombatComponent::ResetHitMultiplier,
			HitMultiplierResetTime
		);
	}
}

void UCombatComponent::ResetHitMultiplier()
{
	CalculateScore();
	UpdateHitMultiplier(-GetHitMultiplier() + 1);
}

void UCombatComponent::ClearHitMultiplierTimer()
{
	//if (Character)
	//{
	//	Character->GetWorldTimerManager().
	//		
	//		//ClearTimer(HitMultiplierResetTimer);
	//}
}

void UCombatComponent::UpdateHitCounter(int32 HitValue)
{
	HitCounter += HitValue;
}

void UCombatComponent::UpdateHitMultiplier(int32 MultiplierValue)
{
	HitMultiplier += MultiplierValue;
	ClearHitMultiplierTimer();
}

void UCombatComponent::CalculateScore()
{
	PlayerScore += HitCounter * HitMultiplier;
}

void UCombatComponent::PlayGunFireMontage()
{
	// Playing Firing Animation
	EquippedWeapon->Fire(BeamEndPoint);
}

void UCombatComponent::PlayHapticEffect()
{
	// Playing Haptic Effect
	if (Character->GetRightHandController())
	{
		Character->GetRightHandController()->PlayHapticEffect();
	}
}

void UCombatComponent::StartFireTimer()
{
	if (EquippedWeapon)
	{
		CombatState = ECombatState::ECS_FireTimerInProgress;

		Character->GetWorldTimerManager().SetTimer(
			AutoFireTimer,
			this,
			&UCombatComponent::AutoFireReset,
			EquippedWeapon->GetAutomaticFireRate()
		);
	}
}

void UCombatComponent::AutoFireReset()
{
	CombatState = ECombatState::ECS_Unoccupied;

	if (WeaponHasAmmo())
	{
		if (bFireButtonPressed)
		{
			FireWeapon();
		}
	}
	else
	{
		ReloadWeapon();
	}
}

void UCombatComponent::ReloadWeapon()
{
	if (CombatState != ECombatState::ECS_Unoccupied) return;	
	if (EquippedWeapon == nullptr) return;
	
	// Do we have ammo of the correct type?
	if (CarryingAmmo() && 
		(EquippedWeapon->GetAmmo() != EquippedWeapon->GetMagazineCapacity()))
	{
		CombatState = ECombatState::ECS_Reloading;

		if (Character)
		{
			UAnimInstance* AnimInstace = Character->GetBodyMesh()->GetAnimInstance();

			if (EquippedWeapon->GetReloadMontage() && AnimInstace)
			{
				AnimInstace->Montage_Play(EquippedWeapon->GetReloadMontage());
				AnimInstace->Montage_JumpToSection(
					EquippedWeapon->GetReloadMontageSection());

				EquippedWeapon->SetMovingClip(true);
			}
			// if no valid Reload montage playing the EquippedWeapon Reaload Animation Sequence
			else
			{
				EquippedWeapon->Reload();
				WeaponReloadAnimStart();
			}
		}
	}
}

void UCombatComponent::FightForYourLife()
{
	if (Character)
	{
		Character->SetIsFightingForLife(true);
		FightForYourLifeTimerStart();
		
		// Screen smooth fade out
		Character->StartFade(0.f, 1.f, FightForYourLifeTime);
	}
}

void UCombatComponent::FightForYourLifeTimerStart()
{
		Character->GetWorldTimerManager().SetTimer(
		FightForYourLifeTimer,
		this,
		&UCombatComponent::FightForYourLifeTimerEnd,
		FightForYourLifeTime
	);
}

void UCombatComponent::FightForYourLifeTimerEnd()
{
	if (Character)
	{
		Character->SetIsDead(true);
		Character->Die();
	}
}

void UCombatComponent::FightForYourLifeReset()
{
	Character->GetWorldTimerManager().ClearTimer(FightForYourLifeTimer);
	Character->SetIsFightingForLife(false);
	Character->SetIsDead(false);
	Health += HealtAfterSurvive;
	Character->StartFade(1.f, 0.f, 0.1f);
	// TODO: Play sound

}

bool UCombatComponent::WeaponHasAmmo()
{
	if (EquippedWeapon == nullptr) return false;

	return EquippedWeapon->GetAmmo() > 0;
}

bool UCombatComponent::CarryingAmmo()
{
	if (EquippedWeapon == nullptr) return false;

	auto AmmoType = EquippedWeapon->GetAmmoType();

	if (AmmoMap.Contains(AmmoType))
	{
		return AmmoMap[AmmoType] > 0;
	}

	return false;
}

void UCombatComponent::GrabClip()
{
	if (EquippedWeapon == nullptr) return;
	if (HandSceneComponent == nullptr) return;

	if (Character)
	{
		// Index for the clip bone on the equipped weapon.
		int32 ClipBoneIndex{ EquippedWeapon->GetItemMesh()->GetBoneIndex(EquippedWeapon->GetClipBoneName()) };
		ClipTransform = EquippedWeapon->GetItemMesh()->GetBoneTransform(ClipBoneIndex);

		FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepRelative, true);
		HandSceneComponent->AttachToComponent(Character->GetBodyMesh(), AttachmentRules, FName(TEXT("hand_l")));
		HandSceneComponent->SetWorldTransform(ClipTransform);

		EquippedWeapon->SetMovingClip(true);
	}
}

void UCombatComponent::ReleaseClip()
{
	if (EquippedWeapon == nullptr) return;

	EquippedWeapon->SetMovingClip(false);
}

void UCombatComponent::WeaponReloadAnimStart()
{
	if (EquippedWeapon->GetWeaponReloadAnimLength())
	{
		Character->GetWorldTimerManager().SetTimer(
			WeaponReloadTimer,
			this,
			&UCombatComponent::WeaponReloadAnimFinished,
			EquippedWeapon->GetWeaponReloadAnimLength()
		);
	}
}

void UCombatComponent::WeaponReloadAnimFinished()
{
	FinishReloading();
}

void UCombatComponent::FinishReloading()
{
	// Upadate the CombatState
	CombatState = ECombatState::ECS_Unoccupied;
	
	if (EquippedWeapon == nullptr) return;

	const auto AmmoType = EquippedWeapon->GetAmmoType();

	// Update the AmmoMap
	if (AmmoMap.Contains(AmmoType))
	{
		// Ammount of ammo the Character is carrying of the EquippedWeapon type
		int32 CarriedAmmo = AmmoMap[AmmoType];

		// Space left in the magazine of EquippedWeapon
		const int32 MagEmptySpace = 
			EquippedWeapon->GetMagazineCapacity() - 
			EquippedWeapon->GetAmmo();

		if (MagEmptySpace > CarriedAmmo)
		{
			// Reload the magazine all the ammo we are carrying
			EquippedWeapon->ReloadAmmo(CarriedAmmo);
			CarriedAmmo = 0;
			AmmoMap.Add(AmmoType, CarriedAmmo);
		}
		else
		{
			// fill the magazine
			EquippedWeapon->ReloadAmmo(MagEmptySpace);
			CarriedAmmo -= MagEmptySpace;
			AmmoMap.Add(AmmoType, CarriedAmmo);
		}
	}
}

bool UCombatComponent::TraceUnderCrosshairs(FHitResult& OutHitResult)
{
	// Get Viewport Size
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	// Get screen space location of crosshairs
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	// Get world postition and direction of crosshairs
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);

	if (bScreenToWorld)
	{
		// Trace from Crosshair world location outward
		/*const FVector Start{ CrosshairWorldPosition };
		const FVector End{ Start + CrosshairWorldDirection * 50'000.f };*/

		const FVector Start{ Character->GetCameraComponent()->GetComponentLocation() };
		const FVector End{ Start + Character->GetCameraComponent()->GetForwardVector() * 50'000.f};

		//DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 2.f);

		GetWorld()->LineTraceSingleByChannel(
			OutHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility
		);

		if (OutHitResult.bBlockingHit)
		{
			return true;
		}
	}

	return false;
}

