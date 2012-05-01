/* Tactor plugin interface
 * 	based on the PainterExample plugin by Bogdan Marinov
 * 
 * Copyright (C) 2012 gammy
 *
 * Useful links:
 * http://stellarium.org/doc/head/classStelMovementMgr.html
 *
 * TODO:
 * [ ] Lock keys if loaded?
 * [ ] Use StelMovementMgr.panView to adjust camera
 *     panView takes Azimuth, Altitude in radians
 * [ ] Get data from serial line
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "JeeScope.hpp"

#include "StelApp.hpp"
#include "StelModuleMgr.hpp"

//Both necessary for drawing:
#include "StelCore.hpp"
#include "StelPainter.hpp"
#include "VecMath.hpp" //For coordinate vectors

#include <QDebug>
#include <stdexcept>


////////////////////////////////////////////////////////////////////////////////
// 
StelModule* JeeScopeStelPluginInterface::getStelModule() const
{
	return new JeeScope();
}

StelPluginInfo JeeScopeStelPluginInterface::getPluginInfo() const
{
	StelPluginInfo info;
	info.id = "JeeScope";
	info.displayedName = "JeeScope Tactor";
	info.authors = "gammy";
	info.contact = "gammy at pean dot org";
	info.description = "Spatial awareness module. Allows Stellarium to obtain spatial information from the wireless Tactor.";
	return info;
}

Q_EXPORT_PLUGIN2(JeeScope, JeeScopeStelPluginInterface)


////////////////////////////////////////////////////////////////////////////////
// Constructor and destructor
JeeScope::JeeScope()
{
	setObjectName("JeeScope");
	font.setPixelSize(12);
}

JeeScope::~JeeScope()
{

}


////////////////////////////////////////////////////////////////////////////////
// Methods inherited from the StelModule class
// init(), update(), draw(), setStelStyle(), getCallOrder()
void JeeScope::init()
{
	qDebug() << "JeeScope::init()";

	movementMgr = GETSTELMODULE(StelMovementMgr);

	movementMgr->setMountMode(StelMovementMgr::MountAltAzimuthal);
	tactor_x = tactor_y = 0.0f;



	try
	{

		// TODO: Init FTDI?
	}
	catch (std::runtime_error &e)
	{
		qWarning() << "JeeScope::init() error: " << e.what();
		return;
	}
}

void JeeScope::deinit()
{
	qDebug() << "JeeScope::deinit()";
}

void JeeScope::update(double deltaTime)
{
	tactor_x = 0.01f;
	//tactor_x += 0.00001f;

	//if(tactor_x > 1.0f)
	//	tactor_x = -1.0f;

	movementMgr->panView(tactor_x, tactor_y);

}

void JeeScope::draw(StelCore* core)
{
	StelPainter painter2D(core->getProjection2d());

	int w = core->getProjection2d()->getViewportWidth();
	int h = core->getProjection2d()->getViewportHeight();
	
	painter2D.setColor(1, 1, 1, 0.7);
	painter2D.setFont(font);
	painter2D.drawText((.5 * w) - 70, 
			   h - (2 * 12), 
			   "JeeScope mode");
	
	
}

double JeeScope::getCallOrder(StelModuleActionName actionName) const
{
	if (actionName == StelModule::ActionDraw)
		return StelApp::getInstance().getModuleMgr().getModule("LandscapeMgr")->getCallOrder(actionName) + 3.;
	return 0.;
}

bool JeeScope::configureGui(bool show)
{
	//This plug-in has no GUI for configuration
	return false;
}

