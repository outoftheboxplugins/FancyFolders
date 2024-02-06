// My super cool copyright notice

#include "FolderIconsStyle.h"

#include "Styling/SlateStyleRegistry.h"
#include "Framework/Application/SlateApplication.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FFolderIconsStyle::StyleInstance = nullptr;

void FFolderIconsStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FFolderIconsStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FFolderIconsStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("FolderIconsStyle"));
	return StyleSetName;
}

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);

TSharedRef< FSlateStyleSet > FFolderIconsStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("FolderIconsStyle"));

	const FString ResourcesFolder = IPluginManager::Get().FindPlugin("FolerIcons")->GetBaseDir() / TEXT("Resources");
	Style->SetContentRoot(ResourcesFolder);

	const FString IconsFolder = ResourcesFolder / TEXT("Icons"); 
	TArray<FString> FoundFiles;
	IFileManager::Get().FindFilesRecursive(FoundFiles, *IconsFolder, TEXT("*.svg"), true, false);

	for(const auto& File : FoundFiles)
	{
		FString Filename = FPaths::GetBaseFilename(File);
		const FString Property = FString::Printf(TEXT("FolderIcons.%s"), *Filename);
		const FString Brush = FString::Printf(TEXT("Icons/%s"), *Filename); 
		Style->Set( FName(Property), new IMAGE_BRUSH_SVG(Brush, FVector2D(64, 64)));
	}

	return Style;
}

void FFolderIconsStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FFolderIconsStyle::Get()
{
	return *StyleInstance;
}
