/*
 * This file is a part of the Voice Call Manager project
 *
 * Copyright (C) 2011-2012  Tom Swindell <t.swindell@rubyx.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */
#include "common.h"

#include "voicecallmanagerdbusservice.h"
#include <dbus/voicecallmanagerdbusadapter.h>
#include <dbus/voicecallhandlerdbusadapter.h>

#include <voicecallmanagerinterface.h>

#include <QDBusError>
#include <QDBusConnection>

class VoiceCallManagerDBusServicePrivate
{
public:
    VoiceCallManagerDBusServicePrivate()
        : manager(NULL), managerAdapter(NULL)
    {/* ... */}

    VoiceCallManagerInterface *manager;
    VoiceCallManagerDBusAdapter *managerAdapter;
    QHash<QString,VoiceCallHandlerDBusAdapter*> handlerAdapters;
};

VoiceCallManagerDBusService::VoiceCallManagerDBusService(QObject *parent)
    : AbstractVoiceCallManagerPlugin(parent), d(new VoiceCallManagerDBusServicePrivate)
{
    TRACE
}

VoiceCallManagerDBusService::~VoiceCallManagerDBusService()
{
    TRACE
    delete this->d;
}

QString VoiceCallManagerDBusService::pluginId() const
{
    TRACE
    return "vcm-dbus-plugin";
}

QString VoiceCallManagerDBusService::pluginVersion() const
{
    TRACE
    return "0.0.0.1";
}

bool VoiceCallManagerDBusService::initialize()
{
    TRACE
    return true;
}

bool VoiceCallManagerDBusService::configure(VoiceCallManagerInterface *manager)
{
    TRACE
    d->manager = manager;
    d->managerAdapter = new VoiceCallManagerDBusAdapter(manager);

    if(!QDBusConnection::sessionBus().registerService("stage.rubyx.voicecall"))
    {

        qWarning() << "Failed to register DBus service:" << QDBusConnection::sessionBus().lastError().message();
        return false;
    }

    if(!QDBusConnection::sessionBus().registerObject("/", manager))
    {
        qWarning() << "Failed to register DBus object:" << QDBusConnection::sessionBus().lastError().message();
        return false;
    }

    QObject::connect(manager, SIGNAL(voiceCallAdded(AbstractVoiceCallHandler*)), SLOT(onVoiceCallAdded(AbstractVoiceCallHandler*)));
    QObject::connect(manager, SIGNAL(voiceCallRemoved(QString)), SLOT(onVoiceCallRemoved(QString)));
    QObject::connect(manager, SIGNAL(activeVoiceCallChanged()), SLOT(onActiveVoiceCallChanged()));

    d->managerAdapter->configure(manager);
    return true;
}

bool VoiceCallManagerDBusService::start()
{
    TRACE
    return true;
}

bool VoiceCallManagerDBusService::suspend()
{
    TRACE
    return true;
}

bool VoiceCallManagerDBusService::resume()
{
    TRACE
    return true;
}

void VoiceCallManagerDBusService::finalize()
{
    TRACE
}

void VoiceCallManagerDBusService::onVoiceCallAdded(AbstractVoiceCallHandler *handler)
{
    TRACE
    d->handlerAdapters.insert(handler->handlerId(), new VoiceCallHandlerDBusAdapter(handler));

    if(!QDBusConnection::sessionBus().registerObject("/calls" + handler->handlerId(), handler))
    {
        qWarning() << "Failed to register DBus object:" << QDBusConnection::sessionBus().lastError().message();
    }
}

void VoiceCallManagerDBusService::onVoiceCallRemoved(const QString &handlerId)
{
    TRACE
    QDBusConnection::sessionBus().unregisterObject("/calls" + handlerId);
    d->handlerAdapters.remove(handlerId);
}

void VoiceCallManagerDBusService::onActiveVoiceCallChanged()
{
    TRACE
    if(d->manager->activeVoiceCall())
    {
        qDebug() << "VoiceCallManagerDBusService:: registering active voice call interface.";
        QDBusConnection::sessionBus().registerObject("/calls/active", d->manager->activeVoiceCall());
    }
    else
    {
        QDBusConnection::sessionBus().unregisterObject("/calls/active");
    }
}