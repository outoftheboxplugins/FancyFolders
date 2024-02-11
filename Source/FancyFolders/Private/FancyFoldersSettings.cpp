// Copyright Out-of-the-Box Plugins 2018-2024. All Rights Reserved.

#include "FancyFoldersSettings.h"

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
