// Copyright Kyrylo Zaverukha. All Rights Reserved.

#include "ScreenshotTaker.h"

#include "HighResScreenshot.h"
#include "SaveSystemSettings.h"

#include "IImageWrapper.h"
#include "IImageWrapperModule.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ScreenshotTaker)

UScreenshotTaker::UScreenshotTaker()
{
	const USaveSystemSettings* Settings = GetDefault<USaveSystemSettings>();
	
	CompressionRate = Settings->CompressionRate;
	ScreenshotFormat = Settings->ScreenshotFormat;
	bIsScreenshotRequested = false;
	bUseCustomDimensions = Settings->bUseCustomScreenshotDimensions;
	Width = Settings->Width;
	Height = Settings->Height;
}

void UScreenshotTaker::RequestScreenshot()
{
	if (!GEngine || !GEngine->GameViewport || bIsScreenshotRequested)
	{
		return;
	}

	bIsScreenshotRequested = true;
	GEngine->GameViewport->OnScreenshotCaptured().AddUObject(this, &ThisClass::AcceptScreenshot);

	if (bUseCustomDimensions)
	{
		GetHighResScreenshotConfig().SetResolution(Width, Height, 1.0f);
	}
	
	FScreenshotRequest::RequestScreenshot(false);
}

void UScreenshotTaker::AcceptScreenshot(int32 InSizeX, int32 InSizeY, const TArray<FColor>& InImageData)
{
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	TSharedPtr<IImageWrapper> ImageWrapper;

	switch (ScreenshotFormat)
	{
	case EScreenshotFormat::JPEG:
		ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::JPEG);
		break;
	case EScreenshotFormat::PNG:
		ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);
		break;
	}
	
	if (!ImageWrapper.IsValid())
	{
		bIsScreenshotRequested = false;
		return;
	}

	if (!ImageWrapper->SetRaw(&InImageData[0], InImageData.Num() * sizeof(FColor), InSizeX, InSizeY, ERGBFormat::BGRA, 8))
	{
		bIsScreenshotRequested = false;
		return;
	}

	TArray64<uint8> CompressedImage64 = ImageWrapper->GetCompressed(CompressionRate);
	CompressedImage64.Shrink();
	TArray<uint8> CompressedImage(CompressedImage64);
	OnScreenshotTaken.Broadcast(CompressedImage);

	GEngine->GameViewport->OnScreenshotCaptured().RemoveAll(this);
	bIsScreenshotRequested = false;
}
