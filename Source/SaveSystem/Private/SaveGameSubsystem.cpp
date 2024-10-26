// Copyright Kyrylo Zaverukha. All Rights Reserved.

#include "SaveGameSubsystem.h"
#include "SaveSystemSettings.h"
#include "SaveGameData.h"
#include "SaveGameMetadata.h"
#include "SavableObjectInterface.h"
#include "SaveSystemLogChannels.h"
#include "ScreenshotTaker.h"
#include "AutosaveCondition.h"

#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/Pawn.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "JsonObjectConverter.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
#include "Engine/AssetManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SaveGameSubsystem)

void USaveGameSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Settings = GetDefault<USaveSystemSettings>();
	AutosaveCounter = 0;

	if (Settings->bTakeScreenshot)
	{
		ScreenshotTaker = NewObject<UScreenshotTaker>();
		ScreenshotTaker->OnScreenshotTaken.AddDynamic(this, &ThisClass::HandleScreenshotTaken);
	}

	if (Settings->bCreateMetadata)
	{
		MetadataCDO = Settings->MetadataClass->GetDefaultObject<USaveGameMetadata>();
	}

	SetSlotName(Settings->DefaultSaveSlotName);

	if (Settings->bEnableAutosave)
	{
		AutosaveCondition = Settings->AutosaveConditionClass->GetDefaultObject<UAutosaveCondition>();
		GetWorld()->GetTimerManager().SetTimer(AutosaveTimer, this, &ThisClass::HandleAutosave, Settings->AutosavePeriod);
	}
}

void USaveGameSubsystem::SetSlotName(FString NewSlotName)
{
	if (NewSlotName.IsEmpty())
	{
		return;
	}

	if (Settings->bCreateSeparateFolderForSave)
	{
		CurrentSlotName = FString::Printf(TEXT("%s/%s"), *NewSlotName, *NewSlotName);
	}
	else
	{
		CurrentSlotName = NewSlotName;
	}

	if (Settings->bCreateMetadata)
	{
		MetadataCDO->SlotName = FText::FromString(NewSlotName);
		CurrentMetadataFilename = FString::Printf(TEXT("%s/%s.json"), *GetSaveDirectory(), *CurrentSlotName);
	}
}

void USaveGameSubsystem::WriteSaveGame(FString InSlotName)
{
	SetSlotName(InSlotName);
	RequestScreenshot();
	SaveGameState();
	
	OnSaveGameWritten.Broadcast(CurrentSaveGame);
}

void USaveGameSubsystem::SaveGameState()
{
	CurrentSaveGame->SavedActors.Empty();
	CurrentSaveGame->SavedAttributes.Empty();
	CurrentSaveGame->SavedGameplayEffects.Empty();
	CurrentSaveGame->SavedPlayerAbilities.Empty();

	SaveWorldState();
	SaveAbilitySystemState();
	SaveGameToSlot();
}

void USaveGameSubsystem::SaveWorldState()
{
	for (AActor* Actor : TActorRange<AActor>(GetWorld()))
	{
		if (!IsValid(Actor) || !Actor->Implements<USavableObjectInterface>())
		{
			continue;
		}

		FActorSaveData ActorData;
		ActorData.Name = Actor->GetFName();
		ActorData.Transform = Actor->GetActorTransform();

		FMemoryWriter MemWriter(ActorData.ByteData);
		FObjectAndNameAsStringProxyArchive Archive(MemWriter, true);
		Archive.ArIsSaveGame = true;
		Actor->Serialize(Archive);

		CurrentSaveGame->SavedActors.Add(ActorData);
	}
}

void USaveGameSubsystem::SaveAbilitySystemState()
{
	UAbilitySystemComponent* ASC = FindPlayerAbilitySystemComponent();

	if (!ASC)
	{
		return;
	}

	const TArray<FGameplayAbilitySpec>& AbilitySpecs = ASC->GetActivatableAbilities();
	for (const FGameplayAbilitySpec& Spec : AbilitySpecs)
	{
		UGameplayAbility* Ability = Spec.Ability;

		if (!Ability->Implements<USavableObjectInterface>())
		{
			continue;
		}

		FGameplayAbilitySaveData AbilityData;
		AbilityData.AbilityClass = Ability->GetClass();
		AbilityData.Level = Ability->GetAbilityLevel();
		AbilityData.DynamicTags = Spec.DynamicAbilityTags;

		CurrentSaveGame->SavedPlayerAbilities.Add(AbilityData);
	}

	TArray<FGameplayEffectSpec> EffectSpecs;
	ASC->GetAllActiveGameplayEffectSpecs(EffectSpecs);

	for (const FGameplayEffectSpec& Spec : EffectSpecs)
	{
		const UGameplayEffect* Effect = Spec.Def;

		if (!Effect->Implements<USavableObjectInterface>())
		{
			continue;
		}

		FGameplayEffectSaveData EffectData;
		EffectData.Level = Spec.GetLevel();
		EffectData.EffectClass = Effect->GetClass();

		CurrentSaveGame->SavedGameplayEffects.Add(EffectData);
	}

	const TArray<UAttributeSet*>& AttrSets = ASC->GetSpawnedAttributes();
	for (UAttributeSet* AttrSet : AttrSets)
	{
		for (TFieldIterator<FProperty> It(AttrSet->GetClass(), EFieldIterationFlags::IncludeSuper); It; ++It)
		{
			FProperty* Property = *It;

			if (FGameplayAttribute::IsGameplayAttributeDataProperty(Property))
			{
				const FGameplayAttributeData* DataPtr = GetAttributeData(Property, AttrSet);
				CurrentSaveGame->SavedAttributes.Add(GetAttributeName(Property), FAttributeSaveData(DataPtr->GetBaseValue()));
			}
		}
	}
}

void USaveGameSubsystem::HandleAutosave()
{
	if (!Settings->bEnableAutosave)
	{
		return;
	}
	
	AsyncTask(ENamedThreads::GameThread, [this]
	{
		while (true)
		{
			if (AutosaveCondition->IsAutosavePossible())
			{
				OnAutosaveStarted.Broadcast(CurrentSaveGame);
				
				SetSlotName(GetAutosaveSlotName());
				SaveGameState();
				RequestScreenshot();
				
				OnAutosaveFinished.Broadcast(CurrentSaveGame);
				
				FTimerManager& TimerManager = GetWorld()->GetTimerManager();
				TimerManager.ClearTimer(AutosaveTimer);
				TimerManager.SetTimer(AutosaveTimer, this, &ThisClass::HandleAutosave, Settings->AutosavePeriod);
				
				break;
			}
		}
	});
}

void USaveGameSubsystem::SaveGameToSlot()
{
	SaveMetadata();
	UGameplayStatics::SaveGameToSlot(CurrentSaveGame, CurrentSlotName, 0);
	UE_LOG(LogSaveSystem, Display, TEXT("Wrote SaveGameData to slot %s"), *CurrentSlotName)
}

void USaveGameSubsystem::LoadSaveGame(FString InSlotName)
{
	SetSlotName(InSlotName);
	
	if (UGameplayStatics::DoesSaveGameExist(CurrentSlotName, 0))
	{
		CurrentSaveGame = Cast<USaveGameData>(UGameplayStatics::LoadGameFromSlot(CurrentSlotName, 0));
		if (!CurrentSaveGame)
		{
			UE_LOG(LogSaveSystem, Warning, TEXT("Failed to load SaveGameData slot %s"), *CurrentSlotName);
			return;
		}
		
		UE_LOG(LogSaveSystem, Display, TEXT("Loaded SaveGameData from slot %s"), *CurrentSlotName);

		for (AActor* Actor : TActorRange<AActor>(GetWorld()))
		{
			if (!Actor->Implements<USavableObjectInterface>())
			{
				continue;
			}

			bool bIsActorValid = false;
			
			for (FActorSaveData ActorData : CurrentSaveGame->SavedActors)
			{
				if (ActorData.Name == Actor->GetFName())
				{
					Actor->SetActorTransform(ActorData.Transform);

					FMemoryReader MemReader(ActorData.ByteData);
					FObjectAndNameAsStringProxyArchive Archive(MemReader, true);
					Archive.ArIsSaveGame = true;
					Actor->Serialize(Archive);
					ISavableObjectInterface::Execute_OnActorLoaded(Actor);

					bIsActorValid = true;

					break;
				}
			}

			if (!bIsActorValid)
			{
				Actor->Destroy();
			}
		}

		OnSaveGameLoaded.Broadcast(CurrentSaveGame);
	}
	else
	{
		CurrentSaveGame = CastChecked<USaveGameData>(UGameplayStatics::CreateSaveGameObject(USaveGameData::StaticClass()));
		UE_LOG(LogSaveSystem, Display, TEXT("Created new SaveGameData object"));
	}
}

void USaveGameSubsystem::LoadPlayerAbilitySystemState()
{
	UAbilitySystemComponent* ASC = FindPlayerAbilitySystemComponent();

	if (!ASC)
	{
		return;
	}

	const TArray<UAttributeSet*>& AttrSets = ASC->GetSpawnedAttributes();
	for (UAttributeSet* AttrSet : AttrSets)
	{
		for (TFieldIterator<FProperty> It(AttrSet->GetClass(), EFieldIterationFlags::IncludeSuper); It; ++It)
		{
			FProperty* Property = *It;

			if (!FGameplayAttribute::IsGameplayAttributeDataProperty(Property))
			{
				continue;
			}

			if (FAttributeSaveData* AttrSaveData = CurrentSaveGame->SavedAttributes.Find(GetAttributeName(Property)))
			{
				FGameplayAttributeData* DataPtr = GetAttributeData(Property, AttrSet);
				DataPtr->SetBaseValue(AttrSaveData->BaseValue);
				DataPtr->SetCurrentValue(AttrSaveData->BaseValue);
			}
		}
	}

	for (const FGameplayAbilitySaveData& AbilityData : CurrentSaveGame->SavedPlayerAbilities)
	{
		UGameplayAbility* AbilityCDO = AbilityData.AbilityClass->GetDefaultObject<UGameplayAbility>();

		FGameplayAbilitySpec AbilitySpec(AbilityCDO, AbilityData.Level);
		AbilitySpec.DynamicAbilityTags = AbilityData.DynamicTags;

		ASC->GiveAbility(AbilitySpec);
	}

	for (const FGameplayEffectSaveData& EffectData : CurrentSaveGame->SavedGameplayEffects)
	{
		const UGameplayEffect* GameplayEffectCDO = EffectData.EffectClass->GetDefaultObject<UGameplayEffect>();
		ASC->ApplyGameplayEffectToSelf(GameplayEffectCDO, EffectData.Level, ASC->MakeEffectContext());
	}
}

const TArray<USaveGameMetadata*>& USaveGameSubsystem::LoadAllSaveGameMetadata()
{
	LoadedMetadata.Empty();
	
	if (!Settings->bCreateMetadata)
	{
		return LoadedMetadata; // Empty array
	}
	
	TArray<FString> MetadataPaths;
	FString Extension = "*.json";
	IFileManager& FileManager = IFileManager::Get();
	FileManager.FindFilesRecursive(MetadataPaths, *GetSaveDirectory(), *Extension, true, false);

	for (const FString& Path : MetadataPaths)
	{
		if (USaveGameMetadata* Metadata = ReadMetadata(Path))
		{
			LoadedMetadata.Add(Metadata);
		}
	}

	return LoadedMetadata;
}

UAbilitySystemComponent* USaveGameSubsystem::FindPlayerAbilitySystemComponent() const
{
	APlayerState* PlayerState = UGameplayStatics::GetPlayerState(GetWorld(), 0);
	
	if (UAbilitySystemComponent* ASC = PlayerState->FindComponentByClass<UAbilitySystemComponent>())
	{
		return ASC;
	}

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (UAbilitySystemComponent* ASC = PlayerPawn->FindComponentByClass<UAbilitySystemComponent>())
	{
		return ASC;
	}

	return nullptr;
}

void USaveGameSubsystem::HandleScreenshotTaken(const TArray<uint8>& ScreenshotBytes)
{
	UE_LOG(LogSaveSystem, Display, TEXT("Screenshot bytes: %d"), ScreenshotBytes.Num())
	
	FFileHelper::SaveArrayToFile(ScreenshotBytes, *GetScreenshotFilename());
}

void USaveGameSubsystem::SaveMetadata()
{
	if (!Settings->bCreateMetadata)
	{
		return;
	}
	
	MetadataCDO->InitMetadata();

	TSharedRef<FJsonObject> JsonObject(new FJsonObject());

	if (!FJsonObjectConverter::UStructToJsonObject(MetadataCDO->GetClass(), MetadataCDO, JsonObject))
	{
		UE_LOG(LogSaveSystem, Error, TEXT("Failed to convert metadata to json object."));
		return;
	}

	FString JsonString;
	if (!FJsonSerializer::Serialize(JsonObject, TJsonWriterFactory<>::Create(&JsonString, 0)))
	{
		UE_LOG(LogSaveSystem, Error, TEXT("Failed to serialize json object to string."));
		return;
	}

	if (!FFileHelper::SaveStringToFile(JsonString, *CurrentMetadataFilename))
	{
		UE_LOG(LogSaveSystem, Error, TEXT("Failed to save json string to file %s."), *CurrentMetadataFilename);
	}
}

USaveGameMetadata* USaveGameSubsystem::ReadMetadata(const FString& MetadataPath) const
{
	if (!Settings->bCreateMetadata)
    {
    	return nullptr;
    }

	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *MetadataPath))
	{
		UE_LOG(LogSaveSystem, Error, TEXT("Failed to read json string from file %s."), *MetadataPath);
		return nullptr;
	}

	TSharedPtr<FJsonObject> JsonObject;
	if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(JsonString), JsonObject))
	{
		UE_LOG(LogSaveSystem, Error, TEXT("Failed to deserialize json object from string."));
		return nullptr;
	}

	USaveGameMetadata* Metadata = NewObject<USaveGameMetadata>();
	if (!FJsonObjectConverter::JsonObjectToUStruct(JsonObject.ToSharedRef(), Metadata->GetClass(), Metadata))
	{
		UE_LOG(LogSaveSystem, Error, TEXT("Failed to convert json object to metadata."));
		return nullptr;
	}

	if (Settings->bTakeScreenshot)
	{
		Metadata->Screenshot = LoadScreenshot(MetadataPath);
	}

	return Metadata;
}

void USaveGameSubsystem::RequestScreenshot() const
{
	if (CanRequestScreenshot())
	{
		ScreenshotTaker->RequestScreenshot();
	}
}

UTexture2D* USaveGameSubsystem::LoadScreenshot(const FString& MetadataPath) const
{
	FString ScreenshotPath = FPaths::ChangeExtension(MetadataPath, GetScreenshotFormat());

	TArray<uint8> ScreenshotBytes;
	if (!FFileHelper::LoadFileToArray(ScreenshotBytes, *ScreenshotPath))
	{
		UE_LOG(LogSaveSystem, Error, TEXT("Failed to load screenshot from file %s."), *ScreenshotPath);
		return nullptr;
	}

	return UKismetRenderingLibrary::ImportBufferAsTexture2D(GetWorld(), ScreenshotBytes);
}

bool USaveGameSubsystem::CanRequestScreenshot() const
{
	return Settings->bTakeScreenshot && ScreenshotTaker;
}

FString USaveGameSubsystem::GetSaveDirectory() const
{
	return FString::Printf(TEXT("%s/SaveGames"), *UKismetSystemLibrary::GetProjectSavedDirectory());
}

FString USaveGameSubsystem::GetScreenshotFilename() const
{
	return FString::Printf(TEXT("%s/%s.%s"), *GetSaveDirectory(), *CurrentSlotName, *GetScreenshotFormat());
}

FString USaveGameSubsystem::GetScreenshotFormat() const
{
	return UEnum::GetDisplayValueAsText(ScreenshotTaker->GetScreenshotFormat()).ToString().ToLower();
}

FString USaveGameSubsystem::GetAttributeName(const FProperty* Property) const
{
	return FString::Printf(TEXT("%s.%s"), *Property->GetOwnerVariant().GetName(), *Property->GetName());
}

FString USaveGameSubsystem::GetAutosaveSlotName()
{
	return FString::Printf(TEXT("%s%02d"), *Settings->DefaultAutosaveName, GetAutosaveIndex());
}

int32 USaveGameSubsystem::GetAutosaveIndex()
{
	if (AutosaveCounter >= Settings->MaxAutosaveNum)
	{
		AutosaveCounter = 1;
		return AutosaveCounter;
	}

	return ++AutosaveCounter;
}

FGameplayAttributeData* USaveGameSubsystem::GetAttributeData(FProperty* Property, UAttributeSet* AttrSet)
{
	FStructProperty* StructProperty = CastFieldChecked<FStructProperty>(Property);
	FGameplayAttributeData* DataPtr = StructProperty->ContainerPtrToValuePtr<FGameplayAttributeData>(AttrSet);
	check(DataPtr);

	return DataPtr;
}
