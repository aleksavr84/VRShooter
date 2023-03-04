#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "BulletHitInterface.generated.h"

UINTERFACE(MinimalAPI)
class UBulletHitInterface : public UInterface
{
	GENERATED_BODY()
};

class VRSHOOTER_API IBulletHitInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void BulletHit(FHitResult HitResult, class AVRShooterCharacter* CauserCharacter);
};
