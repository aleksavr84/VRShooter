#include "CombatComponent.h"
#include "VRShooterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Sound/SoundCue.h"
#include "Engine/SkeletalMeshSocket.h"
#include "DrawDebugHelpers.h"
#include "Particles/ParticleSystemComponent.h"
#include "Weapon.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
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

void UCombatComponent::EquipWeapon(class AWeapon* WeaponToEquip)
{
	if (WeaponToEquip)
	{
		// Get the HandMesh and the HandSocket
		USkeletalMeshComponent* HandMesh = Character->GetBodyMesh(); //Character->GetRightHandController()->GetHandMesh();
		const USkeletalMeshSocket* HandSocket = HandMesh->GetSocketByName(FName("RightHandSocket"));

		if (HandSocket)
		{
			// Attach the Weapon to the HandSocket
			HandSocket->AttachActor(WeaponToEquip, HandMesh);

			EquippedWeapon = WeaponToEquip;
			EquippedWeapon->SetItemState(EItemState::EIS_Equipped);

			bIsEquipped = true;
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
	DropWeapon();
	EquipWeapon(WeaponToSwap);
}

void UCombatComponent::InitializeAmmoMap()
{
	AmmoMap.Add(EAmmoType::EAT_9mm, Starting9mmAmmo);
	AmmoMap.Add(EAmmoType::EAT_AR, StartingARAmmo);
}

void UCombatComponent::FireButtonPressed()
{
	bFireButtonPressed = true;
	StartFireTimer();
}

void UCombatComponent::FireButtonReleased()
{
	bFireButtonPressed = false;
}

void UCombatComponent::AutoFireReset()
{
	bShouldFire = true;

	if (bFireButtonPressed)
	{
		StartFireTimer();
	}
}

void UCombatComponent::StartFireTimer()
{
	if (bShouldFire)
	{
		FireWeapon();
		bShouldFire = false;
		Character->GetWorldTimerManager().SetTimer(
			AutoFireTimer,
			this,
			&UCombatComponent::AutoFireReset,
			AutomaticFireRate
		);
	}
}

void UCombatComponent::FireWeapon()
{
	if (EquippedWeapon)
	{
		if (FireSound)
		{
			UGameplayStatics::PlaySound2D(this, FireSound);
		}
		
		// Playing Firing Animation
		EquippedWeapon->Fire();

		// Playing Haptic Effect
		if (Character->GetRightHandController())
		{
			Character->GetRightHandController()->PlayHapticEffect();
		}

		const USkeletalMeshComponent* WeaponMesh = EquippedWeapon->GetItemMesh();

		if (WeaponMesh)
		{
			const USkeletalMeshSocket* BarrelSocket = WeaponMesh->GetSocketByName(FName("Muzzle"));

			if (BarrelSocket)
			{
				const FTransform SocketTransform = BarrelSocket->GetSocketTransform(WeaponMesh);

				if (MuzzleFlash)
				{
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
				}

				FHitResult FireHit;
				const FVector Start{ SocketTransform.GetLocation() };
				const FQuat Rotation{ SocketTransform.GetRotation() };
				const FVector RotationAxis{ Rotation.GetAxisX() };
				const FVector End{ Start + RotationAxis * 50'000.f };

				FVector BeamEndPoint{ End };

				GetWorld()->LineTraceSingleByChannel(
					FireHit,
					Start,
					End,
					ECollisionChannel::ECC_Visibility
				);

				if (FireHit.bBlockingHit)
				{
					/*DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 2.f);
					DrawDebugPoint(GetWorld(), FireHit.Location, 5.f, FColor::Red, false, 2.f);*/

					BeamEndPoint = FireHit.Location;

					if (ImpactParticles)
					{
						UGameplayStatics::SpawnEmitterAtLocation(
							GetWorld(),
							ImpactParticles,
							FireHit.Location
						);
					}
				}

				if (BeamParticles)
				{
					UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
						GetWorld(),
						BeamParticles,
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
		const FVector Start{ CrosshairWorldPosition };
		const FVector End{ Start + CrosshairWorldDirection * 50'000.f };
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


