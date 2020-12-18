// Copyright Epic Games, Inc. All Rights Reserved.

#include "FreeMovementCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"

// include debug helpers
#include "DrawDebugHelpers.h"

//////////////////////////////////////////////////////////////////////////
// AFreeMovementCharacter

AFreeMovementCharacter::AFreeMovementCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->bEnableCameraRotationLag = true;
	CameraBoom->CameraLagSpeed = 10.f;
	CameraBoom->CameraRotationLagSpeed = 20.f;
	CameraBoom->CameraLagMaxDistance = 0.f;
	CameraBoom->CameraLagMaxDistance = 50.f;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
	DefaultFOV = 90.f;
	FollowCamera->FieldOfView = DefaultFOV;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)

	Scale = GetActorTransform().GetScale3D().Z;

	IKFootTraceDistance = Scale * GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	IKArmTraceDistance = 60.f;

	BaseWalkSpeed = 600.f;
	SprintWalkSpeed = 1200.f;

	LongFallDistance = 500.f;

	FallingGravityScale = 4.5f;

	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
}

//////////////////////////////////////////////////////////////////////////
// Input

void AFreeMovementCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	//PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AFreeMovementCharacter::OnJumpPressed);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &AFreeMovementCharacter::OnJumpReleased);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AFreeMovementCharacter::SprintStart);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AFreeMovementCharacter::SprintStop);

	PlayerInputComponent->BindAxis("MoveForward", this, &AFreeMovementCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AFreeMovementCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AFreeMovementCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AFreeMovementCharacter::LookUpAtRate);
}

void AFreeMovementCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (GravityCurveFloat)
	{
		FOnTimelineFloat JumpProgressFunction;
		JumpProgressFunction.BindUFunction(this, "GravityTimelineHandler");

		GravityCurveTimeline.AddInterpFloat(GravityCurveFloat, JumpProgressFunction);
	}
}

void AFreeMovementCharacter::Tick(float DeltaTime)
{
	IsFallingCheck();
	RotateHead(DeltaTime);
	RotateTorso(DeltaTime);
	UpdateFootIK(DeltaTime);
	UpdateHandIK();

	//UpperBodyCheck();
	//LowerBodyCheck();
	
	
	if (GetMovementComponent()->IsFalling())
	{
		//UE_LOG(LogTemp, Warning, TEXT("In Air"));
		if (GetVelocity().Z < 0.f)
		{
			//GetCharacterMovement()->GravityScale = 3.f;
			//UE_LOG(LogTemp, Warning, TEXT("Falling %f"), GetCharacterMovement()->GetGravityZ());
			UE_LOG(LogTemp, Warning, TEXT("Falling %f"), GetVelocity().Z);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Rising"));
			//GetCharacterMovement()->GravityScale = 1.f;
		}
	}
	else 
	{
		//GetCharacterMovement()->GravityScale = 1.5f;
		DistanceFallen = 0.f;
	}
	
	Super::Tick(DeltaTime);
}

void AFreeMovementCharacter::OnJumpPressed()
{
	Super::Jump();

	UE_LOG(LogTemp, Warning, TEXT("Start Jump"));

	if(GetMovementComponent()->IsFalling())
	{
		//WallRunCheck();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Vault Check"));
		//LowerBodyCheck();
	}

	GravityCurveTimeline.PlayFromStart();
}

void AFreeMovementCharacter::OnJumpReleased()
{
	UE_LOG(LogTemp, Warning, TEXT("Stop Jump"));

	GetCharacterMovement()->GravityScale = FallingGravityScale;
	GravityCurveTimeline.Stop();

	Super::StopJumping();
}

void AFreeMovementCharacter::Landed(const FHitResult& Hit)
{
	//Player has landed on the ground return gravity to normal
	GetCharacterMovement()->GravityScale = 1.f;

	Super::Landed(Hit);
}

void AFreeMovementCharacter::IsFallingCheck()
{
	//Check if the character is falling down, used for checking if the player didn't jump but walked off the edge
	if (GetCharacterMovement()->IsFalling() && (GetVelocity().Z < 0.f))
	{
		GetCharacterMovement()->GravityScale = FallingGravityScale;
	}
}

void AFreeMovementCharacter::WallRunCheck()
{
	UE_LOG(LogTemp, Warning, TEXT("Wall Run Check"));
}

void AFreeMovementCharacter::SprintStart()
{
	GetCharacterMovement()->MaxWalkSpeed = SprintWalkSpeed;
	//CameraBoom->bEnableCameraLag = false;
	//CameraBoom->CameraLagSpeed = 0.f;
	//FollowCamera->FieldOfView = DefaultFOV * 1.2f;
	//UE_LOG(LogTemp, Warning, TEXT("MaxWalkSpeed = %f"), GetCharacterMovement()->MaxWalkSpeed);
}

void AFreeMovementCharacter::SprintStop()
{
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	//CameraBoom->bEnableCameraLag = true;
	//CameraBoom->CameraLagSpeed = 10.f;
	//FollowCamera->FieldOfView = DefaultFOV;
	//UE_LOG(LogTemp, Warning, TEXT("MaxWalkSpeed = %f"), GetCharacterMovement()->MaxWalkSpeed);
}

void AFreeMovementCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AFreeMovementCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AFreeMovementCharacter::MoveForward(float Value)
{
	MoveForwardInput = Value;
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AFreeMovementCharacter::MoveRight(float Value)
{
	MoveRightInput = Value;
	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void AFreeMovementCharacter::IKFootTrace(FName SocketName, float TraceDistance, FVector& Out_HitLocation, float& Out_FootTraceOffset)
{
	FHitResult hit;

	FVector traceStart = FVector(GetMesh()->GetSocketLocation(SocketName).X, GetMesh()->GetSocketLocation(SocketName).Y, GetActorLocation().Z);
	FVector traceEnd = FVector(GetMesh()->GetSocketLocation(SocketName).X, GetMesh()->GetSocketLocation(SocketName).Y, (GetMesh()->GetSocketLocation(SocketName).Z - TraceDistance));

	FCollisionQueryParams queryParam;
	queryParam.AddIgnoredActor(GetOwner());

	/* //Draw trace
	const FName TraceTag("MyTraceTag");
	GetWorld()->DebugDrawTraceTag = TraceTag;
	queryParam.TraceTag = TraceTag;
	*/

	if (GetWorld()->LineTraceSingleByChannel(hit, traceStart, traceEnd, ECollisionChannel::ECC_Visibility, queryParam))
	{
		Out_HitLocation = hit.Location;
		Out_FootTraceOffset = (hit.Location - GetMesh()->GetComponentLocation()).Z - IKHipOffset;
	}
	else
	{
		Out_HitLocation = FVector(0.f, 0.f, 0.f);
		Out_FootTraceOffset = 0.f;
	}
}

void AFreeMovementCharacter::IKHandTrace(FName SocketName, float TraceDistance, float HandOffset, FVector& Out_HitLocation, bool& Out_HitWall)
{
	FHitResult hit;

	FVector traceStart = GetMesh()->GetSocketLocation(SocketName);
	FVector traceEnd =  GetMesh()->GetSocketLocation(SocketName) + (GetActorForwardVector() * TraceDistance) + FVector(0.f, HandOffset, 0.f);

	FCollisionQueryParams queryParam;
	queryParam.AddIgnoredActor(GetOwner());

	/*
	//Draw trace
	const FName TraceTag("MyTraceTag");
	GetWorld()->DebugDrawTraceTag = TraceTag;
	queryParam.TraceTag = TraceTag;
	*/

	if (GetWorld()->LineTraceSingleByChannel(hit, traceStart, traceEnd, ECollisionChannel::ECC_Visibility, queryParam))
	{
		Out_HitLocation = hit.Location;
		Out_HitWall = true;
	}
	else
	{
		Out_HitLocation = FVector(0.f, 0.f, 0.f);
		Out_HitWall = false;
	}
}

void AFreeMovementCharacter::UpdateFootIK(float DeltaTime)
{
	FVector hitLocationLeft, hitLocationRight;
	float footTraceOffset;

	IKFootTrace("LeftFootIKSocket", IKFootTraceDistance, hitLocationLeft, footTraceOffset);
	IKLeftFootOffset = FMath::FInterpTo(IKLeftFootOffset, footTraceOffset, DeltaTime, IKInterpSpeed);

	IKFootTrace("RightFootIKSocket", IKFootTraceDistance, hitLocationRight, footTraceOffset);
	IKRightFootOffset = FMath::FInterpTo(IKRightFootOffset, footTraceOffset, DeltaTime, IKInterpSpeed);

	float hipOffset = FMath::Abs(hitLocationRight.Z - hitLocationLeft.Z);

	if (hipOffset < 50.f)
	{
		hipOffset *= -0.5f;
	}
	else
	{
		hipOffset = 0.f;
	}

	IKHipOffset = hipOffset;
}

void AFreeMovementCharacter::UpdateHandIK()
{
	IKHandTrace("LeftArmIKSocket", IKArmTraceDistance, -10.f, IKLeftHandLocation, bLeftHandHitWall);
	IKHandTrace("RightArmIKSocket", IKArmTraceDistance, 25.f, IKRightHandLocation, bRightHandHitWall);
}

void AFreeMovementCharacter::RotateHead(float DeltaTime)
{
	float target = GetControlRotation().Yaw - GetActorRotation().Yaw;
	HeadRotation.Yaw = FMath::ClampAngle(target, -90.f, 90.f);	//Head look Left/Right
	HeadRotation.Roll = 0.f - GetControlRotation().Pitch;	//Head Look Up/Down
}

void AFreeMovementCharacter::RotateTorso(float DeltaTime)
{
	//UE_LOG(LogTemp, Warning, TEXT("Turn Axis Value %f"), GetInputAxisValue("Turn"));
	float target = FMath::ClampAngle(GetInputAxisValue("Turn") * 10.f, -15.f, 15.f);
	TorsoRotation.Yaw = target;	//Torso left/right look
	TorsoRotation.Pitch = FMath::FInterpTo(TorsoRotation.Pitch, target, DeltaTime, 5.f);	//Torso left/right lean
}

void AFreeMovementCharacter::UpperBodyCheck()
{
	FVector sweepStart = GetActorLocation() + FVector(0.f, 0.f, GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 1.25f);
	FVector sweepEnd = GetActorLocation();

	//DrawDebugLine(GetWorld(), sweepStart, sweepStart + (GetActorForwardVector() * 50), FColor::Green, false, 1.f);
	//DrawDebugLine(GetWorld(), sweepEnd, sweepEnd + (GetActorForwardVector() * 50), FColor::Green, false, 1.f);

	FVector boxExtent = FVector(GetCapsuleComponent()->GetScaledCapsuleRadius() / 2.f, GetCapsuleComponent()->GetScaledCapsuleRadius(), GetCapsuleComponent()->GetScaledCapsuleHalfHeight() / 1.6f);
	FVector boxCenter = GetActorLocation() + (GetActorForwardVector() * GetCapsuleComponent()->GetScaledCapsuleRadius() * 1.5f) + FVector(0.f, 0.f, GetCapsuleComponent()->GetScaledCapsuleHalfHeight() / 1.6f);

	FCollisionShape boxCollision = FCollisionShape::MakeBox(boxExtent);

	//DrawDebugBox(GetWorld(), boxCenter, boxExtent, FColor::Red, false, 1.f);
}

void AFreeMovementCharacter::LowerBodyCheck()
{
	

	FVector boxExtent = FVector(GetCapsuleComponent()->GetScaledCapsuleRadius() / 2.f, GetCapsuleComponent()->GetScaledCapsuleRadius(), GetCapsuleComponent()->GetScaledCapsuleHalfHeight() / 2.5f);
	FVector boxCenter = GetActorLocation() + (GetActorForwardVector() * GetCapsuleComponent()->GetScaledCapsuleRadius() * 1.5f) - FVector(0.f, 0.f, GetCapsuleComponent()->GetScaledCapsuleHalfHeight() / 2.5f);

	FCollisionShape boxCollision = FCollisionShape::MakeBox(boxExtent / 2);

	//DrawDebugBox(GetWorld(), boxCenter, boxExtent, FColor::Blue, false, 1.f);

	FHitResult hit;
	FCollisionQueryParams traceParam("TraceParam", false, this);
	FVector sweepStart = GetActorLocation() + (GetActorForwardVector() * GetCapsuleComponent()->GetScaledCapsuleRadius() * 1.5f);
	FVector sweepEnd = GetActorLocation() + (GetActorForwardVector() * GetCapsuleComponent()->GetScaledCapsuleRadius() * 1.5f) - FVector(0.f, 0.f, GetCapsuleComponent()->GetScaledCapsuleHalfHeight() / 1.25f) ;

	//DrawDebugLine(GetWorld(), sweepStart, sweepEnd, FColor::Green, false, 1.f);
	DrawDebugLine(GetWorld(), sweepStart, sweepStart + (GetActorForwardVector() * 50), FColor::Green, false, 1.f);
	DrawDebugLine(GetWorld(), sweepEnd, sweepEnd + (GetActorForwardVector() * 50), FColor::Green, false, 1.f);

	if (GetWorld()->SweepSingleByChannel(hit, sweepStart, sweepEnd, FQuat(GetActorRotation()), ECollisionChannel::ECC_WorldStatic, boxCollision, traceParam))
	{
		UE_LOG(LogTemp, Warning, TEXT("Hit something I can vault over: %s"), *hit.Actor->GetName());
		DrawDebugLine(GetWorld(), hit.Location, GetActorLocation(), FColor::Green, false, 1.f);

		//DrawDebugBox(GetWorld(), hit.Location, boxCollision.GetExtent(), FColor::Blue, false, 1.f);
	}
}

bool AFreeMovementCharacter::HeightCheck(FVector TraceStart, FVector TraceEnd)
{
	FHitResult hit;
	FCollisionQueryParams queryParam;
	//Draw trace
	const FName TraceTag("MyTraceTag");
	GetWorld()->DebugDrawTraceTag = TraceTag;
	queryParam.TraceTag = TraceTag;
	
	//Check to see if the object is taller than max grab point
	//Returns true if the wall is too high, false if climbable
	return GetWorld()->LineTraceSingleByChannel(hit, TraceStart + FVector(0.f, 0.f, 1.f), TraceEnd + FVector(0.f, 0.f, 1.f), ECollisionChannel::ECC_Visibility, queryParam);
}

void AFreeMovementCharacter::GravityTimelineHandler(float Value)
{
	GetCharacterMovement()->GravityScale = Value;
}
