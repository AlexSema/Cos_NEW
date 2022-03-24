// Copyright 2020-2021 Fly Dream Dev. All Rights Reserved.


#include "InsectsAvoidVolume.h"

#include "Components/BoxComponent.h"

// Sets default values
AInsectsAvoidVolume::AInsectsAvoidVolume()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	//************************************************************************
	// Component                                                                  
	//************************************************************************

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	BoxComponent->SetupAttachment(RootComponent);
	BoxComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BoxComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	BoxComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	//************************************************************************

	

}

// Called when the game starts or when spawned
void AInsectsAvoidVolume::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AInsectsAvoidVolume::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

