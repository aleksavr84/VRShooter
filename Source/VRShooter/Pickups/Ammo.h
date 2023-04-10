#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "VRShooter/Weapon/Types.h"
#include "Ammo.generated.h"

UCLASS()
class VRSHOOTER_API AAmmo : public AItem
{
	GENERATED_BODY()
	
public:
	AAmmo();

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	// Override of SetItemProperties so we can set AmmoMesh properties
	virtual void SetItemProperties(EItemState State) override;

	//UFUNCTION()
	//void AmmoSphereOverlap(
	//	UPrimitiveComponent* OverlappedComponent,
	//	AActor* OtherActor,
	//	UPrimitiveComponent* OtherComp,
	//	int32 OtherBodyIndex,
	//	bool bfromSweep,
	//	const FHitResult& SweepResult
	//	);

	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bfromSweep,
		const FHitResult& SweepResult
	) override;

private:
	//// Mesh for the Ammo pickup
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Ammo, meta = (AllowPrivateAccess = "true"))
	//UStaticMeshComponent* AmmoMesh;

	// Ammo type for the ammo
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Ammo, meta = (AllowPrivateAccess = "true"))
	EAmmoType AmmoType;

	// Texture for the ammo icon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Ammo, meta = (AllowPrivateAccess = "true"))
	UTexture2D* AmmoIconTexture;

	//// Overlap Sphere for picking up the ammo
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Ammo, meta = (AllowPrivateAccess = "true"))
	//class USphereComponent* AmmoCollisionSphere;

public:
	/*FORCEINLINE UStaticMeshComponent* GetAmmoMesh() const { return AmmoMesh; }*/
	FORCEINLINE EAmmoType GetAmmoType() const { return AmmoType; }
};
