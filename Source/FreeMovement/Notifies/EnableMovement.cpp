// Fill out your copyright notice in the Description page of Project Settings.


#include "FreeMovement/Notifies/EnableMovement.h"
#include "FreeMovement/FreeMovementCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UEnableMovement::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	AFreeMovementCharacter* PlayerRef = Cast<AFreeMovementCharacter>(MeshComp->GetOwner());

	if (PlayerRef)
	{
		PlayerRef->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	}
}
