/*
 * This file is a part of the Voice Call Manager Ofono Plugin project.
 *
 * Copyright (C) 2011-2012  Tom Swindell <t.swindell@rubyx.co.uk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
#include "common.h"
#include "ofonovoicecallproviderfactory.h"
#include "ofonovoicecallprovider.h"

#include <voicecallmanagerinterface.h>

#include <QtPlugin>
#include <ofono-qt/ofonomodem.h>
#include <ofono-qt/ofonomodemmanager.h>

class OfonoVoiceCallProviderFactoryPrivate
{
public:
    OfonoVoiceCallProviderFactoryPrivate()
        : isConfigured(false), ofonoModemManager(NULL), manager(NULL)
    {/* ... */}

    bool isConfigured;

    OfonoModemManager *ofonoModemManager;

    VoiceCallManagerInterface *manager;

    QHash<QString, OfonoVoiceCallProvider*> providers;
};

OfonoVoiceCallProviderFactory::OfonoVoiceCallProviderFactory(QObject *parent)
    : AbstractVoiceCallManagerPlugin(parent), d(new OfonoVoiceCallProviderFactoryPrivate)
{
    TRACE
}

OfonoVoiceCallProviderFactory::~OfonoVoiceCallProviderFactory()
{
    TRACE
    delete this->d;
}

QString OfonoVoiceCallProviderFactory::pluginId() const
{
    TRACE
    return PLUGIN_NAME;
}

QString OfonoVoiceCallProviderFactory::pluginVersion() const
{
    TRACE
    return PLUGIN_VERSION;
}

bool OfonoVoiceCallProviderFactory::initialize()
{
    TRACE
    d->ofonoModemManager = new OfonoModemManager(this);

    QObject::connect(d->ofonoModemManager, SIGNAL(modemAdded(QString)), SLOT(onModemAdded(QString)));
    QObject::connect(d->ofonoModemManager, SIGNAL(modemRemoved(QString)), SLOT(onModemRemoved(QString)));

    foreach(QString modemPath, d->ofonoModemManager->modems())
    {
        this->onModemAdded(modemPath);
    }

    return true;
}

bool OfonoVoiceCallProviderFactory::configure(VoiceCallManagerInterface *manager)
{
    TRACE
    if(d->isConfigured)
    {
        qWarning() << "OfonoVoiceCallProviderFactory is already configured!";
        return false;
    }

    d->manager = manager;

    foreach(OfonoVoiceCallProvider *provider, d->providers.values())
    {
        d->manager->appendProvider(provider);
    }

    d->isConfigured = true;
    return true;
}

bool OfonoVoiceCallProviderFactory::start()
{
    TRACE
    return true;
}

bool OfonoVoiceCallProviderFactory::suspend()
{
    TRACE
    return true;
}

bool OfonoVoiceCallProviderFactory::resume()
{
    TRACE
    return true;
}

void OfonoVoiceCallProviderFactory::finalize()
{
    TRACE
}

void OfonoVoiceCallProviderFactory::onModemAdded(const QString &modemPath)
{
    TRACE
    OfonoVoiceCallProvider *provider;

    if(d->providers.contains(modemPath))
    {
        qWarning() << "OfonoVoiceCallProviderFactory: Modem already registered" << modemPath;
        return;
    }

    provider = new OfonoVoiceCallProvider(modemPath, this);
    d->providers.insert(modemPath, provider);

    if(d->isConfigured)
    {
        d->manager->appendProvider(provider);
    }
}

void OfonoVoiceCallProviderFactory::onModemRemoved(const QString &modemPath)
{
    TRACE
    OfonoVoiceCallProvider *provider;

    if(!d->providers.contains(modemPath)) return;

    provider = d->providers.value(modemPath);
    d->providers.remove(modemPath);

    d->manager->removeProvider(provider);

    provider->deleteLater();
}

Q_EXPORT_PLUGIN2(voicecall-ofono-plugin, OfonoVoiceCallProviderFactory)