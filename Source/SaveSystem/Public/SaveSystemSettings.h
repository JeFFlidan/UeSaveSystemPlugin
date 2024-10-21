// Copyright Kyrylo Zaverukha. All Rights Reserved.

#pragma once

#include "SaveSystemCommon.h"
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

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Screenshot")
	bool bTakeScreenshot;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Screenshot", meta = (EditCondition = "bTakeScreenshot"))
	int32 CompressionRate;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Screenshot", meta = (EditCondition = "bTakeScreenshot"))
	EScreenshotFormat ScreenshotFormat;

	USaveSystemSettings(const FObjectInitializer& Initializer);
};
