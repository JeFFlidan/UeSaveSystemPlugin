// Copyright Kyrylo Zaverukha. All Rights Reserved.

#pragma once

#include "Engine/DeveloperSettings.h"
#include "SaveSystemSettings.generated.h"

/**
 * 
 */
UCLASS(Config = SaveSystemSettings, DefaultConfig, meta = (DisplayName = "Save System"))
class SAVESYSTEM_API USaveSystemSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "General")
	FString DefaultSaveSlotName;

	USaveSystemSettings(const FObjectInitializer& Initializer);
};
