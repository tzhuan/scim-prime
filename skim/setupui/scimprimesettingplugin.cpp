/***************************************************************************
 *   Copyright (C) 2003-2005 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

/*
 *  2005-06-26 Takuro Ashie <ashie@homa.ne.jp>
 *
 *    * Adapt to Prime IMEngine.
 */

#include "scimprimesettingplugin.h"

#include "prime.h"
#include "primeui.h"

#include <qcheckbox.h>

#include <kgenericfactory.h>
#include <klocale.h>

typedef KGenericFactory<ScimPrimeSettingPlugin> ScimPrimeSettingLoaderFactory;

K_EXPORT_COMPONENT_FACTORY( kcm_skimplugin_scim_prime, 
    ScimPrimeSettingLoaderFactory( "kcm_skimplugin_scim_prime" ) )

class ScimPrimeSettingPlugin::ScimPrimeSettingPluginPrivate {
public:
    PrimeSettingUI * ui;
};

ScimPrimeSettingPlugin::ScimPrimeSettingPlugin(QWidget *parent, 
					       const char */*name*/,
					       const QStringList &args)
 : KAutoCModule( ScimPrimeSettingLoaderFactory::instance(), 
     parent, args, PrimeConfig::self() ),
   d(new ScimPrimeSettingPluginPrivate)
{
    KGlobal::locale()->insertCatalogue("skim-scim-prime");
    d->ui = new PrimeSettingUI(this);
    setMainWidget(d->ui);
}

ScimPrimeSettingPlugin::~ScimPrimeSettingPlugin() 
{
    KGlobal::locale()->removeCatalogue("skim-scim-prime");
    delete d;
}


#include "scimprimesettingplugin.moc"
