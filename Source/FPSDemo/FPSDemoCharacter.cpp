// Copyright Epic Games, Inc. All Rights Reserved.

#include "FPSDemoCharacter.h"
#include "FPSDemoProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/InputSettings.h"


//////////////////////////////////////////////////////////////////////////
// AFPSDemoCharacter

AFPSDemoCharacter::AFPSDemoCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	TurnRateGamepad = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));

}

void AFPSDemoCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

}

//////////////////////////////////////////////////////////////////////////// Input

void AFPSDemoCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	//冲刺绑定
	PlayerInputComponent->BindAction("Rush",IE_Pressed,this,&AFPSDemoCharacter::StartRush);
	PlayerInputComponent->BindAction("Rush",IE_Released,this,&AFPSDemoCharacter::StopRush);
	//聚焦绑定
	PlayerInputComponent->BindAction("Focus",IE_Pressed,this,&AFPSDemoCharacter::StartFocus);
	PlayerInputComponent->BindAction("Focus",IE_Released,this,&AFPSDemoCharacter::StopFocus);
	

	// Bind fire event
	PlayerInputComponent->BindAction("PrimaryAction", IE_Pressed, this, &AFPSDemoCharacter::OnPrimaryAction);

	// Enable touchscreen input
	EnableTouchscreenMovement(PlayerInputComponent);

	// Bind movement events
	PlayerInputComponent->BindAxis("Move Forward / Backward", this, &AFPSDemoCharacter::MoveForward);
	PlayerInputComponent->BindAxis("Move Right / Left", this, &AFPSDemoCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "Mouse" versions handle devices that provide an absolute delta, such as a mouse.
	// "Gamepad" versions are for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn Right / Left Mouse", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("Look Up / Down Mouse", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn Right / Left Gamepad", this, &AFPSDemoCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("Look Up / Down Gamepad", this, &AFPSDemoCharacter::LookUpAtRate);
}

void AFPSDemoCharacter::OnPrimaryAction()
{
	// Trigger the OnItemUsed Event
	OnUseItem.Broadcast();
}

void AFPSDemoCharacter::BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == true)
	{
		return;
	}
	if ((FingerIndex == TouchItem.FingerIndex) && (TouchItem.bMoved == false))
	{
		OnPrimaryAction();
	}
	TouchItem.bIsPressed = true;
	TouchItem.FingerIndex = FingerIndex;
	TouchItem.Location = Location;
	TouchItem.bMoved = false;
}

void AFPSDemoCharacter::EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == false)
	{
		return;
	}
	TouchItem.bIsPressed = false;
}

void AFPSDemoCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bFocus)
	{
		FirstPersonCameraComponent->SetFieldOfView(60.0f);
	}
	else
	{
		FirstPersonCameraComponent->SetFieldOfView(90.0f);
	}
}

void AFPSDemoCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		if (bRush)
		{
			MovementComponent->MaxWalkSpeed=RushForwardSpeed;
			AddMovementInput(GetActorForwardVector(),Value);
			
		}
		else
		{
			MovementComponent->MaxWalkSpeed=DefaultForwardSpeed;
			AddMovementInput(GetActorForwardVector(), Value);
		}
		
		
		
	}
}

void AFPSDemoCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		if (bRush)
		{
			MovementComponent->MaxWalkSpeed=RushRightSpeed;
			AddMovementInput(GetActorRightVector(), Value);
		}
		else
		{
			MovementComponent->MaxWalkSpeed=DefaultRightSpeed;
			AddMovementInput(GetActorRightVector(), Value);
		}
		
	}
}

void AFPSDemoCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void AFPSDemoCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}
void AFPSDemoCharacter::StartRush()
{
	bRush=true;
}
void AFPSDemoCharacter::StopRush()
{
	bRush=false;
}
void AFPSDemoCharacter::StartFocus()
{
	bFocus=true;
}

void AFPSDemoCharacter::StopFocus()
{
	bFocus=false;
}


bool AFPSDemoCharacter::EnableTouchscreenMovement(class UInputComponent* PlayerInputComponent)
{
	if (FPlatformMisc::SupportsTouchInput() || GetDefault<UInputSettings>()->bUseMouseForTouch)
	{
		PlayerInputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AFPSDemoCharacter::BeginTouch);
		PlayerInputComponent->BindTouch(EInputEvent::IE_Released, this, &AFPSDemoCharacter::EndTouch);

		return true;
	}
	
	return false;
}
