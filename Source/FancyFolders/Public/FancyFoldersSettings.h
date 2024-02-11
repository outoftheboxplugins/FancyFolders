// Copyright Out-of-the-Box Plugins 2018-2024. All Rights Reserved.

#pragma once

#include <Engine/DeveloperSettings.h>

#include "FancyFoldersSettings.generated.h"

USTRUCT()
struct FFolderIconPreset
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "")
	FString FolderName;

	UPROPERTY(EditAnywhere, Category = "")
	FLinearColor Color;

	UPROPERTY(EditAnywhere, Category = "")
	FName Icon;

	const FSlateBrush* GetIcon(bool bIsColumnView, bool bIsOpen = false) const;
};

UCLASS(config = Editor, defaultconfig)
class UFancyFoldersSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, config, Category = "")
	TArray<FFolderIconPreset> Presets = {FFolderIconPreset({TEXT("Maps"), FLinearColor::Blue, TEXT("AndroidNew")})};

	TArray<FFolderIconPreset> Assigned;

private:
	// Begin UDeveloperSettings interface
	virtual FName GetContainerName() const override;
	virtual FName GetCategoryName() const override;
	virtual FName GetSectionName() const override;
#if WITH_EDITOR
	virtual FText GetSectionText() const override;
#endif
	// End UDeveloperSettings interface
};
