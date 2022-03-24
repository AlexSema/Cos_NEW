// Copyright 2020-2021 Fly Dream Dev. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InsectsAvoidVolume.generated.h"

UCLASS()
class ADVANCEDINSECTSSYSTEM_API AInsectsAvoidVolume : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AInsectsAvoidVolume();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//************************************************************************
	// Component                                                                  
	//************************************************************************

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Component")
	class UBoxComponent* BoxComponent;

	//************************************************************************

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:	
	

};
