// Copyright Out-of-the-Box Plugins 2018-2024. All Rights Reserved.

#include "FancyFoldersSettings.h"

#include <DetailWidgetRow.h>
#include <IDetailChildrenBuilder.h>

#include "FancyFolders.h"
#include "FancyFoldersStyle.h"

EFolderState StateFromFlags(bool bIsColumnView, bool bIsOpen)
{
	if (bIsColumnView)
	{
		return bIsOpen ? EFolderState::ColumnOpen : EFolderState::ColumnClosed;
	}

	return EFolderState::Normal;
}

const FSlateBrush* FFolderData::GetIcon(EFolderState State) const
{
	const FString IconType = [=]()
	{
		switch (State)
		{
		case EFolderState::Normal:
			return TEXT("Normal");
		case EFolderState::ColumnOpen:
			return TEXT("ColumnOpen");
		case EFolderState::ColumnClosed:
			return TEXT("ColumnClosed");
		}

		checkNoEntry();
		return TEXT("");
	}();

	const FName Brush = *FString::Printf(TEXT("%s.%s"), *Icon.ToString(), *IconType);
	return FFancyFoldersStyle::Get().GetBrush(Brush);
}

void FFolderDataCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
}

void FFolderDataCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	FolderIcon = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FFolderData, Icon));

	for (const FString& IconAvailable : FFancyFoldersModule::GetIconFoldersOnDisk())
	{
		const FString IconName = FPaths::GetBaseFilename(IconAvailable);
		IconsList.Add(MakeShared<FString>(IconName));
	}

	// clang-format off
	ChildBuilder.AddCustomRow(StructPropertyHandle->GetPropertyDisplayName())
	.NameContent()
	[
		FolderIcon->CreatePropertyNameWidget()
	]
	.ValueContent()
	[
		SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Left)
		[
			SNew(SImage)
			.Image(this, &FFolderDataCustomization::GetCurrentBrush)
			.ColorAndOpacity(this, &FFolderDataCustomization::GetCurrentColor)
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SComboBox<TSharedPtr<FString>>)
				.OptionsSource(&IconsList)
				.OnSelectionChanged(this, &FFolderDataCustomization::HandleSourceComboChanged)
				.OnGenerateWidget_Lambda([](TSharedPtr<FString> Item)
				{
					return SNew(STextBlock).Text(FText::FromString(*Item));
				})
				.Content()
				[
					SNew(STextBlock)
					.Text(this, &FFolderDataCustomization::GetCurrentIcon)
				]
		]
	];
	// clang-format on

	FolderColor = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FFolderData, Color));
	ChildBuilder.AddProperty(FolderColor.ToSharedRef());
}

void FFolderDataCustomization::HandleSourceComboChanged(TSharedPtr<FString> Item, ESelectInfo::Type SelectInfo)
{
	FolderIcon->SetValue(*Item);
}

FText FFolderDataCustomization::GetCurrentIcon() const
{
	FString IconValue;
	FolderIcon->GetValueAsDisplayString(IconValue);
	return FText::FromString(IconValue);
}

FSlateColor FFolderDataCustomization::GetCurrentColor() const
{
	void* Data;
	FolderColor->GetValueData(Data);

	const FLinearColor* ColorData = static_cast<FLinearColor*>(Data);
	return FSlateColor(*ColorData);
}

const FSlateBrush* FFolderDataCustomization::GetCurrentBrush() const
{
	FString IconValue;
	FolderIcon->GetValueAsDisplayString(IconValue);

	return FFancyFoldersStyle::Get().GetBrush(*FString::Printf(TEXT("%s.Normal"), *IconValue));
}

const FSlateBrush* UFancyFoldersSettings::GetIconForPath(const FString& VirtualPath, bool bIsColumnView, bool bIsOpen) const
{
	const EFolderState FolderState = StateFromFlags(bIsColumnView, bIsOpen);

	for (const auto& PathAssigned : PathAssignments)
	{
		if (PathAssigned.Path == VirtualPath)
		{
			return PathAssigned.Data.GetIcon(FolderState);
		}
	}

	for (const auto& FolderPreset : FolderPresets)
	{
		const FString FolderName = FPaths::GetBaseFilename(VirtualPath);

		const FRegexPattern FolderPattern(FolderPreset.FolderRegex);
		if (FRegexMatcher(FolderPattern, FolderName).FindNext())
		{
			return FolderPreset.Data.GetIcon(FolderState);
		}
	}

	for (const auto& PathPreset : PathPresets)
	{
		const FRegexPattern FolderPattern(PathPreset.PathRegex);
		if (FRegexMatcher(FolderPattern, VirtualPath).FindNext())
		{
			return PathPreset.Data.GetIcon(FolderState);
		}
	}

	return nullptr;
}

FName UFancyFoldersSettings::GetContainerName() const
{
	return TEXT("Editor");
}

FName UFancyFoldersSettings::GetCategoryName() const
{
	return TEXT("Out-of-the-Box Plugins");
}
FName UFancyFoldersSettings::GetSectionName() const
{
	return TEXT("Folder Icons");
}

#if WITH_EDITOR
FText UFancyFoldersSettings::GetSectionText() const
{
	const FName DisplaySectionName = GetSectionName();
	return FText::FromName(DisplaySectionName);
}
#endif
