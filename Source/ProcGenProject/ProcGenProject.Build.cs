// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ProcGenProject : ModuleRules
{
	public ProcGenProject(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "GeometricObjects", "RuntimeMeshComponent"});

		PrivateDependencyModuleNames.AddRange(new string[] { "ProceduralMeshComponent" });

		//PublicIncludePaths.AddRange(new string[] { "<GeometricObjects>/Public" });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
