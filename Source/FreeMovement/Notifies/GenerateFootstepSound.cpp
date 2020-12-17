// Fill out your copyright notice in the Description page of Project Settings.


#include "FreeMovement/Notifies/GenerateFootstepSound.h"
#include "Kismet/GameplayStatics.h"
#include "FreeMovement/FreeMovementCharacter.h"

void UGenerateFootstepSound::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	AFreeMovementCharacter* PlayerRef = Cast<AFreeMovementCharacter>(MeshComp->GetOwner());

	if (PlayerRef)
	{
		UGameplayStatics::PlaySoundAtLocation(MeshComp, PlayerRef->Footstep, PlayerRef->GetActorLocation());
	}
}