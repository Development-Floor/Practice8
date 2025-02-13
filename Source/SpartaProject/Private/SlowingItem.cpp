#include "SlowingItem.h"
#include "SpartaCharacter.h"

ASlowingItem::ASlowingItem()
{
	SlowPercent = 50;
	SlowDuration = 10.0f;
	ItemType = "Slowing";
}

void ASlowingItem::ActivateItem(AActor* Activator)
{
	Super::ActivateItem(Activator);

	if (Activator && Activator->ActorHasTag("Player"))
	{
		if (ASpartaCharacter* PlayerCharacter = Cast<ASpartaCharacter>(Activator))
		{
			PlayerCharacter->SlowState(SlowPercent);

			GetWorld()->GetTimerManager().SetTimer(
				SlowTimerHandle,
				[this, Activator]() {
					Normalize(Activator);
				},
				SlowDuration,
				false
			);
		}

		DestroyItem();
	}
}

void ASlowingItem::Normalize(AActor* Activator)
{
	if (this)
	{
		if (Activator && Activator->ActorHasTag("Player"))
		{
			if (ASpartaCharacter* PlayerCharacter = Cast<ASpartaCharacter>(Activator))
			{
				PlayerCharacter->EndSlowState(SlowPercent);
			}
		}
	}
}
