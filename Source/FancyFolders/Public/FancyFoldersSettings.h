// Copyright Out-of-the-Box Plugins 2018-2024. All Rights Reserved.

#pragma once

#include <Engine/DeveloperSettings.h>

#include "FancyFoldersSettings.generated.h"

UENUM()
enum class EFolderState
{
	Normal,
	ColumnOpen,
	ColumnClosed,
};

USTRUCT()
struct FFolderData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "")
	FLinearColor Color;

	UPROPERTY(EditAnywhere, Category = "")
	FName Icon;

	const FSlateBrush* GetIcon(EFolderState State) const;
};

class FFolderDataCustomization : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance() { return MakeShared<FFolderDataCustomization>(); }

private:
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, class IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

	void HandleSourceComboChanged(TSharedPtr<FString> Item, ESelectInfo::Type SelectInfo);
	FText GetCurrentIcon() const;

	TSharedPtr<IPropertyHandle> FolderIcon;
	TArray<TSharedPtr<FString>> IconsList;
};

USTRUCT()
struct FPathAssignedData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "")
	FString Path;

	UPROPERTY(EditAnywhere, Category = "")
	FFolderData Data;
};

USTRUCT()
struct FPathPresetData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "")
	FString PathRegex;

	UPROPERTY(EditAnywhere, Category = "")
	FFolderData Data;
};

USTRUCT()
struct FFolderPresetData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "")
	FString FolderRegex;

	UPROPERTY(EditAnywhere, Category = "")
	FFolderData Data;
};

UCLASS(config = Editor, defaultconfig)
class UFancyFoldersSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	const FSlateBrush* GetIconForPath(const FString& VirtualPath, bool bIsColumnView, bool bIsOpen) const;

	UPROPERTY(EditAnywhere, config, Category = "")
	TArray<FPathAssignedData> PathAssignments;

	UPROPERTY(EditAnywhere, config, Category = "")
	TArray<FPathPresetData> PathPresets;

	UPROPERTY(EditAnywhere, config, Category = "")
	TArray<FFolderPresetData> FolderPresets;

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
