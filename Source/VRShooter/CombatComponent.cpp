#include "CombatComponent.h"
#include "VRShooterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Sound/SoundCue.h"
#include "Engine/SkeletalMeshSocket.h"
#include "DrawDebugHelpers.h"
#include "Particles/ParticleSystemComponent.h"


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
	if (FireSound)
	{
		UGameplayStatics::PlaySound2D(this, FireSound);
	}

	const USkeletalMeshComponent* WeaponMesh = Character->GetRightHandController()->GetWeaponMesh();

	if (WeaponMesh)
	{
		const USkeletalMeshSocket* BarrelSocket = WeaponMesh->GetSocketByName("Muzzle");

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

