// Copyright (c) 2021 by Lukerrr


#include "VolumetricPointLight.h"
#include "DrawDebugHelpers.h"

#if WITH_EDITOR
void AVolumetricPointLight::DebugDrawContour()
{
	DrawDebugSphere(GetWorld(), GetActorLocation(), SourceRadius, 16, LineColor, false, -1.f, 0, SourceRadius * LineThickness);
}
#endif // WITH_EDITOR

void AVolumetricPointLight::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AVolumetricPointLight, SourceRadius);
}

bool AVolumetricPointLight::SerializeData(TArray<float>& Buffer)
{
	Super::SerializeData(Buffer);

	FVector Location = GetActorLocation();

	Buffer.Add(Location.X);
	Buffer.Add(Location.Y);
	Buffer.Add(Location.Z);
	Buffer.Add(SourceRadius);

	return true;
}

EVLightSourceType AVolumetricPointLight::GetType()
{
	return EVLightSourceType::VLS_POINT;
}
