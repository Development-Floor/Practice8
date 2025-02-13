#include "SpartaCharacter.h"
#include "SpartaPlayerController.h"
#include "SpartaGameState.h"
#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/TextBlock.h"

ASpartaCharacter::ASpartaCharacter()
{
 	PrimaryActorTick.bCanEverTick = false;

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->TargetArmLength = 300.0f;
	SpringArmComp->bUsePawnControlRotation = true;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
	CameraComp->bUsePawnControlRotation = false;

	CharacterStateWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("CharacterState"));
	CharacterStateWidget->SetupAttachment(GetMesh());
	CharacterStateWidget->SetWidgetSpace(EWidgetSpace::Screen);

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(GetMesh());
	OverheadWidget->SetWidgetSpace(EWidgetSpace::Screen);

	NormalSpeed = 600.0f;
	SprintSpeedMultiplier = 1.7f;
	SprintSpeed = NormalSpeed * SprintSpeedMultiplier;

	GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;

	MaxHealth = 100.0f;
	Health = MaxHealth;
	SlowNormalSpeed = NormalSpeed;
	IsSprint = false;
	IsReverse = false;
	SlowCount = 0;
	ReverseCount = 0;
}

void ASpartaCharacter::BeginPlay()
{
	Super::BeginPlay();
	UpdateOverheadHP();
}

void ASpartaCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (ASpartaPlayerController* PlayerController = Cast<ASpartaPlayerController>(GetController()))
		{
			if (PlayerController->MoveAction)
			{
				EnhancedInput->BindAction(
					PlayerController->MoveAction,
					ETriggerEvent::Triggered,
					this,
					&ASpartaCharacter::Move
				);
			}

			if (PlayerController->JumpAction)
			{
				EnhancedInput->BindAction(
					PlayerController->JumpAction,
					ETriggerEvent::Triggered,
					this,
					&ASpartaCharacter::StartJump
				);

				EnhancedInput->BindAction(
					PlayerController->MoveAction,
					ETriggerEvent::Completed,
					this,
					&ASpartaCharacter::StopJump
				);
			}

			if (PlayerController->LookAction)
			{
				EnhancedInput->BindAction(
					PlayerController->LookAction,
					ETriggerEvent::Triggered,
					this,
					&ASpartaCharacter::Look
				);
			}

			if (PlayerController->SprintAction)
			{
				EnhancedInput->BindAction(
					PlayerController->SprintAction,
					ETriggerEvent::Triggered,
					this,
					&ASpartaCharacter::StartSprint
				);

				EnhancedInput->BindAction(
					PlayerController->SprintAction,
					ETriggerEvent::Completed,
					this,
					&ASpartaCharacter::StopSprint
				);
			}
		}
	}
}

void ASpartaCharacter::Move(const FInputActionValue& value)
{
	if (!Controller) return;

	FVector2D MoveInput = value.Get<FVector2D>();

	if (IsReverse)
	{
		MoveInput *= -1;
	}

	if (!FMath::IsNearlyZero(MoveInput.X))
	{
		AddMovementInput(GetActorForwardVector(), MoveInput.X);
	}

	if (!FMath::IsNearlyZero(MoveInput.Y))
	{
		AddMovementInput(GetActorRightVector(), MoveInput.Y);
	}
}

void ASpartaCharacter::StartJump(const FInputActionValue& value)
{
	if (value.Get<bool>())
	{
		Jump();
	}
}

void ASpartaCharacter::StopJump(const FInputActionValue& value)
{
	if (!value.Get<bool>())
	{
		StopJumping();
	}
}

void ASpartaCharacter::Look(const FInputActionValue& value)
{
	FVector2D LookInput = value.Get<FVector2D>();

	AddControllerYawInput(LookInput.X);
	AddControllerPitchInput(LookInput.Y);
}

void ASpartaCharacter::StartSprint(const FInputActionValue& value)
{
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
		IsSprint = true;
	}
}

void ASpartaCharacter::StopSprint(const FInputActionValue& value)
{
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;
		IsSprint = false;
	}
}

float ASpartaCharacter::GetHealth() const
{
	return Health;;
}

void ASpartaCharacter::AddHealth(float Amount)
{
	Health = FMath::Clamp(Health + Amount, 0.0f, MaxHealth);
	UpdateOverheadHP();
}

float ASpartaCharacter::TakeDamage(
	float DamageAmount,
	struct FDamageEvent const& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser)
{
	// DamageAmount 방어력 적용 이전 데미지
	// ActualDamage 방어력 등을 적용한 실제 데미지
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	Health = FMath::Clamp(Health - DamageAmount, 0.0f, MaxHealth);
	UpdateOverheadHP();

	if (Health <= 0.0f)
	{
		OnDeath();
	}

	return ActualDamage;
}

void ASpartaCharacter::OnDeath()
{
	ASpartaGameState* SpartaGameState = GetWorld() ? GetWorld()->GetGameState<ASpartaGameState>() : nullptr;
	if (SpartaGameState)
	{
		SpartaGameState->OnGameOver();
	}
}

void ASpartaCharacter::UpdateCharacterState()
{
	if (!CharacterStateWidget) return;

	UUserWidget* CharacterStateWidgetInstance = CharacterStateWidget->GetUserWidgetObject();
	if (!CharacterStateWidgetInstance) return;

	if (UTextBlock* CharacterStateText = Cast<UTextBlock>(CharacterStateWidgetInstance->GetWidgetFromName(TEXT("CharacterState"))))
	{
		//// 1. GetText()로 현재 텍스트 가져오기
		//FText CurrentText = CharacterStateText->GetText();

		//// 2. FText에서 FString으로 변환
		//FString CurrentString = CurrentText.ToString();

		//// 3. 문자열 수정 (예: "원래 텍스트 수정" -> "수정된 텍스트")
		//CurrentString = CurrentString + TEXT(" slow");

		//// 4. 수정된 FString을 FText로 변환
		//FText NewText = FText::FromString(CurrentString);

		//// 5. SetText로 수정된 텍스트 설정
		//CharacterStateText->SetText(NewText);

		FString PrintString;

		for (FString& State : States)
		{
			PrintString = PrintString + State + TEXT(" ");
		}

		FText NewText = FText::FromString(PrintString);

		CharacterStateText->SetText(NewText);
	}
}

void ASpartaCharacter::UpdateOverheadHP()
{
	if (!OverheadWidget) return;

	UUserWidget* OverheadWidgetInstance = OverheadWidget->GetUserWidgetObject();
	if (!OverheadWidgetInstance) return;

	if (UTextBlock* HPText = Cast<UTextBlock>(OverheadWidgetInstance->GetWidgetFromName(TEXT("OverHeadHP"))))
	{
		HPText->SetText(FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), Health, MaxHealth)));
	}
}

void ASpartaCharacter::SlowState(int32 SlowPercent)
{
	States.Add("slow");
	SlowCount++;
	NormalSpeed = NormalSpeed * (100 - SlowPercent) / 100;
	SprintSpeed = NormalSpeed * SprintSpeedMultiplier;
	GetCharacterMovement()->MaxWalkSpeed = IsSprint ? SprintSpeed : NormalSpeed;
	UpdateCharacterState();
}

void ASpartaCharacter::EndSlowState(int32 SlowPercent)
{
	if (SlowCount <= 0 || States.Find("slow") == INDEX_NONE)
	{
		return;
	}

	States.RemoveAt(States.Find("slow"));
	SlowCount--;
	NormalSpeed = NormalSpeed * 100 / (100 - SlowPercent);
	SprintSpeed = NormalSpeed * SprintSpeedMultiplier;

	// 소수 연산이기 때문에
	if (SlowCount == 0)
	{
		NormalSpeed = SlowNormalSpeed;
	}

	GetCharacterMovement()->MaxWalkSpeed = IsSprint ? SprintSpeed : NormalSpeed;
	GetCharacterMovement()->MaxWalkSpeed = IsSprint ? SprintSpeed : NormalSpeed;

	UpdateCharacterState();
}

void ASpartaCharacter::ReverseState()
{
	if (States.Find("reverse") != INDEX_NONE)
	{
		States.RemoveAt(States.Find("reverse"));
	}

	IsReverse = true;
	ReverseCount++;
	States.Add("reverse");
	UpdateCharacterState();
}

void ASpartaCharacter::EndReverseState()
{
	if (IsReverse && ReverseCount > 0)
	{
		ReverseCount--;
	}

	if (ReverseCount == 0 && States.Find("reverse") != INDEX_NONE)
	{
		States.RemoveAt(States.Find("reverse"));
		IsReverse = false;
		UpdateCharacterState();
	}
}
