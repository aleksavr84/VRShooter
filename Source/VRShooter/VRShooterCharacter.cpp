#include "VRShooterCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Components/PostProcessComponent.h"
#include "TimerManager.h"
#include "GameFramework/Controller.h"
#include "Components/CapsuleComponent.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "CombatComponent.h"
#include "Item.h"
#include "Weapon.h"
#include "Components/WidgetComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "VRHUD.h"
#include "Sound/SoundCue.h"
#include "Ammo.h"
#include "EnemyController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Camera/CameraShakeBase.h"

AVRShooterCharacter::AVRShooterCharacter()
{
 	PrimaryActorTick.bCanEverTick = true;

	VRRoot = CreateDefaultSubobject<USceneComponent>(FName("VRRoot"));
	VRRoot->SetupAttachment(GetRootComponent());

	Camera = CreateDefaultSubobject<UCameraComponent>(FName("Camera"));
	Camera->SetupAttachment(VRRoot);

	//HUDWidget = CreateDefaultSubobject<UWidgetComponent>(FName("HUDWidget"));
	//HUDWidget->SetupAttachment(Camera, FName());

	BodyMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(Camera, FName());

	TeleportPath = CreateDefaultSubobject<USplineComponent>(FName("TeleportPath"));
	TeleportPath->SetupAttachment(VRRoot);

	DestinationMarker = CreateDefaultSubobject<UStaticMeshComponent>(FName("DestinationMarker"));
	DestinationMarker->SetupAttachment(GetRootComponent());

	PostProcessComponent = CreateDefaultSubobject<UPostProcessComponent>(FName("PostProcessComponent"));
	PostProcessComponent->SetupAttachment(GetRootComponent());

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));

	// Create interpolation components
	WeaponInterpComp = CreateDefaultSubobject<USceneComponent>(TEXT("WeaponInterpolationComponent"));
	WeaponInterpComp->SetupAttachment(Camera);

	InterpComp1 = CreateDefaultSubobject<USceneComponent>(TEXT("InterpolationComponent1"));
	InterpComp1->SetupAttachment(Camera);

	InterpComp2 = CreateDefaultSubobject<USceneComponent>(TEXT("InterpolationComponent2"));
	InterpComp2->SetupAttachment(Camera);

	InterpComp3 = CreateDefaultSubobject<USceneComponent>(TEXT("InterpolationComponent3"));
	InterpComp3->SetupAttachment(Camera);

	InterpComp4 = CreateDefaultSubobject<USceneComponent>(TEXT("InterpolationComponent4"));
	InterpComp4->SetupAttachment(Camera);

	InterpComp5 = CreateDefaultSubobject<USceneComponent>(TEXT("InterpolationComponent5"));
	InterpComp5->SetupAttachment(Camera);

	InterpComp6 = CreateDefaultSubobject<USceneComponent>(TEXT("InterpolationComponent6"));
	InterpComp6->SetupAttachment(Camera);
}

void AVRShooterCharacter::PostInitializeComponents() 
{
	Super::PostInitializeComponents();

	if (Combat)
	{
		Combat->Character = this;
	}
}

void AVRShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	DestinationMarker->SetVisibility(false);

	if (BlinkerMaterialBase)
	{
		BlinkerMaterialInstance = UMaterialInstanceDynamic::Create(BlinkerMaterialBase, this);
		PostProcessComponent->AddOrUpdateBlendable(BlinkerMaterialInstance);
	}

	if (HandControllerClass)
	{
		LeftController = GetWorld()->SpawnActor<AHandController>(HandControllerClass);

		if (LeftController)
		{
			LeftController->AttachToComponent(VRRoot, FAttachmentTransformRules::KeepRelativeTransform);
			LeftController->SetHand(EControllerHand::Left);
			LeftController->SetOwner(this);
		}

		RightController = GetWorld()->SpawnActor<AHandController>(HandControllerClass);

		if (RightController)
		{
			RightController->AttachToComponent(VRRoot, FAttachmentTransformRules::KeepRelativeTransform);
			RightController->SetHand(EControllerHand::Right);
			RightController->SetOwner(this);

			////// Spawning HUD
			//FActorSpawnParameters SpawnParam;
			//SpawnParam.Instigator = this;
			//VRHUD = GetWorld()->SpawnActor<AVRHUD>(VRHUDClass, SpawnParam);
			//VRHUD->AttachToComponent(Cast<USceneComponent>(RightController), FAttachmentTransformRules::KeepRelativeTransform);
			////VRHUD->SetHandController(RightController);
			//VRHUD->SetOwner(this);
		}

		if (LeftController && RightController)
		{
			LeftController->PairController(RightController);
		}
	}

	// Spawn the DefaultWeapon and Equip it
	if (Combat)
	{
		Combat->InitializeAmmoMap();
		Combat->EquipWeapon(Combat->SpawnDefaultWeapon());
		if (Combat->EquippedWeapon)
		{
			Combat->Inventory.Add(Combat->EquippedWeapon);
			Combat->EquippedWeapon->SetSlotIndex(0);
			Combat->EquippedWeapon->SetCharacter(this);
		}
	}

	// Create FInterpLocation structs for each interp location. Add  to array
	InitializeInterpLocations();
}

void AVRShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Location corretion in PlayerSpace
	FVector NewCameraOffset = Camera->GetComponentLocation() - GetActorLocation();
	NewCameraOffset.Z = 0;
	AddActorWorldOffset(NewCameraOffset);
	VRRoot->AddWorldOffset(-NewCameraOffset);

	//UpdateDestinationMarker();
	
	//UpdateBlinkers();

	TraceForItems();

	// Calculate FPS
	FrameRate = 1 / DeltaTime;
}

void AVRShooterCharacter::InitializeInterpLocations()
{
	FInterpLocation WeaponLocation{ WeaponInterpComp, 0 };
	InterpLocations.Add(WeaponLocation);

	FInterpLocation InterpLoc1{ InterpComp1, 0 };
	InterpLocations.Add(InterpLoc1);

	FInterpLocation InterpLoc2{ InterpComp2, 0 };
	InterpLocations.Add(InterpLoc2);

	FInterpLocation InterpLoc3{ InterpComp3, 0 };
	InterpLocations.Add(InterpLoc3);

	FInterpLocation InterpLoc4{ InterpComp4, 0 };
	InterpLocations.Add(InterpLoc4);

	FInterpLocation InterpLoc5{ InterpComp5, 0 };
	InterpLocations.Add(InterpLoc5);

	FInterpLocation InterpLoc6{ InterpComp6, 0 };
	InterpLocations.Add(InterpLoc6);
}

void AVRShooterCharacter::UnHighlightInventorySlot()
{
	if (Combat)
	{
		Combat->UnHighlightInventorySlot();
	}
}

int32 AVRShooterCharacter::GetInterpLocationIndex()
{
	int32 LowestIndex = 1;
	int32 LowestCount = INT_MAX;

	for (int32 i = 1; i < InterpLocations.Num(); i++)
	{
		if (InterpLocations[i].ItemCount < LowestCount)
		{
			LowestIndex = i;
			LowestCount = InterpLocations[i].ItemCount;
		}
	}

	return LowestIndex;
}

void AVRShooterCharacter::IncrementInterpLocItemCount(int32 Index, int32 Amount)
{
	if (Amount < -1 || Amount > 1) return;

	if (InterpLocations.Num() >= Index)
	{
		InterpLocations[Index].ItemCount += Amount;
	}
}

void AVRShooterCharacter::ShowWeaponHUD()
{
	if (Combat &&
		Combat->GetIsEquipped() &&
		!bWeaponHUDShowing)
	{
		//// Spawning HUD
		FActorSpawnParameters SpawnParam;
		SpawnParam.Instigator = this;
		VRHUD = GetWorld()->SpawnActor<AVRHUD>(VRHUDClass, SpawnParam);
		VRHUD->AttachToComponent(Cast<USceneComponent>(RightController), FAttachmentTransformRules::KeepRelativeTransform);
		//VRHUD->SetHandController(RightController);
		VRHUD->SetOwner(this);

		bWeaponHUDShowing = true;
	}
}

bool AVRShooterCharacter::FindTeleportDestination(TArray<FVector>& OutPath, FVector& OutLocation)
{
	const USkeletalMeshSocket* TeleportSocket = BodyMesh->GetSocketByName(FName("LeftHandTeleportSocket"));
	FTransform TeleportSocketTransform = TeleportSocket->GetSocketTransform(BodyMesh);
	
	FVector Start = TeleportSocketTransform.GetLocation(); //LeftController->GetActorLocation();
	FVector Look = LeftController->GetActorForwardVector() * -1;
    //Look = Look.RotateAngleAxis(ControllerRotation, RightController->GetActorRightVector());
	Look = Look.RotateAngleAxis(ControllerRotation, LeftController->GetActorRightVector());

	FPredictProjectilePathParams Params(
		TeleportProjectileRadius,
		Start,
		Look * TeleportProjectileSpeed,
		TeleportSimulationTime,
		ECollisionChannel::ECC_Visibility,
		this
	);

	//Params.DrawDebugType = EDrawDebugTrace::ForOneFrame;
	Params.bTraceComplex = true;

	FPredictProjectilePathResult Result;

	bool bHit = UGameplayStatics::PredictProjectilePath(this, Params, Result);

	if (!bHit) return false;

	for (FPredictProjectilePathPointData PointData : Result.PathData)
	{
		OutPath.Add(PointData.Location);
	}

	FNavLocation NavLocation;

	bool bOnNavMesh = UNavigationSystemV1::GetCurrent(GetWorld())->ProjectPointToNavigation(
		Result.HitResult.Location,
		NavLocation,
		TeleportProjectionExtent
	);

	if (!bOnNavMesh) return false;

	OutLocation = NavLocation.Location;

	return bHit && bOnNavMesh;
}

void AVRShooterCharacter::UpdateDestinationMarker()
{
	TArray<FVector> Path;
	FVector Location;
	bool bHasDestination = FindTeleportDestination(Path, Location);

	if (bHasDestination)
	{
		DestinationMarker->SetVisibility(true);
		DestinationMarker->SetWorldLocation(Location);

		DrawTeleportPath(Path);
	}
	else
	{
		DestinationMarker->SetVisibility(false);

		TArray<FVector> EmptyPath;
		DrawTeleportPath(EmptyPath);
	}
}

FVector2D AVRShooterCharacter::GetBlinkerCenter()
{
	FVector MovementDirection = GetVelocity().GetSafeNormal();

	if (MovementDirection.IsNearlyZero())
	{
		return FVector2D(0.5, 0.5);
	}

	FVector WorldStationaryLocation;

	if (FVector::DotProduct(Camera->GetForwardVector(), MovementDirection) > 0)
	{
		// Moving forwards
		WorldStationaryLocation = Camera->GetComponentLocation() + MovementDirection * 1000;
	}
	else
	{
		// Moving backwards
		WorldStationaryLocation = Camera->GetComponentLocation() - MovementDirection * 1000;
	}

	PlayerController = PlayerController == nullptr ? Cast<APlayerController>(Controller) : PlayerController;

	FVector2D ScreenStationaryLocation;

	if (PlayerController)
	{
		PlayerController->ProjectWorldLocationToScreen(WorldStationaryLocation, ScreenStationaryLocation);

		int32 SizeX, SizeY;
		PlayerController->GetViewportSize(SizeX, SizeY);

		ScreenStationaryLocation.X /= SizeX;
		ScreenStationaryLocation.Y /= SizeY;
	}
	else
	{
		return FVector2D(0.5, 0.5);
	}

	return ScreenStationaryLocation;
}

void AVRShooterCharacter::UpdateBlinkers()
{
	if (RadiusVsVelocity == nullptr) return;

	float Speed = GetVelocity().Size();
	float Radius = RadiusVsVelocity->GetFloatValue(Speed);

	if (BlinkerMaterialInstance)
	{
		BlinkerMaterialInstance->SetScalarParameterValue(TEXT("Radius"), Radius);

		FVector2D Center = GetBlinkerCenter();

		BlinkerMaterialInstance->SetVectorParameterValue(TEXT("Center"), FLinearColor(Center.X, Center.Y, 0));
	}
}

void AVRShooterCharacter::DrawTeleportPath(const TArray<FVector>& Path)
{
	UpdateSpline(Path);

	for (USplineMeshComponent* SplineMesh : TeleportPathMeshPool)
	{
		SplineMesh->SetVisibility(false);
	}

	int32 SegmentNum = Path.Num() - 1;

	for (int32 i = 0; i < SegmentNum; ++i)
	{
		if (TeleportPathMeshPool.Num() <= i)
		{
			USplineMeshComponent* SplineMesh = NewObject<USplineMeshComponent>(this);
			SplineMesh->SetMobility(EComponentMobility::Movable);
			SplineMesh->AttachToComponent(TeleportPath, FAttachmentTransformRules::KeepRelativeTransform);
			SplineMesh->SetStaticMesh(TeleportArchMesh);
			SplineMesh->SetMaterial(0, TeleportArchMaterial);
			SplineMesh->RegisterComponent();

			TeleportPathMeshPool.Add(SplineMesh);
		}

		USplineMeshComponent* SplineMesh = TeleportPathMeshPool[i];
		SplineMesh->SetVisibility(true);

		FVector StartPos, StartTangent, EndPos, EndTangent;

		TeleportPath->GetLocalLocationAndTangentAtSplinePoint(i, StartPos, StartTangent);
		TeleportPath->GetLocalLocationAndTangentAtSplinePoint(i + 1, EndPos, EndTangent);

		SplineMesh->SetStartAndEnd(StartPos, StartTangent, EndPos, EndTangent, true);
	}
}

void AVRShooterCharacter::UpdateSpline(const TArray<FVector>& Path)
{
	TeleportPath->ClearSplinePoints(false);

	for (int32 i = 0; i < Path.Num(); ++i)
	{
		FVector LocalPosition = TeleportPath->GetComponentTransform().InverseTransformPosition(Path[i]);
		FSplinePoint Point(i, LocalPosition, ESplinePointType::Curve);
		TeleportPath->AddPoint(Point, false);
	}

	TeleportPath->UpdateSpline();
}

void AVRShooterCharacter::IncrementOverlappedItemCount(int8 Amount)
{
	if (OverlappedItemCount + Amount <= 0)
	{
		OverlappedItemCount = 0;
		bShouldTraceForItems = false;
	}
	else
	{
		OverlappedItemCount += Amount;
		bShouldTraceForItems = true;
	}
}

void AVRShooterCharacter::TraceForItems()
{
	if (bShouldTraceForItems)
	{
		if (Combat)
		{
			FHitResult ItemTraceResult;
			Combat->TraceUnderCrosshairs(ItemTraceResult);

			if (ItemTraceResult.bBlockingHit)
			{
				Combat->TraceHitItem = Cast<AItem>(ItemTraceResult.GetActor());
				const auto TraceHitWeapon = Cast<AWeapon>(Combat->TraceHitItem);

				if (TraceHitWeapon)
				{
					if (Combat->HighlightedSlot == -1)
					{
						// Not currently highlight a slot; highlight one
						Combat->HighlightInventorySlot();
					}
				} 
				else
				{
					// Is a slot being highlighted?
					if (Combat->HighlightedSlot != -1)
					{
						// Unhighlight the slot
						Combat->UnHighlightInventorySlot();
					}
				}

				if (Combat->TraceHitItem &&
					Combat->TraceHitItem->GetItemState() == EItemState::EIS_EquipInterping)
				{
					Combat->TraceHitItem = nullptr;
				}

				if (Combat->TraceHitItem &&
					Combat->TraceHitItem->GetPickupWidget() &&
					Combat->TraceHitItem != Combat->EquippedWeapon)
				{
					// Show item's Pickup Widget
					Combat->TraceHitItem->RotateWidgetToPlayer(Camera->GetComponentLocation());
					//TraceHitItem->GetPickupWidget()->SetVisibility(true);
					Combat->TraceHitItem->ShowPickupWidget(true);
					
					if (Combat->Inventory.Num() >= Combat->INVENTORY_CAPACITY)
					{
						// Inventory is full
						Combat->TraceHitItem->SetCharacterInventoryFull(true);
					}
					else
					{
						// Inventory has room
						Combat->TraceHitItem->SetCharacterInventoryFull(false);
					}
				}

				// We hit an AItem last frame
				if (Combat->TraceHitItemLastFrame)
				{
					if (Combat->TraceHitItem != Combat->TraceHitItemLastFrame)
					{
						// We are hitting a different AItem this frame from last frame or AItem is NULL
						//TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(true);
						Combat->TraceHitItemLastFrame->ShowPickupWidget(true);
					} 
				}
				// Store a reference to HitItem for next frame;
				Combat->TraceHitItemLastFrame = Combat->TraceHitItem;
			}
		}
	}
	else if (Combat->TraceHitItemLastFrame)
	{
		// No longer overlapping any items, Item last frame should not show widget
		//TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
		Combat->TraceHitItemLastFrame->ShowPickupWidget(false);
	}
}

FInterpLocation AVRShooterCharacter::GetInterpLoctaion(int32 Index)
{
	if (Index <= InterpLocations.Num())
	{
		return InterpLocations[Index];
	}

	return FInterpLocation();
}

// No longer needed; AItem has GetInterpLocation
//FVector AVRShooterCharacter::GetCameraInterpLocation()
//{
//	const FVector CameraWorldLocation{ Camera->GetComponentLocation() };
//	const FVector CameraForward{ Camera->GetForwardVector() };
//	const FVector CameraUp{ Camera->GetUpVector() };
//
//	// Desired = CameraWorldLocation + Forward * A + Up * B
//	return CameraWorldLocation + 
//		CameraForward * 
//		CameraInterpDistance + 
//		CameraUp * 
//		CameraInterpElevation;
//}

void AVRShooterCharacter::GetPickupItem(AItem* Item)
{
	if (Item->GetEquipSound())
	{
		Item->PlayEquipSound();
	}

	auto Weapon = Cast<AWeapon>(Item);

	if (Weapon && Combat)
	{
		if (Combat->Inventory.Num() < Combat->INVENTORY_CAPACITY)
		{
			// If the first weapon; Equip it!
			if (Combat->Inventory.Num() == 0)
			{
				//Weapon->SetItemState(EItemState::EIS_EquipInterping);
				Combat->EquipWeapon(Weapon, false);
			}
			else
			{
				Weapon->SetItemState(EItemState::EIS_PickedUp);
			}

			Weapon->SetSlotIndex(Combat->Inventory.Num());
			Combat->Inventory.Add(Weapon);
		}
		else // Inventory is ful! swap with EquippedWeapon
		{
			Combat->SwapWeapon(Weapon);
		}
	}

	auto Ammo = Cast<AAmmo>(Item);

	if (Ammo &&
		Combat)
	{
		Combat->PickupAmmo(Ammo);
	}
}

void AVRShooterCharacter::StartPickupSoundTimer()
{
	bShouldPlayPickupSound = false;

	GetWorldTimerManager().SetTimer(
		PickupSoundTimer, 
		this, 
		&AVRShooterCharacter::ResetPickupSoundTimer,
		PickupSoundResetTime
	);
}

void AVRShooterCharacter::ResetPickupSoundTimer()
{
	bShouldPlayPickupSound = true;
}

void AVRShooterCharacter::StartEquipSoundTimer()
{
	bShouldPlayEquipSound = false;

	GetWorldTimerManager().SetTimer(
		EquipSoundTimer,
		this,
		&AVRShooterCharacter::ResetEquipSoundTimer,
		EquipSoundResetTime
	);
}

void AVRShooterCharacter::ResetEquipSoundTimer()
{
	bShouldPlayEquipSound = true;
}

void AVRShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("Forward"), this, &AVRShooterCharacter::MoveForward);
	
	PlayerInputComponent->BindAxis(TEXT("Right"), this, &AVRShooterCharacter::MoveRight);
	
	PlayerInputComponent->BindAction(TEXT("Teleport"), IE_Released, this, &AVRShooterCharacter::BeginTeleport);
	
	PlayerInputComponent->BindAction(TEXT("GripLeft"), IE_Pressed, this, &AVRShooterCharacter::GripLeft);
	PlayerInputComponent->BindAction(TEXT("GripLeft"), IE_Released, this, &AVRShooterCharacter::ReleaseLeft);
	
	PlayerInputComponent->BindAction(TEXT("GripRight"), IE_Pressed, this, &AVRShooterCharacter::GripRight);
	PlayerInputComponent->BindAction(TEXT("GripRight"), IE_Released, this, &AVRShooterCharacter::ReleaseRight);
	
	PlayerInputComponent->BindAction(TEXT("FireButton"), IE_Pressed, this, &AVRShooterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction(TEXT("FireButton"), IE_Released, this, &AVRShooterCharacter::FireButtonReleased);
	
	PlayerInputComponent->BindAction(TEXT("Reload"), IE_Pressed, this, &AVRShooterCharacter::ReloadButtonPressed);
	
	PlayerInputComponent->BindAction(TEXT("Select"), IE_Pressed, this, &AVRShooterCharacter::SelectButtonPressed);
	PlayerInputComponent->BindAction(TEXT("Select"), IE_Released, this, &AVRShooterCharacter::SelectButtonReleased);

	PlayerInputComponent->BindAction(TEXT("FKey"), IE_Pressed, this, &AVRShooterCharacter::FKeyPressed);

	PlayerInputComponent->BindAction(TEXT("1Key"), IE_Pressed, this, &AVRShooterCharacter::OneKeyPressed);

	PlayerInputComponent->BindAction(TEXT("2Key"), IE_Pressed, this, &AVRShooterCharacter::TwoKeyPressed);
	
	PlayerInputComponent->BindAction(TEXT("3Key"), IE_Pressed, this, &AVRShooterCharacter::ThreeKeyPressed);
	
	PlayerInputComponent->BindAction(TEXT("4Key"), IE_Pressed, this, &AVRShooterCharacter::FourKeyPressed);

	PlayerInputComponent->BindAction(TEXT("5Key"), IE_Pressed, this, &AVRShooterCharacter::FiveKeyPressed);
}

void AVRShooterCharacter::MoveForward(float value)
{
	AddMovementInput(value * Camera->GetForwardVector());
}

void AVRShooterCharacter::MoveRight(float value)
{
	AddMovementInput(value * Camera->GetRightVector());
}

void AVRShooterCharacter::BeginTeleport()
{
	StartFade(0, 1);

	FTimerHandle Handle;

	GetWorldTimerManager().SetTimer(
		Handle,
		this,
		&AVRShooterCharacter::FinishTeleport,
		TeleportFadeTime
	);
}

void AVRShooterCharacter::StartFade(float FromAlpha, float ToAlpha)
{
	PlayerController = PlayerController == nullptr ? Cast<APlayerController>(Controller) : PlayerController;

	if (PlayerController)
	{
		PlayerController->PlayerCameraManager->StartCameraFade(
			FromAlpha,
			ToAlpha,
			TeleportFadeTime,
			FLinearColor::Black
		);
	}
}

void AVRShooterCharacter::FinishTeleport()
{
	FVector Destination = DestinationMarker->GetComponentLocation();
	Destination += GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * GetActorUpVector();

	SetActorLocation(Destination);

	StartFade(1, 0);
}

void AVRShooterCharacter::FireButtonPressed()
{
	if (Combat)
	{
		Combat->FireButtonPressed();
		bFireButtonPressed = true;
	}
}

void AVRShooterCharacter::UpdateKillCounter(int32 KillsToAdd)
{
	if (Combat)
	{
		Combat->UpdateKillCounter(KillsToAdd);
	}
}

void AVRShooterCharacter::FireButtonReleased()
{
	if (Combat)
	{
		Combat->FireButtonReleased();
		bFireButtonPressed = false;
	}
}

void AVRShooterCharacter::ReloadButtonPressed()
{
	if (Combat)
	{
		// We have no ReloadMontage, simply call FinishReloading()
		Combat->ReloadWeapon();
		//Combat->FinishReloading();
	}
}

void AVRShooterCharacter::SelectButtonPressed()
{
	if (Combat &&
		Combat->CombatState != ECombatState::ECS_Unoccupied) return;

	if (Combat && 
		Combat->TraceHitItem &&
		Combat->TraceHitItem != Combat->EquippedWeapon
		)
	{
		Combat->TraceHitItem->StartItemCurve(this, true);
		Combat->TraceHitItem = nullptr;
	}
}

void AVRShooterCharacter::SelectButtonReleased()
{

}

void AVRShooterCharacter::FKeyPressed()
{
	if (Combat &&
		Combat->EquippedWeapon)
	{
		if (Combat->EquippedWeapon->GetSlotIndex() == 0) return;
		Combat->ExchangeInventoryItems(Combat->EquippedWeapon->GetSlotIndex(), 0);
	}
}

void AVRShooterCharacter::OneKeyPressed()
{
	if (Combat &&
		Combat->EquippedWeapon)
	{
		if (Combat->EquippedWeapon->GetSlotIndex() == 1) return;
		Combat->ExchangeInventoryItems(Combat->EquippedWeapon->GetSlotIndex(), 1);
	}
}

void AVRShooterCharacter::TwoKeyPressed()
{
	if (Combat &&
		Combat->EquippedWeapon)
	{
		if (Combat->EquippedWeapon->GetSlotIndex() == 2) return;
		Combat->ExchangeInventoryItems(Combat->EquippedWeapon->GetSlotIndex(), 2);
	}
}

void AVRShooterCharacter::ThreeKeyPressed()
{
	if (Combat &&
		Combat->EquippedWeapon)
	{
		if (Combat->EquippedWeapon->GetSlotIndex() == 3) return;
		Combat->ExchangeInventoryItems(Combat->EquippedWeapon->GetSlotIndex(), 3);
	}
}

void AVRShooterCharacter::FourKeyPressed()
{
	if (Combat &&
		Combat->EquippedWeapon)
	{
		if (Combat->EquippedWeapon->GetSlotIndex() == 4) return;
		Combat->ExchangeInventoryItems(Combat->EquippedWeapon->GetSlotIndex(), 4);
	}
}

void AVRShooterCharacter::FiveKeyPressed()
{
	if (Combat &&
		Combat->EquippedWeapon)
	{
		if (Combat->EquippedWeapon->GetSlotIndex() == 5) return;
		Combat->ExchangeInventoryItems(Combat->EquippedWeapon->GetSlotIndex(), 5);
	}
}

float AVRShooterCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (Combat)
	{
		if (Combat->Health - DamageAmount <= 0.f)
		{
			Combat->Health = 0.f;
			Die();
			
			auto EnemyController = Cast<AEnemyController>(EventInstigator);

			if (EnemyController)
			{
				EnemyController->GetBlackboardComponent()->SetValueAsBool(
					FName(TEXT("PlayerDead")), 
					true
				);
			}
		}
		else
		{
			PlayCameraShake();
			Combat->Health -= DamageAmount;
		}
	}	
	return DamageAmount;
}

void AVRShooterCharacter::PlayCameraShake()
{
	if (TakeDamageCameraShake)
	{
		UGameplayStatics::PlayWorldCameraShake(
			GetWorld(),
			TakeDamageCameraShake,
			GetActorLocation(),
			1000.f,
			1000.f
		);
	}
}

void AVRShooterCharacter::Die()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->Montage_Play(DeathMontage);
	}
	else
	{
		FinishDeath();
	}
}

void AVRShooterCharacter::FinishDeath()
{
	GetMesh()->bPauseAnims = true;

	//APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	PlayerController = PlayerController == nullptr ? Cast<APlayerController>(Controller) : PlayerController;

	if (PlayerController)
	{
		DisableInput(PlayerController);
	}
}

USoundCue* AVRShooterCharacter::GetMeleeImpactSound()
{
	if (Combat)
	{
		if (Combat->GetMeleeImpactSound())
		{
			return Combat->GetMeleeImpactSound();
		}
	}
	return nullptr;
}

UParticleSystem* AVRShooterCharacter::GetBloodParticles()
{
	if (Combat)
	{
		if (Combat->GetBloodParticles())
		{
			return Combat->GetBloodParticles();
		}
	}
	return nullptr;
}

UNiagaraSystem* AVRShooterCharacter::GetBloodNiagara()
{
	if (Combat)
	{
		if (Combat->GetBloodNiagara())
		{
			return Combat->GetBloodNiagara();
		}
	}
	return nullptr;
}

