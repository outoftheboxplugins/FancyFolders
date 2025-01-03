// Copyright Out-of-the-Box Plugins 2018-2025. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class FancyFolders : ModuleRules
{
	public FancyFolders(ReadOnlyTargetRules Target) : base(Target)
	{
		// Hack to access the private Widgets from the ContentBrowser module
		PrivateIncludePaths.Add(Path.Combine(EngineDirectory, "Source/Editor/ContentBrowser/Private"));

		PrivateDependencyModuleNames.AddRange(new []
		{
			"AssetTools",
			"ContentBrowser",
			"ContentBrowserData",
			"Core",
			"CoreUObject",
			"DeveloperSettings",
			"EditorSubsystem",
			"Engine",
			"InputCore",
			"Projects", 
			"Slate",
			"SlateCore",
			"ToolMenus",
			"UnrealEd",
		});
	}
}
