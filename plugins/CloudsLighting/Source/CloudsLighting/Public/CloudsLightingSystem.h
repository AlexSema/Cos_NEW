// Copyright (c) 2021 by Lukerrr

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Materials/MaterialParameterCollection.h"
#include "Components/BillboardComponent.h"
#include "VolumetricLightSource.h"
#include "CloudsLightingSystem.generated.h"

UCLASS()
class CLOUDSLIGHTING_API ACloudsLightingSystem : public AActor
{
	GENERATED_BODY()
	
public:
	ACloudsLightingSystem();

	/**
	* =============================================================
	* AActor interface
	*/
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
#if WITH_EDITOR
	virtual bool ShouldTickIfViewportsOnly() const override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR
	/**
	* =============================================================
	*/

	UFUNCTION(BlueprintCallable, Category = CloudLightingSystem)
	bool IsValid();

private:
	/** Fill necessary params of the render target and generate its resource */
	void InitializeRt();

	/** Save current render target params into the parameter collection */
	void UpdateLightMapParams();

	/** Collect all light sources data and serialize it into the render target */
	void UpdateLightData();

	/** Returns currently required size of render target in pixels */
	uint32 GetRequiredRtSize();

	UPROPERTY(EditAnywhere, Category = Billboard)
	UBillboardComponent* Billboard = NULL;

	/** A render target to store light sources data */
	UPROPERTY(EditAnywhere, Category = LightDataMap)
	UTextureRenderTarget2D* LightDataMap = NULL;

	/** A parameter collection to store size of the render targets */
	UPROPERTY(EditAnywhere, Category = LightDataMap)
	UMaterialParameterCollection* LightDataMapParams = NULL;

	/** Minimal allowed size of render target (power of two) */
	uint32 MinRtSizePow = 3;

	/** Currently required size of render target (power of two) */
	uint32 RequiredRtSizePow = MinRtSizePow;

	bool bIsValid = false;
	bool bIsRtValid = false;
	bool bIsRtParamsValid = false;

	std::atomic<bool> bIsBakingLight{ false };
};
