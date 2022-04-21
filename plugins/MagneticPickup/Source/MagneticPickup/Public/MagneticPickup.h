// Developed by Charalampos Hadjiantoniou - www.chadjiantoniou.com - 2019

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FMagneticPickupModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
