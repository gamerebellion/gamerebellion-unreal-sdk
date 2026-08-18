#pragma once

#include "Modules/ModuleManager.h"

class FGameRebellionModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
