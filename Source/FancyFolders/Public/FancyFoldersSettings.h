// Copyright Out-of-the-Box Plugins 2018-2024. All Rights Reserved.

#pragma once

#include <Engine/DeveloperSettings.h>

#include "FancyFolderData.h"

#include "FancyFoldersSettings.generated.h"

/**
 * Struct holding data directly assigned to an individual folder
 */
USTRUCT()
struct FPathAssignedData
{
	GENERATED_BODY()
	/**
	 * Full path of the folder, e.g.: /Game/MyTopFolder/MyLowerFolder
	 */
	UPROPERTY(EditAnywhere, Category = "")
	FString Path;
	/**
	 * Color & icon assigned
	 */
	UPROPERTY(EditAnywhere, Category = "")
	FFolderData Data;
};
/**
 * Struct holding data assigned to a folder's path regex match
 */
USTRUCT()
struct FPathPresetData
{
	GENERATED_BODY()
	/**
	 * Regex to match a folder's path, e.g.: /Game/ArchitectureVis_*
	 */
	UPROPERTY(EditAnywhere, Category = "")
	FString PathRegex;
	/**
	 * Color & icon assigned
	 */
	UPROPERTY(EditAnywhere, Category = "")
	FFolderData Data;
};
/**
 * Struct holding data assigned to a folder's name regex match
 */
USTRUCT()
struct FFolderPresetData
{
	GENERATED_BODY()
	/**
	 * Regex to match a folder's name, e.g.: BP_*
	 */
	UPROPERTY(EditAnywhere, Category = "")
	FString FolderRegex;
	/**
	 * Color & icon assigned
	 */
	UPROPERTY(EditAnywhere, Category = "")
	FFolderData Data;
};

/**
 * Implements the settings for the FancyFolder plugin.
 */
UCLASS(config = FancyFolders, defaultconfig)
class UFancyFoldersSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/**
	 * Convince function to access a folder's data based on it's path
	 */
	TOptional<FFolderData> GetDataForPath(const FString& Path) const;
	/**
	 * Get the color assigned to a specific folder (if any)
	 */
	TOptional<FLinearColor> GetColorForPath(const FString& Path) const;
	/**
	 * Get the icon assigned to a specific folder (if any)
	 */
	const FSlateBrush* GetIconForPath(const FString& Path, bool bIsColumnView, bool bIsOpen) const;
	/**
	 * Update (or creates) the assignment at a specific path using the provided icon. Color remains unchanged or set to default
	 */
	void UpdateOrCreateAssignmentIcon(const FString& Path, const FName& Icon);
	/**
	 * Update (or creates) the assignment at a specific path using the provided color. Icon remains unchanged or set to default
	 */
	void UpdateOrCreateAssignmentColor(const FString& Path, const FLinearColor& Color);

private:
	/**
	 * Data rules based on a folder's full path
	 */
	UPROPERTY(EditAnywhere, config, Category = "")
	TArray<FPathAssignedData> PathAssignments;
	/**
	 * Data rules based on a folder's path regex match
	 */
	UPROPERTY(EditAnywhere, config, Category = "")
	TArray<FPathPresetData> PathPresets;
	/**
	 * Data rules based on a folder's name regex match
	 */
	UPROPERTY(EditAnywhere, config, Category = "")
	TArray<FFolderPresetData> FolderPresets;

	// Begin UDeveloperSettings interface
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual FName GetContainerName() const override;
	virtual FName GetCategoryName() const override;
	virtual FName GetSectionName() const override;
#if WITH_EDITOR
	virtual FText GetSectionText() const override;
#endif
	// End UDeveloperSettings interface
};
