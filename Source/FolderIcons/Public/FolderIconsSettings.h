// My super cool copyright notice

#pragma once

#include <Engine/DeveloperSettings.h>

#include "FolderIconsSettings.generated.h"

USTRUCT()
struct FFolderIconPreset
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "")
	FString FolderName;

	UPROPERTY(EditAnywhere, Category = "")
	FLinearColor Color;

	UPROPERTY(EditAnywhere, Category = "")
	FString Icon;
};

UCLASS(config = Editor, defaultconfig)
class UFolderIconsSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, config, Category = "")
	TArray<FFolderIconPreset> Presets;

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
