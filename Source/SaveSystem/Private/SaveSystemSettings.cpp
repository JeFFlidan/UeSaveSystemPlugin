// Copyright Kyrylo Zaverukha. All Rights Reserved.

#include "SaveSystemSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SaveSystemSettings)

USaveSystemSettings::USaveSystemSettings(const FObjectInitializer& Initializer)
	: Super(Initializer)
{
	CategoryName = "Plugins";
	SectionName = "Save System";
	
	DefaultSaveSlotName = "SaveGame01";
	bTakeScreenshot = true;
	bSaveScreenshotAsSeparateFile = true;
	ScreenshotFormat = EScreenshotFormat::JPEG;
	CompressionRate = 90;
	bUseCustomScreenshotDimensions = false;
	Width = 228;
	Height = 128;
}
