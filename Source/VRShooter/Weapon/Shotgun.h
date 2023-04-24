// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "Shotgun.generated.h"

/**
 * 
 */
UCLASS()
class VRSHOOTER_API AShotgun : public AWeapon
{
	GENERATED_BODY()
	
public:
	void ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector>& HitTargets);

private:
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter");
	uint32 NumberOfPellets = 10;
};
