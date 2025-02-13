#pragma once

#include "CoreMinimal.h"
#include "BaseItem.h"
#include "SlowingItem.generated.h"

UCLASS()
class SPARTAPROJECT_API ASlowingItem : public ABaseItem
{
	GENERATED_BODY()
	
public:
	ASlowingItem();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 SlowPercent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	float SlowDuration;

	FTimerHandle SlowTimerHandle;

	virtual void ActivateItem(AActor* Activator) override;

	void Normalize(AActor* Activator);
};
