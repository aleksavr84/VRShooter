#include "Item.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"
#include "VRShooter/Character/VRShooterCharacter.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

AItem::AItem()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	SetRootComponent(CollisionBox);

	ItemSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ItemSkeletalMesh"));
	ItemSkeletalMesh->SetupAttachment(CollisionBox);

	ItemStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemStaticMesh"));
	ItemStaticMesh->SetupAttachment(CollisionBox);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(CollisionBox);

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(CollisionBox);

	MarkerSystemComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("MarkerSystemComponent"));
	MarkerSystemComponent->SetupAttachment(CollisionBox);
}

void AItem::OnConstruction(const FTransform& Transform)
{
	// Load the data in the ItemRarity DataTable

	// Path to the Item Rarity DataTable
	FString RarityTablePath(TEXT("/Script/Engine.DataTable'/Game/DataTables/DT_ItemRarity.DT_ItemRarity'"));
	UDataTable* RarityTableObject = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *RarityTablePath));
	
	if (RarityTableObject)
	{
		/*UE_LOG(LogTemp, Warning, TEXT("RarityTableObject"));*/
		FItemRarityTable* RarityRow = nullptr;
		switch (ItemRarity)
		{
			case EItemRarity::EIR_Damaged:
				RarityRow = RarityTableObject->FindRow<FItemRarityTable>(FName("Damaged"), TEXT(""));
				break;

			case EItemRarity::EIR_Common:
				RarityRow = RarityTableObject->FindRow<FItemRarityTable>(FName("Common"), TEXT(""));
				break;

			case EItemRarity::EIR_Uncommon:
				RarityRow = RarityTableObject->FindRow<FItemRarityTable>(FName("Uncommon"), TEXT(""));
				break;

			case EItemRarity::EIR_Rare:
				RarityRow = RarityTableObject->FindRow<FItemRarityTable>(FName("Rare"), TEXT(""));
				break;

			case EItemRarity::EIR_Legendary:
				RarityRow = RarityTableObject->FindRow<FItemRarityTable>(FName("Legendary"), TEXT(""));
				break;
		}

		if (RarityRow)
		{
			/*UE_LOG(LogTemp, Warning, TEXT("RarityRow"));*/
			GlowColor = RarityRow->GlowColor;
			LightColor = RarityRow->LightColor;
			DarkColor = RarityRow->DarkColor;
			NumberOfStarts = RarityRow->NumberOfStars;
			IconBackground = RarityRow->IconBackground;
		}
	}

	if (MaterialInstance)
	{
		DynamicMaterialInstance = UMaterialInstanceDynamic::Create(MaterialInstance, this);
		//DynamicMaterialInstance->SetVectorParameterValue(TEXT("FresnelColor"), GlowColor);
		ItemSkeletalMesh->SetMaterial(MaterialIndex, DynamicMaterialInstance);

		//EnableGlowMaterial();
	}
}

void AItem::BeginPlay()
{
	Super::BeginPlay();
	
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
		SetActiveStas();
	}

	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnSphereOverlap);
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AItem::OnSphereEndOverlap);

	ItemDefaultRotation = GetValidMeshComponent()->GetComponentRotation();
	// Set Item properties based on ItemState
	SetItemProperties(ItemState);
}

void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Handle Item Interping when in the EquipInterping state
	ItemInterp(DeltaTime);

	if (GetValidMeshComponent() &&
		(ItemState == EItemState::EIS_Pickup ||
		ItemState == EItemState::EIS_EquipInterping))
	{
		GetItemStaticMesh()->AddWorldRotation(FRotator(0.f, BaseTurnRate * DeltaTime, 0.f));
		GetItemSkeletalMesh()->AddWorldRotation(FRotator(0.f, BaseTurnRate * DeltaTime, 0.f));
	}

	//// TODO: This is only a Temporary Solution -> The pickup Widget doesn't hide after the player not looking at the Item
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}
}

UMeshComponent* AItem::GetValidMeshComponent()
{
	if (ItemSkeletalMesh)
	{
		return ItemSkeletalMesh;
	}
	else if (ItemStaticMesh)
	{
		return ItemStaticMesh;
	}
	return nullptr;
}

void AItem::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bfromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		AVRShooterCharacter* VRShooterCharacter = Cast<AVRShooterCharacter>(OtherActor);

		if (VRShooterCharacter)
		{
			VRShooterCharacter->IncrementOverlappedItemCount(1);
		}
	}
}

void AItem::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		AVRShooterCharacter* VRShooterCharacter = Cast<AVRShooterCharacter>(OtherActor);

		if (VRShooterCharacter)
		{
			VRShooterCharacter->IncrementOverlappedItemCount(-1);
			VRShooterCharacter->UnHighlightInventorySlot();
		}
	}
}

void AItem::ShowPickupWidget(bool Visible)
{
	if (PickupWidget)
	{
		if (ItemState == EItemState::EIS_Pickup)
		{
			PickupWidget->SetVisibility(Visible);
		}
		else
		{
			PickupWidget->SetVisibility(false);
		}
	}
}

void AItem::SetItemState(EItemState State)
{
	ItemState = State;
	SetItemProperties(State);
}

void AItem::SetActiveStas()
{
	// the 0 element ins't used
	for (int32 i = 0; i <= 5; i++)
	{
		ActiveStars.Add(false);
	}

	switch (ItemRarity)
	{
	case EItemRarity::EIR_Damaged:
		ActiveStars[1] = true;
		break;
	case EItemRarity::EIR_Common:
		ActiveStars[1] = true;
		ActiveStars[2] = true;
		break;
	case EItemRarity::EIR_Uncommon:
		ActiveStars[1] = true;
		ActiveStars[2] = true;
		ActiveStars[3] = true;
		break;
	case EItemRarity::EIR_Rare:
		ActiveStars[1] = true;
		ActiveStars[2] = true;
		ActiveStars[3] = true;
		ActiveStars[4] = true;
		break;
	case EItemRarity::EIR_Legendary:
		ActiveStars[1] = true;
		ActiveStars[2] = true;
		ActiveStars[3] = true;
		ActiveStars[4] = true;
		ActiveStars[5] = true;
		break;
	}
}

void AItem::SetItemProperties(EItemState State)
{
	switch (State)
	{
	case EItemState::EIS_Pickup:
		PickupWidget->SetVisibility(false);
		// Set Mesh properties
		GetValidMeshComponent()->SetSimulatePhysics(false);
		GetValidMeshComponent()->SetEnableGravity(false);
		GetValidMeshComponent()->SetVisibility(true);
		GetValidMeshComponent()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		GetValidMeshComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
		// Set AreaSphere properties
		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		
		// Set CollisionBox properties
		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		break;

	case EItemState::EIS_Equipped:
		PickupWidget->SetVisibility(false);
		// Set Mesh properties
		GetValidMeshComponent()->SetSimulatePhysics(false);
		GetValidMeshComponent()->SetEnableGravity(false);
		GetValidMeshComponent()->SetVisibility(true);
		GetValidMeshComponent()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		GetValidMeshComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
		// Set AreaSphere properties
		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// Set CollisionBox properties
		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;

	case EItemState::EIS_Falling:
		PickupWidget->SetVisibility(false);
		// Set Mesh properties
		GetValidMeshComponent()->SetSimulatePhysics(true);
		GetValidMeshComponent()->SetEnableGravity(true);
		GetValidMeshComponent()->SetVisibility(true);
		GetValidMeshComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		GetValidMeshComponent()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		GetValidMeshComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
		//ItemMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Block);
		
		// Set AreaSphere properties
		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// Set CollisionBox properties
		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;

	case EItemState::EIS_EquipInterping:
		PickupWidget->SetVisibility(false);
		// Set Mesh properties
		GetValidMeshComponent()->SetSimulatePhysics(false);
		GetValidMeshComponent()->SetEnableGravity(false);
		GetValidMeshComponent()->SetVisibility(true);
		GetValidMeshComponent()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		GetValidMeshComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// Set AreaSphere properties
		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// Set CollisionBox properties
		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;

	case EItemState::EIS_PickedUp:
		PickupWidget->SetVisibility(false);

		// Set mesh properties
		GetValidMeshComponent()->SetSimulatePhysics(false);
		GetValidMeshComponent()->SetEnableGravity(false);
		GetValidMeshComponent()->SetVisibility(false);
		GetValidMeshComponent()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		GetValidMeshComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// Set AreaSphere properties
		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// Set CollisionBox properties
		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	}
}

void AItem::RotateWidgetToPlayer(FVector PlayerLocation)
{
	if (ItemState == EItemState::EIS_Pickup)
	{
		FRotator WidgetRotation = PickupWidget->GetRelativeRotation();
		FVector Direction = PlayerLocation - GetActorLocation();
		FRotator Rotation = FRotationMatrix::MakeFromX(Direction).Rotator();
		PickupWidget->SetWorldRotation(FRotator(WidgetRotation.Pitch, Rotation.Yaw, WidgetRotation.Roll));
	}
}

FVector AItem::GetInterpLocation()
{
	if (ShooterCharacter == nullptr) return FVector(0.f);

	switch (ItemType)
	{
	case EItemType::EIT_Ammo:
		return ShooterCharacter->GetInterpLoctaion(InterpLocIndex).SceneComponent->GetComponentLocation();
		
		break;
	case EItemType::EIT_Weapon:
		return ShooterCharacter->GetInterpLoctaion(0).SceneComponent->GetComponentLocation();
		
		break;
	}

	return FVector();
}

void AItem::ItemInterp(float DeltaTime)
{
	if (!bInterping) return;

	if (ShooterCharacter && ItemZCurve)
	{
		// Elapsed time since we started ItemInterpTimer
		const float ElapsedTime = GetWorldTimerManager().GetTimerElapsed(ItemInterpTimer);
		// Get curve value corresponding to ElapsedTime
		const float CurveValue = ItemZCurve->GetFloatValue(ElapsedTime);
		// Get the item's initial location when the curve started
		FVector ItemLocation = ItemInterpStartLocation;
		// Get location in the front of the camera
		//const FVector CameraInterpLocation{ ShooterCharacter->GetCameraInterpLocation() };
		const FVector CameraInterpLocation{ GetInterpLocation() };
		// Vector from item to CameraInterpLocation (X and Y are zeroed out)
		const FVector ItemToCamera{ FVector(0.f, 0.f, (CameraInterpLocation - ItemLocation).Z) };
		// Scale factor to multiply with CurveValue
		const float DeltaZ = ItemToCamera.Size();

		const FVector CurrentLocation{ GetActorLocation() };
		// Interpolated X value
		const float InterpXValue = FMath::FInterpTo(
			CurrentLocation.X, 
			CameraInterpLocation.X, 
			DeltaTime,
			30.0f
		);
		// Interpolated Y value
		const float InterpYValue = FMath::FInterpTo(
			CurrentLocation.Y,
			CameraInterpLocation.Y,
			DeltaTime,
			30.f
		);

		// Set X and Y of ItemLocation to Interped values
		ItemLocation.X = InterpXValue;
		ItemLocation.Y = InterpYValue;

		// Adding curve value to the Z component of the InitialLocation (Scaled by DeltaZ)
		ItemLocation.Z += CurveValue * DeltaZ;
		SetActorLocation(ItemLocation, true, nullptr, ETeleportType::TeleportPhysics);

		// Camera rotation this frame
		FRotator CameraRotation{ ShooterCharacter->GetCameraComponent()->GetComponentRotation() };
		
		// Camera rotation plus initial Yaw offset
		FRotator ItemRotation{ 0.f, CameraRotation.Yaw + InterpInitialYawOffset ,0.f };
		SetActorRotation(ItemRotation, ETeleportType::TeleportPhysics);

		if (ItemScaleCurve)
		{
			const float ScaleCurveValue = ItemScaleCurve->GetFloatValue(ElapsedTime);
			SetActorScale3D(FVector(ScaleCurveValue, ScaleCurveValue, ScaleCurveValue));
		}
	}
}

void AItem::StartItemCurve(AVRShooterCharacter* Character, bool bForcePlaySound)
{
	if (Character)
	{
		// Store a handle to the character
		ShooterCharacter = Character;

		// Get array index in InterpLocations with the lowest item count
		InterpLocIndex = ShooterCharacter->GetInterpLocationIndex();
		// Add 1 to the Item Count for this interp location struct
		ShooterCharacter->IncrementInterpLocItemCount(InterpLocIndex, 1);

		PlayPickupSound(bForcePlaySound);

		// Hide Marker
		MarkerSystemComponent->SetVisibility(false);

		// Store initial location of the item
		ItemInterpStartLocation = GetActorLocation();
		bInterping = true;
		SetItemState(EItemState::EIS_EquipInterping);

		GetWorldTimerManager().SetTimer(
			ItemInterpTimer,
			this,
			&AItem::FinishInterping,
			ZCurveTime
		);

		// Get initial Yaw of the Camera
		const double CameraRotationYaw{ ShooterCharacter->GetCameraComponent()->GetComponentRotation().Yaw };
		// Get initial Yaw of the Item
		const double ItemRotationYaw{ GetActorRotation().Yaw };
		// Initial Yaw offset between camera and item
		InterpInitialYawOffset = ItemRotationYaw - CameraRotationYaw;

		// VFX -> PostProcess
		ShooterCharacter->StartPostProcess(PickUpPostProcess, PostProcessEffectTime);
	}
}

void AItem::PlayPickupSound(bool bForcePlaySound)
{
	if (ShooterCharacter)
	{
		if (bForcePlaySound)
		{
			if (PickupSound)
			{
				UGameplayStatics::PlaySound2D(
					this,
					PickupSound
				);
			}
		}
		else if (ShooterCharacter->ShouldPlayPickupSound())
		{
			ShooterCharacter->StartPickupSoundTimer();

			if (PickupSound)
			{
				UGameplayStatics::PlaySound2D(
					this,
					PickupSound
				);
			}
		}
	}
}

void AItem::PlayEquipSound(bool bForcePlaySound)
{
	if (ShooterCharacter)
	{

		if (bForcePlaySound)
		{
			if (EquipSound)
			{
				UGameplayStatics::PlaySound2D(
					this,
					EquipSound
				);
			}
		}
		else if (ShooterCharacter->ShouldPlayEquipSound())
		{
			ShooterCharacter->StartEquipSoundTimer();

			if (EquipSound)
			{
				UGameplayStatics::PlaySound2D(
					this,
					EquipSound
				);
			}
		}
	}
}

void AItem::FinishInterping()
{
	bInterping = false;

	if (ShooterCharacter)
	{
		// Subtract 1 from the ItemCount of the interp location struct
		ShooterCharacter->IncrementInterpLocItemCount(InterpLocIndex, -1);
		ShooterCharacter->GetPickupItem(this);
		ShooterCharacter->UnHighlightInventorySlot();
	}

	// Set scale back to normal
	SetActorScale3D(FVector(1.f));
}

void AItem::ThrowItem(bool bForward, float Impulse)
{
	FRotator MeshRotation;
	FVector MeshRight;
	FVector MeshForward;
	FVector MeshUp;
	
	MeshRotation = FRotator(0.f, GetValidMeshComponent()->GetComponentRotation().Yaw, 0.f);
	GetValidMeshComponent()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);
	MeshForward = GetValidMeshComponent()->GetForwardVector();
	MeshUp = GetValidMeshComponent()->GetUpVector();
	MeshRight = GetValidMeshComponent()->GetRightVector();
	
	// Direction in which we throw the Weapon
	FVector ImpulseDirection = MeshRight.RotateAngleAxis(-20.f, bForward ? MeshForward : MeshUp);

	float RandomRotation{ FMath::FRandRange(10.f, 30.f) };
	ImpulseDirection = ImpulseDirection.RotateAngleAxis(RandomRotation, FVector(0.f, 0.f, 1.f));
	ImpulseDirection *= Impulse;

	bFalling = true;

	GetValidMeshComponent()->AddImpulse(ImpulseDirection);
	
	GetWorldTimerManager().SetTimer(
		ThrowItemTimer,
		this,
		&AItem::StopFalling,
		ThrowItemTime);
}

void AItem::StopFalling()
{
	FRotator MeshRotation;

	MeshRotation = FRotator(0.f, GetValidMeshComponent()->GetComponentRotation().Yaw, 0.f);
	SetActorRotation(FQuat(FRotator(0.f, 0.f, 0.f)));
	GetValidMeshComponent()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);

	SetItemState(EItemState::EIS_Pickup);
	bFalling = false;
}

