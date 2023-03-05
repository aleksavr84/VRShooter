#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WidgetActor.generated.h"

UCLASS()
class VRSHOOTER_API AWidgetActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AWidgetActor();

protected:
	virtual void BeginPlay() override;

private:
	// WidgetComponent
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Initialization, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* WidgetComp;

	// Reference to the WidgetBlueprint class
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Initialization, meta = (AllowPrivateAccess = "True"))
	TSubclassOf<class UUserWidget> WidgetBlueprintClass;

	// Variable to hold the HitNumberWidget after created it
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Initialization, meta = (AllowPrivateAccess = "True"))
	UUserWidget* Widget;

	class AVRShooterCharacter* ShooterCharacter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Initialization, meta = (AllowPrivateAccess = "True"))
	float MovementSpeed = 125.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Initialization, meta = (AllowPrivateAccess = "True"))
	FColor SecondaryColor = FColor::Red;

	void RotateWidgetToPlayer();
	void MoveWidgetUpwards(float DeltaTime);

public:	
	virtual void Tick(float DeltaTime) override;
	FORCEINLINE void SetShooterCharacter(AVRShooterCharacter* Character) { ShooterCharacter = Character; }
	void SetTextAndStartAnimation(FString Text, bool bChangeColor);
};
