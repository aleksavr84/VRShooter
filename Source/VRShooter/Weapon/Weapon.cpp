#include "Weapon.h"
#include "Animation/AnimationAsset.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SkeletalMeshComponent.h"
#include "Casing.h"

AWeapon::AWeapon() :
	ThrowWeaponTime(.5f),
	bFalling(false)
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
			
			//// Inventory
			SetIconItem(WeaponDataRow->InventoryIcon);
			SetAmmoIcon(WeaponDataRow->AmmoIcon);
			
			// Meshes and MaterialInstances
			GetItemMesh()->SetSkeletalMesh(WeaponDataRow->ItemMesh);
			SetMaterialInstance(WeaponDataRow->MaterialInstance);
			SetMaterialIndex(WeaponDataRow->MaterialIndex);
			PreviousMaterialIndex = GetMaterialIndex();
			GetItemMesh()->SetMaterial(PreviousMaterialIndex, nullptr);
			
			//// Animations and Montages
			GetItemMesh()->SetAnimInstanceClass(WeaponDataRow->AnimBP);
			FireAnimation = WeaponDataRow->FireAnimation;
			ReloadAnimation = WeaponDataRow->ReloadAnimation;
			ReloadMontage = WeaponDataRow->ReloadMontage;
			WeaponReloadAnimLength = WeaponDataRow->WeaponReloadAnimLength;
			SetReloadMontageSection(WeaponDataRow->ReloadMontageSection);
			
			//// VFX
			MuzzleFlash = WeaponDataRow->MuzzleFlash;
			MuzzleFlashNiagara = WeaponDataRow->MuzzleFlashNiagara;
			ImpactParticles = WeaponDataRow->ImpactParticles;
			BeamParticles = WeaponDataRow->BeamParticles;
			
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
				GetItemMesh()->SetMaterial(GetMaterialIndex(), GetDynamicMaterialInstance());

				//EnableGlowMaterial();
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

void AWeapon::ThrowWeapon()
{

	FRotator MeshRotation{ 0.f, GetItemMesh()->GetComponentRotation().Yaw, 0.f };
	GetItemMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);

	const FVector MeshForward{ GetItemMesh()->GetForwardVector() };
	const FVector MeshRight{ GetItemMesh()->GetRightVector() };
	
	// Direction in which we throw the Weapon
	FVector ImpulseDirection = MeshRight.RotateAngleAxis(-20.f, MeshForward);

	float RandomRotation{ FMath::FRandRange(10.f, 30.f) };
	ImpulseDirection = ImpulseDirection.RotateAngleAxis(RandomRotation, FVector(0.f, 0.f, 1.f));
	ImpulseDirection *= 2'000.f;
	
	bFalling = true;
	
	GetItemMesh()->AddImpulse(ImpulseDirection);

	GetWorldTimerManager().SetTimer(
		ThrowWeaponTimer, 
		this, 
		&AWeapon::StopFalling, 
		ThrowWeaponTime);
}

void AWeapon::StopFalling()
{
	FRotator MeshRotation{ 0.f, GetItemMesh()->GetComponentRotation().Yaw, 0.f };
	SetActorRotation(FQuat(FRotator(0.f, 0.f, 0.f)));
	GetItemMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);

	SetItemState(EItemState::EIS_Pickup);
	bFalling = false;
}

void AWeapon::Fire(const FVector& HitTarget)
{
	if (FireAnimation)
	{
		GetItemMesh()->PlayAnimation(FireAnimation, false);
	}

	if (CasingClass)
	{
		USkeletalMeshComponent* WeaponMesh = GetItemMesh();
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
		GetItemMesh()->PlayAnimation(ReloadAnimation, false);
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
	USkeletalMeshComponent* WeaponMesh = GetItemMesh();

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
