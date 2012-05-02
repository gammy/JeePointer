/* Tactor plugin interface
 * 	based on the PainterExample plugin by Bogdan Marinov
 * 
 * Copyright (C) 2012 gammy
 *
 * Useful links:
 * http://stellarium.org/doc/head/classStelMovementMgr.html
 *
 * TODO:
 * [X] Use StelMovementMgr.panView to adjust camera
 *     panView takes Azimuth, Altitude in radians
 * [ ] Get data from serial line (port my C ftdi code to C++)
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

	try
	{
		movementMgr = GETSTELMODULE(StelMovementMgr);

		databus.set_interface(INTERFACE_A);
		databus.open(0x0403, 0x6001);

		// TODO: Init FTDI?
	}
	catch (std::runtime_error &e)
	{
		qWarning() << "JeeScope::init() error: " << e.what();
		return;
	}
	
	//movementMgr->setMountMode(StelMovementMgr::MountAltAzimuthal);
	movementMgr->setEquatorialMount(false);

}

void JeeScope::deinit()
{
	qDebug() << "JeeScope::deinit()";
}

void JeeScope::update(double deltaTime)
{
	int tactor_x, tactor_y;
	if(! getAxes(&tactor_x, &tactor_y)) // No new axial data? Pass through, 
		return;                     // allowing the arrow keys to work.

	// Quantize values (input on both axes is -256 to 256)
	// Altitude: -90 to 90 degrees  (vertical angle)
	// Azimuth :   0 to 360 degrees (horizontal angle)
	double deg_azi = ((256 + tactor_x) / 512.0f) * 360.0f;
	double deg_alt = -90.0f + ((256 + tactor_y) / 512.0f) * 180;

	// Translate alt/azi angles to J200 & orient viewport (stolen from scripting system)
	Vec3d aim;
	double dAlt = StelUtils::getDecAngle(QString::number(deg_alt));
	double dAzi = M_PI - StelUtils::getDecAngle(QString::number(deg_azi));

	StelUtils::spheToRect(dAzi, dAlt, aim);
	float duration = 0; // Instantly move
	movementMgr->moveToJ2000(StelApp::getInstance().getCore()->altAzToJ2000(aim, StelCore::RefractionOff), duration);


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

int JeeScope::getAxes(int *tactor_x, int *tactor_y)
{

	int x = *tactor_x;
	int y = *tactor_y;

	// Read data
	// No new data?
	// return(0)

	// Clip
	if(x < -256)
		x = -256;
	if(x > 256)
		x = 256;

	if(y < -256)
		y = -256;
	if(y > 256)
		y = 256;

	*tactor_x = x;
	*tactor_y = y;

	return(1);
}

