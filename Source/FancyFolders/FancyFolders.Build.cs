// Copyright Out-of-the-Box Plugins 2018-2024. All Rights Reserved.

using System.IO; //TODO: this is only needed for the PrivateIncludePaths, if that gets deleted, delete this too.
using UnrealBuildTool;

public class FancyFolders : ModuleRules
{
	public FancyFolders(ReadOnlyTargetRules Target) : base(Target)
	{

		//TODO: Find out if we actually use this
		PrivateIncludePaths.Add(Path.Combine(EngineDirectory, "Source/Editor/ContentBrowser/Private"));

		//TODO: Clean this up at the end to see what we actually use.
		PrivateDependencyModuleNames.AddRange(new []
		{
			"AssetTools",
			"ContentBrowser",
			"ContentBrowserData",
			"Core",
			"CoreUObject",
			"DeveloperSettings",
			"EditorStyle",
			"EditorSubsystem",
			"Engine",
			"Projects", 
			"Slate",
			"SlateCore",
			"ToolMenus",
			"UnrealEd",
			"InputCore",
		});
	}
}
