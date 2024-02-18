// Copyright Out-of-the-Box Plugins 2018-2024. All Rights Reserved.

#pragma once

#include "FancyFolderData.generated.h"

/**
 * Representation of all possible states a folder can be in
 *
 * Folder --- Normal View
 *        \-- Column View --- Open
 *                        \-- Closed
 */
UENUM()
enum class EFolderState
{
	Normal,
	ColumnOpen,
	ColumnClosed,
};

/**
 * Holds icon & color data which can be assigned to a specific, folder, path or a regex match
 */
USTRUCT()
struct FFolderData
{
	GENERATED_BODY()

	FFolderData();
	FFolderData(const FName& InIcon, const FLinearColor& InColor);

	/**
	 * Name of the Icon assigned to a folder
	 */
	UPROPERTY(EditAnywhere, Category = "")
	FName Icon;
	/**
	 * Color assigned to a folder
	 */
	UPROPERTY(EditAnywhere, Category = "")
	FLinearColor Color;
	/**
	 * Convince getter to access the matching FSlateBrush of the current icon based on the desired state
	 */
	const FSlateBrush* GetIcon(EFolderState State) const;
};

/**
 * Implements a details view customization for the FFolderData
 */
class FFolderDataCustomization : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance() { return MakeShared<FFolderDataCustomization>(); }

private:
	// Begin IPropertyTypeCustomization interface
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	// End IPropertyTypeCustomization interface

	/**
	 * Callback executed when a new icon is selected from the dropdown
	 */
	void HandleSourceComboChanged(TSharedPtr<FString> Item, ESelectInfo::Type SelectInfo);
	/**
	 * Convince function to access the folder icon current value
	 */
	FText GetCurrentIcon() const;
	/**
	 * Convince function to access the folder color current value
	 */
	FSlateColor GetCurrentColor() const;
	/**
	 * Convince function to access the folder icon current brush
	 */
	const FSlateBrush* GetCurrentBrush() const;

	/**
	 * Reference to the folder icon property currently edited
	 */
	TSharedPtr<IPropertyHandle> FolderIcon;
	/**
	 * Reference to the folder color property currently edited
	 */
	TSharedPtr<IPropertyHandle> FolderColor;
	/**
	 * List holding all the available icons the user can chose from
	 */
	TArray<TSharedPtr<FString>> IconsList;
};