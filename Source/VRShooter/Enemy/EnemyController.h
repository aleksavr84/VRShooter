#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyController.generated.h"

UCLASS()
class VRSHOOTER_API AEnemyController : public AAIController
{
	GENERATED_BODY()

public:
	AEnemyController();
	virtual void OnPossess(APawn* InPawn) override;

private:
	// Blackboard component for this enemy
	UPROPERTY(BlueprintReadWrite, Category = Initialization, meta = (AllowPrivateAccess = "True"))
	class UBlackboardComponent* BlackboardComponent;
	
	// BehaviorTree compoent for this enemy
	UPROPERTY(BlueprintReadWrite, Category = Initialization, meta = (AllowPrivateAccess = "True"))
	class UBehaviorTreeComponent* BehaviorTreeComponent;

public:
	FORCEINLINE UBlackboardComponent* GetBlackboardComponent() const { return BlackboardComponent; }

};
