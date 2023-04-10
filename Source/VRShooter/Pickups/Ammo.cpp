#include "Ammo.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"
#include "VRShooter/Character/VRShooterCharacter.h"

AAmmo::AAmmo()
{
	//AmmoMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AmmoMesh"));
	
	//SetRootComponent(GetRootComponent());
	//SetRootComponent(GetCollisionBox());
	/*GetCollisionBox()->SetupAttachment(GetRootComponent());
	GetPickupWidget()->SetupAttachment(GetRootComponent());
	GetAreaSphere()->SetupAttachment(GetRootComponent());*/

	//AmmoCollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AmmoColisionSphere"));
	////AmmoCollisionSphere->SetupAttachment(GetRootComponent());
	//AmmoCollisionSphere->SetSphereRadius(50.f);
}

void AAmmo::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AAmmo::BeginPlay()
{
	Super::BeginPlay();
	//GetCollisionBox()->OnComponentBeginOverlap.AddDynamic(this, &AAmmo::AmmoSphereOverlap);
	//AmmoCollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AAmmo::AmmoSphereOverlap);
}

//void AAmmo::AmmoSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bfromSweep, const FHitResult& SweepResult)
//{
//	if (OtherActor)
//	{
//		auto OverlappedCharacter = Cast<AVRShooterCharacter>(OtherActor);
//
//		if (OverlappedCharacter)
//		{
//			StartItemCurve(OverlappedCharacter);
//			GetCollisionBox()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
//			//AmmoCollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
//		}
//	}
//}

void AAmmo::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bfromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bfromSweep, SweepResult);

	if (OtherActor)
	{
		auto OverlappedCharacter = Cast<AVRShooterCharacter>(OtherActor);

		if (OverlappedCharacter)
		{
			StartItemCurve(OverlappedCharacter);
			GetCollisionBox()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			//AmmoCollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void AAmmo::SetItemProperties(EItemState State)
{
	Super::SetItemProperties(State);

	switch (State)
	{
	case EItemState::EIS_Pickup:
		// Set Mesh properties
		GetValidMeshComponent()->SetSimulatePhysics(false);
		GetValidMeshComponent()->SetEnableGravity(false);
		GetValidMeshComponent()->SetVisibility(true);
		GetValidMeshComponent()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		GetValidMeshComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			
		break;

	case EItemState::EIS_Equipped:
		// Set Mesh properties
		GetValidMeshComponent()->SetSimulatePhysics(false);
		GetValidMeshComponent()->SetEnableGravity(false);
		GetValidMeshComponent()->SetVisibility(true);
		GetValidMeshComponent()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		GetValidMeshComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
		break;

	case EItemState::EIS_Falling:
		// Set Mesh properties
		GetValidMeshComponent()->SetSimulatePhysics(true);
		GetValidMeshComponent()->SetEnableGravity(true);
		GetValidMeshComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		GetValidMeshComponent()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		GetValidMeshComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
				
		break;

	case EItemState::EIS_EquipInterping:		
		// Set Mesh properties
		GetValidMeshComponent()->SetSimulatePhysics(false);
		GetValidMeshComponent()->SetEnableGravity(false);
		GetValidMeshComponent()->SetVisibility(true);
		GetValidMeshComponent()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		GetValidMeshComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		break;
	}
}

