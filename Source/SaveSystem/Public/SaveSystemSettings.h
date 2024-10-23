// Copyright Kyrylo Zaverukha. All Rights Reserved.

#pragma once

#include "SaveSystemCommon.h"
#include "Engine/DeveloperSettings.h"

#include "SaveSystemSettings.generated.h"

class USaveGameMetadata;

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

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "General")
	bool bCreateSeparateFolderForSave;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "General")
	bool bCreateMetadata;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "General", meta = (EditCondition = "bCreateMetadata"))
	TSubclassOf<USaveGameMetadata> MetadataClass;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Screenshot")
	bool bTakeScreenshot;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Screenshot", meta = (EditCondition = "bTakeScreenshot"))
	bool bSaveScreenshotAsSeparateFile;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Screenshot", meta = (EditCondition = "bTakeScreenshot"))
	int32 CompressionRate;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Screenshot", meta = (EditCondition = "bTakeScreenshot"))
	EScreenshotFormat ScreenshotFormat;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Screenshot", meta = (EditCondition = "bTakeScreenshot"))
	bool bUseCustomScreenshotDimensions;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Screenshot", meta = (EditCondition = "bUseCustomScreenshotDimensions", ClampMin = 1, ClampMax = 3840))
	int32 Width;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Screenshot", meta = (EditCondition = "bUseCustomScreenshotDimensions", ClampMin = 1, ClampMax = 2160))
	int32 Height;
	
	USaveSystemSettings(const FObjectInitializer& Initializer);
};
