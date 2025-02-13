#pragma once

#include "CoreMinimal.h"
#include "BaseItem.h"
#include "ReverseControllItem.generated.h"

UCLASS()
class SPARTAPROJECT_API AReverseControllItem : public ABaseItem
{
	GENERATED_BODY()
	
public:
	AReverseControllItem();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	float ReverseControllDuration;

	FTimerHandle ReverseControllTimerHandle;

	virtual void ActivateItem(AActor* Activator) override;

	void Normalize(AActor* Activator);
};
