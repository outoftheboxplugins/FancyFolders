// Copyright Out-of-the-Box Plugins 2018-2025. All Rights Reserved.

#include "FancyFoldersSettings.h"

#include <AssetViewUtils.h>

EFolderState StateFromFlags(bool bIsColumnView, bool bIsOpen)
{
	if (bIsColumnView)
	{
		return bIsOpen ? EFolderState::ColumnOpen : EFolderState::ColumnClosed;
	}

	return EFolderState::Normal;
}

TOptional<FFolderData> UFancyFoldersSettings::GetDataForPath(const FString& Path) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UFancyFoldersSettings::GetDataForPath)

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
	TRACE_CPUPROFILER_EVENT_SCOPE(UFancyFoldersSettings::GetColorForPath)

	const TOptional<FFolderData> DataForPath = GetDataForPath(Path);
	if (!DataForPath)
	{
		return {};
	}

	return DataForPath.GetValue().Color;
}

const FSlateBrush* UFancyFoldersSettings::GetIconForPath(const FString& Path, bool bIsColumnView, bool bIsOpen) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UFancyFoldersSettings::GetIconForPath)

	const TOptional<FFolderData> DataForPath = GetDataForPath(Path);
	if (!DataForPath)
	{
		return nullptr;
	}

	const EFolderState FolderState = StateFromFlags(bIsColumnView, bIsOpen);
	return DataForPath.GetValue().GetIcon(FolderState);
}

void UFancyFoldersSettings::UpdateOrCreateAssignmentIcon(const FString& Path, TOptional<FName> Icon)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UFancyFoldersSettings::UpdateOrCreateAssignmentIcon)

	auto FindEntryPredicate = [Path](const FPathAssignedData& Data)
	{
		return Data.Path == Path;
	};

	FPathAssignedData* CurrentAssignment = PathAssignments.FindByPredicate(FindEntryPredicate);
	if (Icon.IsSet())
	{
		if (CurrentAssignment)
		{
			CurrentAssignment->Data.Icon = *Icon;
		}
		else
		{
			FPathAssignedData NewAssignment = {Path, {*Icon, AssetViewUtils::GetDefaultColor()}};
			PathAssignments.Emplace(NewAssignment);
		}
	}
	else
	{
		if (CurrentAssignment && !CurrentAssignment->Data.Color.Equals(AssetViewUtils::GetDefaultColor(), 0.1f))
		{
			CurrentAssignment->Data.Icon = FName("Default");
		}
		else
		{
			PathAssignments.RemoveAll(FindEntryPredicate);
		}
	}

	TryUpdateDefaultConfigFile();
}

void UFancyFoldersSettings::UpdateOrCreateAssignmentColor(const FString& Path, TOptional<FLinearColor> Color)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UFancyFoldersSettings::UpdateOrCreateAssignmentColor)

	auto FindEntryPredicate = [Path](const FPathAssignedData& Data)
	{
		return Data.Path == Path;
	};

	FPathAssignedData* CurrentAssignment = PathAssignments.FindByPredicate(FindEntryPredicate);
	if (Color.IsSet())
	{
		if (CurrentAssignment)
		{
			CurrentAssignment->Data.Color = *Color;
		}
		else
		{
			FPathAssignedData NewAssignment = {Path, {FName("Default"), *Color}};
			PathAssignments.Emplace(NewAssignment);
		}
	}
	else
	{
		if (CurrentAssignment && CurrentAssignment->Data.Icon != FName("Default"))
		{
			CurrentAssignment->Data.Color = Color.GetValue();
		}
		else
		{
			PathAssignments.RemoveAll(FindEntryPredicate);
		}
	}

	TryUpdateDefaultConfigFile();
}

void UFancyFoldersSettings::PreEditChange(FEditPropertyChain& PropertyAboutToChange)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UFancyFoldersSettings::PreEditChange)

	Super::PreEditChange(PropertyAboutToChange);

	FProperty* PropertyChanged = PropertyAboutToChange.GetActiveMemberNode()->GetValue();
	if (PropertyChanged && PropertyChanged->GetFName() == GET_MEMBER_NAME_CHECKED(UFancyFoldersSettings, PathAssignments))
	{
		for (FPathAssignedData Assignment : PathAssignments)
		{
			// Reset the color of all path assignments before the change takes effect 
			AssetViewUtils::SetPathColor(Assignment.Path, {});
		}
	}
}

void UFancyFoldersSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UFancyFoldersSettings::PostEditChangeProperty)

	Super::PostEditChangeProperty(PropertyChangedEvent);

	FProperty* PropertyChanged = PropertyChangedEvent.MemberProperty;
	if (PropertyChanged && PropertyChanged->GetFName() == GET_MEMBER_NAME_CHECKED(UFancyFoldersSettings, PathAssignments))
	{
		for (FPathAssignedData Assignment : PathAssignments)
		{
			// Re-apply the color of all path assignments after the change has taken effect
			AssetViewUtils::SetPathColor(Assignment.Path, Assignment.Data.Color);
		}
	}
}

FName UFancyFoldersSettings::GetContainerName() const
{
	return TEXT("Project");
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
