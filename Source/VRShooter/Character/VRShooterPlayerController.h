#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "VRShooterPlayerController.generated.h"

UCLASS()
class VRSHOOTER_API AVRShooterPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AVRShooterPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
};
