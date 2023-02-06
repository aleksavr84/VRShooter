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
#include "Sound/SoundCue.h"
#include "Engine/SkeletalMeshSocket.h"
#include "DrawDebugHelpers.h"
#include "Particles/ParticleSystemComponent.h"

AVRShooterCharacter::AVRShooterCharacter()
{
	bAiming = false;
	CameraDefaultFOV = 0.f;
	CameraZoomedFOV = 60.f;

 	PrimaryActorTick.bCanEverTick = true;

	VRRoot = CreateDefaultSubobject<USceneComponent>(FName("VRRoot"));
	VRRoot->SetupAttachment(GetRootComponent());

	Camera = CreateDefaultSubobject<UCameraComponent>(FName("Camera"));
	Camera->SetupAttachment(VRRoot);

	TeleportPath = CreateDefaultSubobject<USplineComponent>(FName("TeleportPath"));
	TeleportPath->SetupAttachment(VRRoot);

	DestinationMarker = CreateDefaultSubobject<UStaticMeshComponent>(FName("DestinationMarker"));
	DestinationMarker->SetupAttachment(GetRootComponent());

	PostProcessComponent = CreateDefaultSubobject<UPostProcessComponent>(FName("PostProcessComponent"));
	PostProcessComponent->SetupAttachment(GetRootComponent());
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
	}

	if (LeftController && RightController)
	{
		LeftController->PairController(RightController);
	}

	if (Camera)
	{
		CameraDefaultFOV = GetCameraComponent()->FieldOfView;
	}
}

void AVRShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Location corretion in PlayerSpace
	FVector NewCameraOffset = Camera->GetComponentLocation() - GetActorLocation();
	NewCameraOffset.Z = 0;
	AddActorWorldOffset(NewCameraOffset);
	VRRoot->AddWorldOffset(-NewCameraOffset);

	UpdateDestinationMarker();
	UpdateBlinkers();
}


bool AVRShooterCharacter::FindTeleportDestination(TArray<FVector>& OutPath, FVector& OutLocation)
{
	FVector Start = LeftController->GetActorLocation();// RightController->GetActorLocation();
	FVector Look = LeftController->GetActorForwardVector(); // RightController->GetActorForwardVector();
	// Look = Look.RotateAngleAxis(ControllerRotation, RightController->GetActorRightVector());
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
	PlayerInputComponent->BindAction(TEXT("FireButton"), IE_Pressed, this, &AVRShooterCharacter::FireWeapon);
	PlayerInputComponent->BindAction(TEXT("AimingButton"), IE_Pressed, this, &AVRShooterCharacter::AimingButtonPressed);
	PlayerInputComponent->BindAction(TEXT("AimingButton"), IE_Released, this, &AVRShooterCharacter::AimingButtonReleased);
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

void AVRShooterCharacter::FireWeapon()
{
	if (FireSound)
	{
		UGameplayStatics::PlaySound2D(this, FireSound);
	}

	const USkeletalMeshComponent* WeaponMesh = RightController->GetWeaponMesh();
	
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

void AVRShooterCharacter::AimingButtonPressed()
{
	bAiming = true;
	GetCameraComponent()->SetFieldOfView(CameraZoomedFOV);
}

void AVRShooterCharacter::AimingButtonReleased()
{
	bAiming = false;
	GetCameraComponent()->SetFieldOfView(CameraDefaultFOV);
}


