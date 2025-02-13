#include "ReverseControllItem.h"
#include "SpartaCharacter.h"

AReverseControllItem::AReverseControllItem()
{
	ReverseControllDuration = 10.0f;
	ItemType = "ReverseControll";
}

void AReverseControllItem::ActivateItem(AActor* Activator)
{
	Super::ActivateItem(Activator);

	if (Activator && Activator->ActorHasTag("Player"))
	{
		if (ASpartaCharacter* PlayerCharacter = Cast<ASpartaCharacter>(Activator))
		{

			PlayerCharacter->ReverseState();

			GetWorld()->GetTimerManager().SetTimer(
				ReverseControllTimerHandle,
				[this, Activator]() {
					Normalize(Activator);
				},
				ReverseControllDuration,
				false
			);

			DestroyItem();
		}
	}
}

void AReverseControllItem::Normalize(AActor* Activator)
{
	if (Activator && Activator->ActorHasTag("Player"))
	{
		if (ASpartaCharacter* PlayerCharacter = Cast<ASpartaCharacter>(Activator))
		{
			PlayerCharacter->EndReverseState();
		}
	}
}
