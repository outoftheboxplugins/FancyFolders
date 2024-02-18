// Copyright Out-of-the-Box Plugins 2018-2024. All Rights Reserved.

#include "FancyFoldersSettings.h"

#include <AssetViewUtils.h>

TOptional<FFolderData> UFancyFoldersSettings::GetDataForPath(const FString& Path) const
{
	for (const auto& PathAssigned : PathAssignments)
	{
		if (PathAssigned.Path == Path)
		{
			return PathAssigned.Data;
		}
	}

	for (const auto& FolderPreset : FolderPresets)
	{
		const FString FolderName = FPaths::GetBaseFilename(Path);

		const FRegexPattern FolderPattern(FolderPreset.FolderRegex);
		if (FRegexMatcher(FolderPattern, FolderName).FindNext())
		{
			return FolderPreset.Data;
		}
	}

	for (const auto& PathPreset : PathPresets)
	{
		const FRegexPattern FolderPattern(PathPreset.PathRegex);
		if (FRegexMatcher(FolderPattern, Path).FindNext())
		{
			return PathPreset.Data;
		}
	}

	return {};
}

TOptional<FLinearColor> UFancyFoldersSettings::GetColorForPath(const FString& Path) const
{
	const TOptional<FFolderData> DataForPath = GetDataForPath(Path);
	if (!DataForPath)
	{
		return {};
	}

	return DataForPath.GetValue().Color;
}

const FSlateBrush* UFancyFoldersSettings::GetIconForPath(const FString& Path, bool bIsColumnView, bool bIsOpen) const
{
	const TOptional<FFolderData> DataForPath = GetDataForPath(Path);
	if (!DataForPath)
	{
		return nullptr;
	}

	const EFolderState FolderState = StateFromFlags(bIsColumnView, bIsOpen);
	return DataForPath.GetValue().GetIcon(FolderState);
}

void UFancyFoldersSettings::UpdateOrCreateAssignmentIcon(const FString& Path, const FName& Icon)
{
	FPathAssignedData* CurrentAssignment = PathAssignments.FindByPredicate(
		[Path](const FPathAssignedData& Data)
		{
			return Data.Path == Path;
		}
	);

	if (CurrentAssignment)
	{
		CurrentAssignment->Data.Icon = Icon;
	}
	else
	{
		FPathAssignedData NewAssignment(Path, {Icon, AssetViewUtils::GetDefaultColor()});
		PathAssignments.Emplace(NewAssignment);
	}
}

void UFancyFoldersSettings::UpdateOrCreateAssignmentColor(const FString& Path, const FLinearColor& Color)
{
	FPathAssignedData* CurrentAssignment = PathAssignments.FindByPredicate(
		[Path](const FPathAssignedData& Data)
		{
			return Data.Path == Path;
		}
	);

	if (CurrentAssignment)
	{
		CurrentAssignment->Data.Color = Color;
	}
	else
	{
		FPathAssignedData NewAssignment(Path, {FName("Default"), Color});
		PathAssignments.Emplace(NewAssignment);
	}
}

void UFancyFoldersSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.MemberProperty && PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive)
	{
		if (PropertyChangedEvent.MemberProperty->GetFName() == GET_MEMBER_NAME_CHECKED(UFancyFoldersSettings, PathAssignments))
		{
			for (FPathAssignedData Assignment : PathAssignments)
			{
				AssetViewUtils::SetPathColor(Assignment.Path, Assignment.Data.Color);
			}
		}
	}
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
