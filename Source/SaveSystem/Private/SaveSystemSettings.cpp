// Copyright Kyrylo Zaverukha. All Rights Reserved.

#include "SaveSystemSettings.h"
#include "SaveGameMetadata.h"
#include "AutosaveCondition.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SaveSystemSettings)

USaveSystemSettings::USaveSystemSettings(const FObjectInitializer& Initializer)
	: Super(Initializer)
{
	CategoryName = "Plugins";
	SectionName = "Save System";
	
	DefaultSaveSlotName = "SaveGame01";
	bCreateSeparateFolderForSave = true;
	
	bEnableAutosave = true;
	DefaultAutosaveName = "Autosave";
	AutosavePeriod = 120.0f;
	MaxAutosaveNum = 5;
	AutosaveConditionClass = UAutosaveCondition::StaticClass();
	
	bCreateMetadata = true;
	MetadataClass = USaveGameMetadata::StaticClass();
	
	bTakeScreenshot = true;
	ScreenshotFormat = EScreenshotFormat::JPEG;
	CompressionRate = 90;
	bUseCustomScreenshotDimensions = false;
	Width = 228;
	Height = 128;

	bSetControllerRotationAfterLoadingPlayerState = true;
}
