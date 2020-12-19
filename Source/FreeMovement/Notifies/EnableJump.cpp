// Fill out your copyright notice in the Description page of Project Settings.


#include "FreeMovement/Notifies/EnableJump.h"
#include "FreeMovement/FreeMovementCharacter.h"

void UEnableJump::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	AFreeMovementCharacter* PlayerRef = Cast<AFreeMovementCharacter>(MeshComp->GetOwner());

	if (PlayerRef)
	{
		PlayerRef->bCanJump = true;
	}
}