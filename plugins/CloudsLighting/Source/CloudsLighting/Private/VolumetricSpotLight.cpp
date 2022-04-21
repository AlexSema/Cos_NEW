// Copyright (c) 2021 by Lukerrr


#include "VolumetricSpotLight.h"
#include "DrawDebugHelpers.h"

#if WITH_EDITOR
void AVolumetricSpotLight::DebugDrawContour()
{
	float Angle = FMath::Atan2(SourceRadius, SourceLength);
	float ThicknessMult = SourceLength;
	DrawDebugCone(	GetWorld(), GetActorLocation(), GetActorUpVector(),
					SourceLength, Angle, Angle, 16, LineColor,
					false, -1.f, 0, SourceLength * LineThickness);
}
#endif // WITH_EDITOR

void AVolumetricSpotLight::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AVolumetricSpotLight, SourceLength);
	DOREPLIFETIME(AVolumetricSpotLight, SourceRadius);
	DOREPLIFETIME(AVolumetricSpotLight, CutDistance);
	DOREPLIFETIME(AVolumetricSpotLight, AngFalloffMult);
}

bool AVolumetricSpotLight::SerializeData(TArray<float>& Buffer)
{
	if (SourceLength <= 0.f || SourceRadius <= 0.f)
	{
		return false;
	}

	Super::SerializeData(Buffer);

	FVector Location = GetActorLocation();
	float SectorRadius = FMath::Sqrt(SourceRadius * SourceRadius + SourceLength * SourceLength);
	float SourceAngleCos = SourceLength / SectorRadius;
	FVector LightDirection = GetActorUpVector();

	Buffer.Add(Location.X);
	Buffer.Add(Location.Y);
	Buffer.Add(Location.Z);
	Buffer.Add(LightDirection.X);
	Buffer.Add(LightDirection.Y);
	Buffer.Add(LightDirection.Z);
	Buffer.Add(SectorRadius);
	Buffer.Add(SourceAngleCos);
	Buffer.Add(CutDistance);
	Buffer.Add(AngFalloffMult);

	return true;
}

EVLightSourceType AVolumetricSpotLight::GetType()
{
	return EVLightSourceType::VLS_SPOT;
}
