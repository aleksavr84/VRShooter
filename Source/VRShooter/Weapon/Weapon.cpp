#include "Weapon.h"
#include "Animation/AnimationAsset.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SkeletalMeshComponent.h"
#include "Casing.h"
#include "Camera/CameraShakeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AWeapon::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	const FString WeaponTablePath{ TEXT("/Script/Engine.DataTable'/Game/DataTables/DT_Weapon.DT_Weapon'") };
	UDataTable* WeaponTableObject = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *WeaponTablePath));

	if (WeaponTableObject)
	{
		FWeaponDataTable* WeaponDataRow = nullptr;

		switch (WeaponType)
		{
		case EWeaponType::EWT_Pistol:
			WeaponDataRow = WeaponTableObject->FindRow<FWeaponDataTable>(FName("Pistol"), TEXT(""));
			break;

		case EWeaponType::EWT_SubmachineGun:
			WeaponDataRow = WeaponTableObject->FindRow<FWeaponDataTable>(FName("SubmachineGun"), TEXT(""));
			break;

		case EWeaponType::EWT_Shotgun:
			WeaponDataRow = WeaponTableObject->FindRow<FWeaponDataTable>(FName("Shotgun"), TEXT(""));
			break;

		case EWeaponType::EWT_GrenadeLauncher:
			WeaponDataRow = WeaponTableObject->FindRow<FWeaponDataTable>(FName("GrenadeLauncher"), TEXT(""));
			break;
		}

		if (WeaponDataRow)
		{
			// Weapon Details -> Initialization
			SetItemName(WeaponDataRow->ItemName);
			FireType = WeaponDataRow->FireType;
			ProjectileClass = WeaponDataRow->ProjectileClass;
			CasingClass = WeaponDataRow->CasingClass;
			AmmoType = WeaponDataRow->AmmoType;
			Ammo = WeaponDataRow->WeaponAmmo;
			MagazineCapacity = WeaponDataRow->MagazineCapacity;
			DistanceToSphere = WeaponDataRow->DistanceToSphere;
			SphereRadius = WeaponDataRow->SphereRadius;
			
			//// Inventory
			SetIconItem(WeaponDataRow->InventoryIcon);
			SetAmmoIcon(WeaponDataRow->AmmoIcon);
			
			// Meshes and MaterialInstances
			GetItemSkeletalMesh()->SetSkeletalMesh(WeaponDataRow->ItemMesh);
			SetMaterialInstance(WeaponDataRow->MaterialInstance);
			SetMaterialIndex(WeaponDataRow->MaterialIndex);
			PreviousMaterialIndex = GetMaterialIndex();
			GetItemSkeletalMesh()->SetMaterial(PreviousMaterialIndex, nullptr);
			
			//// Animations and Montages
			GetItemSkeletalMesh()->SetAnimInstanceClass(WeaponDataRow->AnimBP);
			FireAnimation = WeaponDataRow->FireAnimation;
			FireAnimation2 = WeaponDataRow->FireAnimation2;
			ReloadAnimation = WeaponDataRow->ReloadAnimation;
			ReloadMontage = WeaponDataRow->ReloadMontage;
			WeaponReloadAnimLength = WeaponDataRow->WeaponReloadAnimLength;
			SetReloadMontageSection(WeaponDataRow->ReloadMontageSection);
			
			//// VFX
			MuzzleFlash = WeaponDataRow->MuzzleFlash;
			MuzzleFlashNiagara = WeaponDataRow->MuzzleFlashNiagara;
			ImpactParticles = WeaponDataRow->ImpactParticles;
			BeamParticles = WeaponDataRow->BeamParticles;
			FireCameraShake = WeaponDataRow->FireCameraShake;

			//// SFX
			SetPickupSound(WeaponDataRow->PickupSound);
			SetEquipSound(WeaponDataRow->EquipSound);
			FireSound = WeaponDataRow->FireSound;
			
			//// Bones and Sockets
			ClipBoneName = WeaponDataRow->ClipBoneName;
			HandSocketName = WeaponDataRow->HandSocketName;
			MuzzleFlashSocketName = WeaponDataRow->MuzzleFlashSocketName;

			//// Weapon Properties
			//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties")
			//	bool bAutomatic;
			//
			AutomaticFireRate = WeaponDataRow->AutoFireRate;
			Damage = WeaponDataRow->Damage;
			HeadShotDamage = WeaponDataRow->HeadShotDamage;
			//// Crosshairs
			//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crosshairs")
			//	UTexture2D* CrosshairsMiddle;
			//
			//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crosshairs")
			//	UTexture2D* CrosshairsLeft;
			//
			//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crosshairs")
			//	UTexture2D* CrosshairsRight;
			//
			//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crosshairs")
			//	UTexture2D* CrosshairsBottom;
			//
			//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crosshairs")
			//	UTexture2D* CrosshairsTop;

			if (GetMaterialInstance())
			{
				SetDynamicMaterialInstance(UMaterialInstanceDynamic::Create(GetMaterialInstance(), this));
				//GetDynamicMaterialInstance()->SetVectorParameterValue(TEXT("FresnelColor"), GetGlowColor());
				GetItemSkeletalMesh()->SetMaterial(GetMaterialIndex(), GetDynamicMaterialInstance());

				//EnableGlowMaterial();
			}
		}
	}
}

void AWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit)
{
	UWorld* World = GetWorld();

	if (World)
	{
		FVector End = TraceStart + (HitTarget - TraceStart) * 1.25f;
		//FVector End = TraceStart + (HitTarget - TraceStart) * 50'000.f;

		World->LineTraceSingleByChannel(
			OutHit,
			TraceStart,
			End,
			ECollisionChannel::ECC_Visibility
		);

		FVector BeamEnd = End;

		if (OutHit.bBlockingHit)
		{
			BeamEnd = OutHit.ImpactPoint;
		}

		/*DrawDebugSphere(
			World,
			BeamEnd,
			16.f,
			12,
			FColor::Orange,
			true
		);*/

		if (BeamParticles)
		{
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
				World,
				BeamParticles,
				TraceStart,
				FRotator::ZeroRotator,
				true
			);
			if (Beam)
			{
				Beam->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}
	}
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//// Keep the Weapon upright
	//if (GetItemState() == EItemState::EIS_Falling && bFalling)
	//{
	//	FRotator MeshRotation{ 0.f, GetItemMesh()->GetComponentRotation().Yaw, 0.f };
	//	SetActorRotation(FQuat(FRotator(0.f, 0.f, 0.f)));
	//	GetItemMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);
	//}
}

void AWeapon::Fire(const FVector& HitTarget)
{
	FireCounter++;

	if (FireAnimation &&
		FireCounter == 1)
	{
		GetItemSkeletalMesh()->PlayAnimation(FireAnimation, false);
	}
	if (FireAnimation2 &&
		FireCounter == 2)
	{
		GetItemSkeletalMesh()->PlayAnimation(FireAnimation2, false);
	} 
	else
	{
		if (FireAnimation)
		{
			GetItemSkeletalMesh()->PlayAnimation(FireAnimation, false);
		}
	}

	// Reset FireCounter
	if (FireCounter == 2) FireCounter = 0;

	if (FireCameraShake)
	{
		UGameplayStatics::PlayWorldCameraShake(
			GetWorld(),
			FireCameraShake,
			GetActorLocation(),
			1000.f,
			1000.f
		);
	}

	if (CasingClass)
	{
		USkeletalMeshComponent* WeaponMesh = GetItemSkeletalMesh();
		const USkeletalMeshSocket* AmmoEjectSocket = WeaponMesh->GetSocketByName(FName("AmmoEject"));

		if (AmmoEjectSocket)
		{
			FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(WeaponMesh);
			FActorSpawnParameters SpawnParams;
			UWorld* World = GetWorld();

			if (World)
			{
				World->SpawnActor<ACasing>(
					CasingClass,
					SocketTransform.GetLocation(),
					SocketTransform.GetRotation().Rotator(),
					SpawnParams
					);
			}
		}
	}
}

void AWeapon::Reload()
{
	if (ReloadAnimation)
	{
		GetItemSkeletalMesh()->PlayAnimation(ReloadAnimation, false);
	}
}

void AWeapon::DecrementAmmo()
{
	if (Ammo - 1 <= 0)
	{
		Ammo = 0;
	}
	else
	{
		--Ammo;
	}
}

const USkeletalMeshSocket* AWeapon::GetMuzzleFlashSocket()
{
	USkeletalMeshComponent* WeaponMesh = GetItemSkeletalMesh();

	if (WeaponMesh)
	{
		return WeaponMesh->GetSocketByName(MuzzleFlashSocketName);
	}

	return nullptr;
}

void AWeapon::ReloadAmmo(int32 Amount)
{
	checkf(Ammo + Amount <= MagazineCapacity, TEXT("Attemted to reload with more than magazine capacity"));
	Ammo += Amount;
}
