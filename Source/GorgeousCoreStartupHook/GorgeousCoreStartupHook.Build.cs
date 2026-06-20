using UnrealBuildTool;

public class GorgeousCoreStartupHook : ModuleRules
{
	public GorgeousCoreStartupHook(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
			}
			);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Json",
				"JsonUtilities",
				"Projects",
				"ApplicationCore"
			}
			);
	}
}
