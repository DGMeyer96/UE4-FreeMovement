// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FreeMovementCharacter.generated.h"

UCLASS(config=Game)
class AFreeMovementCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

public:
	AFreeMovementCharacter();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement)
	float MoveForwardInput;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement)
	float MoveRightInput;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement)
	bool bClimbing;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement)
	float BaseWalkSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement)
	float SprintWalkSpeed;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement)
	float LongFallDistance;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement)
	float DistanceFallen;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	FRotator HeadRotation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IK)
	float Scale;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IK)
	float IKFootTraceDistance;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IK)
	float IKArmTraceDistance;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IK)
	float IKInterpSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IK)
	float IKHipOffset;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IK)
	float IKLeftFootOffset;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IK)
	float IKRightFootOffset;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IK)
	FVector IKLeftHandLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IK)
	FVector IKRightHandLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IK)
	bool bLeftHandHitWall;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = IK)
	bool bRightHandHitWall;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Audio)
	class USoundBase* Footstep;

protected:
	virtual void BeginPlay();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void IKFootTrace(FName SocketName, float TraceDistance, FVector &Out_HitLocation, float &Out_FootTraceOffset);

	void IKHandTrace(FName SocketName, float TraceDistance, float HandOffset, FVector& Out_HitLocation, bool& Out_HitWall);

	void UpdateFootIK(float DeltaTime);

	void UpdateHandIK();

	void RotateHead(float DeltaTime);

	void UpperBodyCheck();

	void LowerBodyCheck();

	bool HeightCheck(FVector TraceStart, FVector TraceEnd);

protected:

	void OnStartJump();

	void WallRunCheck();

	void SprintStart();

	void SprintStop();

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

