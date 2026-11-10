#pragma once

#include "CoreMinimal.h"
#include "ILiveLinkSource.h"
#include "OSCServer.h"
#include "UObject/GCObject.h"
#include "Interfaces/IPv4/IPv4Endpoint.h"

class ILiveLinkClient;

struct FMediaPipeLiveLinkSettings
{
    FIPv4Endpoint LocalEndpoint;
    // Bound to all interfaces on purpose: Epic's OSC receiver replaces a loopback bind
    // address with the machine's LAN IP (SetAsLocalHostIfLoopback in OSCServerReceiver.cpp),
    // and a socket bound there never sees packets sent to 127.0.0.1. Loopback-only is
    // enforced per packet instead, in OnOSCMessageReceived.
    FMediaPipeLiveLinkSettings() : LocalEndpoint(FIPv4Address::Any, 9001) {}
};

// Inherit FGCObject to prevent the garbage collector from destroying our UOSCServer
class MEDIAPIPELIVELINK_API FMediaPipeLiveLinkSource : public ILiveLinkSource, public FGCObject
{
public:
    FMediaPipeLiveLinkSource(const FMediaPipeLiveLinkSettings& InSettings);
    virtual ~FMediaPipeLiveLinkSource();

    virtual void ReceiveClient(ILiveLinkClient* InClient, FGuid InSourceGuid) override;
    virtual void Update() override {}
    virtual bool IsSourceStillValid() const override { return true; }
    virtual bool RequestSourceShutdown() override;
    virtual FText GetSourceType() const override { return FText::FromString(SourceTypeName); }
    virtual FText GetSourceMachineName() const override { return FText::FromString(Settings.LocalEndpoint.ToString()); }
    virtual FText GetSourceStatus() const override;

    virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
    virtual FString GetReferencerName() const override { return "FMediaPipeLiveLinkSource"; }

    // Shown in the Live Link window; the module also uses it to see whether a source of
    // ours already exists before it adds one at startup.
    static const TCHAR* const SourceTypeName;

private:
    void OnOSCMessageReceived(const FOSCMessage& Message, const FString& IPAddress, uint16 Port);
    void CreateSubject(const FName& InSubjectName);

    ILiveLinkClient* Client;
    FGuid SourceGuid;
    FMediaPipeLiveLinkSettings Settings;

    TObjectPtr<UOSCServer> OSCServer;

    TSet<FName> RegisteredSubjects;
    FCriticalSection SubjectsCriticalSection;
};