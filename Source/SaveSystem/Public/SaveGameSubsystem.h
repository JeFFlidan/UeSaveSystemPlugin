// Copyright Kyrylo Zaverukha. All Rights Reserved.

#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "SaveGameSubsystem.generated.h"

class APlayerState;
class USaveGameData;
class UScreenshotTaker;
class USaveSystemSettings;
class USaveGameMetadata;
class UAbilitySystemComponent;
class UAttributeSet;
class UAutosaveCondition;
struct FGameplayAttributeData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReadWriteSaveGame, USaveGameData*, SaveGameObj);

/**
 * 
 */
UCLASS()
class SAVESYSTEM_API USaveGameSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnReadWriteSaveGame OnSaveGameLoaded;

	UPROPERTY(BlueprintAssignable)
	FOnReadWriteSaveGame OnSaveGameWritten;

	UPROPERTY(BlueprintAssignable)
	FOnReadWriteSaveGame OnAutosaveStarted;

	UPROPERTY(BlueprintAssignable)
	FOnReadWriteSaveGame OnAutosaveFinished;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "Save System")
	virtual void LoadPlayerState();

	UFUNCTION(BlueprintCallable, Category = "Save System")
	void SetSlotName(FString NewSlotName);

	UFUNCTION(BlueprintCallable, Category = "Save System")
	virtual void WriteSaveGame(FString InSlotName = "");

	UFUNCTION(BlueprintCallable, Category = "Save System")
	virtual void LoadSaveGame(FString InSlotName = "");

	UFUNCTION(BlueprintCallable, Category = "Save System")
	virtual void LoadPlayerAbilitySystemState();

	UFUNCTION(BlueprintCallable, Category = "Save System")
	virtual const TArray<USaveGameMetadata*>& LoadAllSaveGameMetadata();

	UFUNCTION(BlueprintPure, Category = "Save System")
	virtual const TArray<USaveGameMetadata*>& GetCachedGameSaveMetadata() const { return LoadedMetadata; }

protected:
	UPROPERTY()
	TObjectPtr<USaveGameData> CurrentSaveGame;

	UPROPERTY()
	TObjectPtr<USaveGameMetadata> MetadataCDO;

	UPROPERTY()
	TObjectPtr<UAutosaveCondition> AutosaveCondition;
	
	UPROPERTY()
	TObjectPtr<UScreenshotTaker> ScreenshotTaker;

	UPROPERTY()
	TObjectPtr<const USaveSystemSettings> Settings;

	UPROPERTY()
	TArray<TObjectPtr<USaveGameMetadata>> LoadedMetadata;

	FString CurrentSlotName;
	FString CurrentMetadataFilename;

	FTimerHandle AutosaveTimer;

	int32 AutosaveCounter;

	virtual void SaveGameState();
	virtual void SaveWorldState();
	virtual void SaveAbilitySystemState();
	virtual void SavePlayerState();
	virtual void HandleAutosave();
	virtual UAbilitySystemComponent* FindPlayerAbilitySystemComponent() const;
	virtual void OverrideSpawnTransform();

	UFUNCTION()
	virtual void HandleScreenshotTaken(const TArray<uint8>& ScreenshotBytes);

	void SaveGameToSlot();
	void SaveMetadata();
	USaveGameMetadata* ReadMetadata(const FString& MetadataPath) const;
	
	void RequestScreenshot() const;
	UTexture2D* LoadScreenshot(const FString& MetadataPath) const;
	
	bool CanRequestScreenshot() const;
	FString GetSaveDirectory() const;
	FString GetScreenshotFilename() const;
	FString GetScreenshotFormat() const;
	FString GetAttributeName(const FProperty* Property) const;
	FString GetAutosaveSlotName();
	int32 GetAutosaveIndex();
	FGameplayAttributeData* GetAttributeData(FProperty* Property, UAttributeSet* AttrSet);
	APlayerState* GetPlayerState() const;
	void CreateSaveGameDataObject();
};
