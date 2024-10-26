// Copyright Kyrylo Zaverukha. All Rights Reserved.

#pragma once

#include "SaveSystemCommon.h"
#include "Engine/DeveloperSettings.h"

#include "SaveSystemSettings.generated.h"

class USaveGameMetadata;
class UAutosaveCondition;

/**
 * 
 */
UCLASS(Config = SaveSystemSettings, DefaultConfig)
class SAVESYSTEM_API USaveSystemSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "General")
	FString DefaultSaveSlotName;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "General")
	bool bCreateSeparateFolderForSave;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Autosave")
	bool bEnableAutosave;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Autosave", meta = (EditCondition = "bEnableAutosave"))
	FString DefaultAutosaveName;
	
	/**
	 * At the end of a period of time in seconds, autosaving occurs.
	 * After autosaving, the timer will be reset and the period will begin counting again.
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Autosave", meta = (EditCondition = "bEnableAutosave"))
	float AutosavePeriod;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Autosave", meta = (EditCondition = "bEnableAutosave"))
	int32 MaxAutosaveNum;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Autosave", meta = (EditCondition = "bEnableAutosave"))
	TSubclassOf<UAutosaveCondition> AutosaveConditionClass;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Metadata")
	bool bCreateMetadata;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Metadata", meta = (EditCondition = "bCreateMetadata"))
	TSubclassOf<USaveGameMetadata> MetadataClass;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Screenshot")
	bool bTakeScreenshot;

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
