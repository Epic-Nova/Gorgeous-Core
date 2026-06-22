import re

path = '/media/aerien/Projects/SimsalabimStudio/Plugins/GorgeousThings/GorgeousShowcase/Plugins/Gorgeous-Core/Source/GorgeousCoreEditor/Private/SourceFiles/LibraryWizard/GorgeousUpdateManager.cpp'
with open(path, 'r') as f:
    content = f.read()

# Make sure we have the includes for notifications
includes = """#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
"""
if "SNotificationList.h" not in content:
    content = content.replace('#include "SourceFiles/LibraryWizard/GorgeousUpdateManager.h"', '#include "SourceFiles/LibraryWizard/GorgeousUpdateManager.h"\n' + includes)

old_toast = """                                    // Block update
                                    // Could show toast: FNotificationInfo Info(FText::FromString(TEXT("Update Blocked: Core is outdated")));"""

new_toast = """                                    // Block update
                                    bUpdatesAvailable = false;
                                    FNotificationInfo Info(FText::FromString(FString::Printf(TEXT("Update blocked for %s: Minimum Core Version %s required."), *Elem.Key, *MinCoreVer)));
                                    Info.ExpireDuration = 5.0f;
                                    FSlateNotificationManager::Get().AddNotification(Info);
                                    continue;"""

content = content.replace(old_toast, new_toast)

with open(path, 'w') as f:
    f.write(content)
