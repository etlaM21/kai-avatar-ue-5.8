#include "MediaPipeLiveLink.h"
#include "MediaPipeLiveLinkSource.h"
#include "Features/IModularFeatures.h"
#include "ILiveLinkClient.h"
#include "Misc/CoreDelegates.h"

#define LOCTEXT_NAMESPACE "FMediaPipeLiveLinkModule"

void FMediaPipeLiveLinkModule::StartupModule()
{
    // The Live Link client only exists once the engine is up.
    PostEngineInitHandle = FCoreDelegates::GetOnPostEngineInit().AddRaw(this, &FMediaPipeLiveLinkModule::AddDefaultSource);
}

void FMediaPipeLiveLinkModule::ShutdownModule()
{
    FCoreDelegates::GetOnPostEngineInit().Remove(PostEngineInitHandle);
}

// Live Link does not remember sources between editor sessions (only a preset would), so
// without this the receiver had to be added by hand every time, and until then nothing
// listened on 9001. Adds one receiver on 127.0.0.1:9001 unless one of ours already exists.
void FMediaPipeLiveLinkModule::AddDefaultSource()
{
    IModularFeatures& Features = IModularFeatures::Get();
    if (!Features.IsModularFeatureAvailable(ILiveLinkClient::ModularFeatureName)) return;
    ILiveLinkClient& Client = Features.GetModularFeature<ILiveLinkClient>(ILiveLinkClient::ModularFeatureName);

    const FString OurType(FMediaPipeLiveLinkSource::SourceTypeName);
    for (const FGuid& SourceGuid : Client.GetSources())
    {
        if (Client.GetSourceType(SourceGuid).ToString() == OurType) return;
    }

    Client.AddSource(MakeShared<FMediaPipeLiveLinkSource>(FMediaPipeLiveLinkSettings()));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FMediaPipeLiveLinkModule, MediaPipeLiveLink)
