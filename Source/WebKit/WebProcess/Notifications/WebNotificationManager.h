/*
 * Copyright (C) 2011, 2012, 2013 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "MessageReceiver.h"
#include "WebProcessSupplement.h"
#include <WebCore/NotificationClient.h>
#include <WebCore/ScriptExecutionContextIdentifier.h>
#include <optional>
#include <wtf/CompletionHandler.h>
#include <wtf/HashMap.h>
#include <wtf/Noncopyable.h>
#include <wtf/RefPtr.h>
#include <wtf/TZoneMalloc.h>
#include <wtf/UUID.h>
#include <wtf/Vector.h>
#include <wtf/WeakRef.h>
#include <wtf/text/StringHash.h>

namespace WebCore {
class SecurityOrigin;
class SecurityOriginData;

struct NotificationData;
}

namespace WebKit {

class WebPage;
class WebProcess;

class WebNotificationManager : public WebProcessSupplement, public IPC::MessageReceiver {
    WTF_MAKE_TZONE_ALLOCATED(WebNotificationManager);
    WTF_MAKE_NONCOPYABLE(WebNotificationManager);
public:
    explicit WebNotificationManager(WebProcess&);
    ~WebNotificationManager();

    void ref() const;
    void deref() const;

    static ASCIILiteral supplementName();
    
    bool show(WebCore::NotificationData&&, RefPtr<WebCore::NotificationResources>&&, WebPage*, CompletionHandler<void()>&&);
    void cancel(WebCore::NotificationData&&, WebPage*);

    void requestPermission(WebCore::SecurityOriginData&&, RefPtr<WebPage>, CompletionHandler<void(bool)>&&);

    // This callback comes from WebCore, not messaged from the UI process.
    void didDestroyNotification(WebCore::NotificationData&&, WebPage*);

    void didUpdateNotificationDecision(const String& originString, bool allowed);

    // Looks in local cache for permission. If not found, returns DefaultDenied.
    WebCore::NotificationClient::Permission policyForOrigin(const String& originString, WebPage* = nullptr) const;

    void removeAllPermissionsForTesting();

private:
    // WebProcessSupplement
    void initialize(const WebProcessCreationParameters&) override;

    // IPC::MessageReceiver
    // Implemented in generated WebNotificationManagerMessageReceiver.cpp
    void didReceiveMessage(IPC::Connection&, IPC::Decoder&) override;
    
    void didShowNotification(const WTF::UUID& notificationID);
    void didClickNotification(const WTF::UUID& notificationID);
    void didCloseNotifications(const Vector<WTF::UUID>& notificationIDs);
    void didRemoveNotificationDecisions(const Vector<String>& originStrings);

    WeakRef<WebProcess> m_process;
#if ENABLE(NOTIFICATIONS)
    HashMap<WTF::UUID, WebCore::ScriptExecutionContextIdentifier> m_nonPersistentNotificationsContexts;
    HashMap<String, bool> m_permissionsMap;
#endif
};

} // namespace WebKit
