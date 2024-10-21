// Copyright Kyrylo Zaverukha. All Rights Reserved.

#include "SaveGameData.h"
#include "Kismet/KismetRenderingLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SaveGameData)

UTexture2D* USaveGameData::GetScreenshotTexture(UObject* WorldContextObject) const
{
	return UKismetRenderingLibrary::ImportBufferAsTexture2D(WorldContextObject, ScreenshotBytes);
}
