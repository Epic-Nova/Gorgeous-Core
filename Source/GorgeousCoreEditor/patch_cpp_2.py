import re

path = '/media/aerien/Projects/SimsalabimStudio/Plugins/GorgeousThings/GorgeousShowcase/Plugins/Gorgeous-Core/Source/GorgeousCoreEditor/Private/SourceFiles/LibraryWizard/GorgeousUpdateManager.cpp'
with open(path, 'r') as f:
    content = f.read()

# Replace endpoint macro logic
old_endpoint_logic = """    FString Endpoint = bIsDevMode ? FString(TEXT("http://api.gorgeous.simsalabim.studio/api/v1/systems/update-check")) : FString(TEXT(GORGEOUS_API_ENDPOINT_UPDATE_CHECK));"""
new_endpoint_logic = """    FString Endpoint = bIsDevMode ? FString(TEXT("http://api.gorgeous.simsalabim.studio/api/v1/plugins/update-check")) : FString(TEXT(GORGEOUS_API_V1_ENDPOINT) TEXT("/plugins/update-check"));"""
content = content.replace(old_endpoint_logic, new_endpoint_logic)

# Replace response parsing
old_parse_logic = """            if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
            {
                JsonObject->TryGetBoolField(TEXT("UpdatesAvailable"), bUpdatesAvailable);
            }"""
new_parse_logic = """            if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
            {
                // Core version check
                FString CurrentCoreVersion = TEXT("0.0.0"); // Fallback
                // Since it's a map of PluginName -> UpdateData
                for (auto& Elem : JsonObject->Values)
                {
                    TSharedPtr<FJsonObject> PluginData = Elem.Value->AsObject();
                    if (PluginData.IsValid())
                    {
                        bool bAvail = false;
                        if (PluginData->TryGetBoolField(TEXT("UpdateAvailable"), bAvail) && bAvail)
                        {
                            FString MinCoreVer;
                            if (PluginData->TryGetStringField(TEXT("MinimumCoreVersion"), MinCoreVer) && !MinCoreVer.IsEmpty())
                            {
                                // A simplistic version check. In a real scenario you'd compare segments.
                                if (MinCoreVer > CurrentCoreVersion)
                                {
                                    // Block update
                                    // Could show toast: FNotificationInfo Info(FText::FromString(TEXT("Update Blocked: Core is outdated")));
                                }
                            }
                            bUpdatesAvailable = true;
                        }
                    }
                }
            }"""
content = content.replace(old_parse_logic, new_parse_logic)

with open(path, 'w') as f:
    f.write(content)
