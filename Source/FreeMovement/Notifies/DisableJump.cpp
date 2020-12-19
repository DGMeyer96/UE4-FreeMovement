// Fill out your copyright notice in the Description page of Project Settings.


#include "FreeMovement/Notifies/DisableJump.h"
#include "FreeMovement/FreeMovementCharacter.h"

void UDisableJump::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	AFreeMovementCharacter* PlayerRef = Cast<AFreeMovementCharacter>(MeshComp->GetOwner());

	if (PlayerRef)
	{
		PlayerRef->bCanJump = false;
	}
}