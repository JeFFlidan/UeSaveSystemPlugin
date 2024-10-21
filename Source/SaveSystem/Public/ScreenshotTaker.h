// Copyright Kyrylo Zaverukha. All Rights Reserved.

#pragma once

#include "SaveSystemCommon.h"
#include "Containers/ContainersFwd.h"
#include "ScreenshotTaker.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScreenshotTaken, const TArray<uint8>&, ImageData);

/**
 * Idea taken from https://gist.github.com/alanedwardes/282e54302bcdffbf51de440600091ea5
 */
UCLASS(BlueprintType, Blueprintable)
class SAVESYSTEM_API UScreenshotTaker : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Screenshot")
	FOnScreenshotTaken OnScreenshotTaken;
	
	UScreenshotTaker();

	bool IsScreenshotRequested() const { return bIsScreenshotRequested; }
	void SetImageFormat(EScreenshotFormat InImageFormat) { ScreenshotFormat = InImageFormat; }
	void SetCompressionRate(int32 InCompressionRate) { CompressionRate = InCompressionRate; }
	
	UFUNCTION(BlueprintCallable, Category = "Screenshot")
	virtual void RequestScreenshot();

protected:
	virtual void AcceptScreenshot(int32 InSizeX, int32 InSizeY, const TArray<FColor>& InImageData);

	bool bIsScreenshotRequested;
	EScreenshotFormat ScreenshotFormat;
	int32 CompressionRate;
};
