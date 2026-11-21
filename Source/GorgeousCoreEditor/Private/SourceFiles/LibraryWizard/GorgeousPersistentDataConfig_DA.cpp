#include "LibraryWizard/GorgeousPersistentDataConfig_DA.h"
#include "Helpers/GorgeousPersistentData.h"
#include "UObject/UnrealType.h"

#if WITH_EDITOR
void UGorgeousPersistentDataConfig_DA::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName MemberName = PropertyChangedEvent.GetMemberPropertyName();
	if (MemberName == GET_MEMBER_NAME_CHECKED(UGorgeousPersistentDataConfig_DA, ValidationInterval))
	{
		FGorgeousPersistentData::Get().SetSystemValidationInterval(ValidationInterval);
	}
}
#endif