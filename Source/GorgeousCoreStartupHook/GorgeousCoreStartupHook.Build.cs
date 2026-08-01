using UnrealBuildTool;

public class GorgeousCoreStartupHook : GorgeousModuleRules
{
	public GorgeousCoreStartupHook(ReadOnlyTargetRules Target) : base(Target)
	{
        ApplyGorgeousBuildSettings(new GorgeousBuildSettings {
            TargetModuleType = GorgeousModuleType.Editor,
            ModulesToExclude = new[] { "GorgeousCoreStartupHook" }
        });

		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Json",
				"JsonUtilities",
				"Projects",
				"ApplicationCore"
			}
		);
	}
}
