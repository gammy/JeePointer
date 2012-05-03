/* JeePointer plugin interface
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

#include "JeePointer.hpp"

#include <sstream>

////////////////////////////////////////////////////////////////////////////////
// 
StelModule* JeePointerStelPluginInterface::getStelModule() const
{
	return new JeePointer();
}

StelPluginInfo JeePointerStelPluginInterface::getPluginInfo() const
{
	StelPluginInfo info;
	info.id = "JeePointer";
	info.displayedName = "JeePointer";
	info.authors = "gammy";
	info.contact = "gammy at pean dot org";
	info.description = "Provides Stellarium with spatial information from a JeePointer.";
	return info;
}

Q_EXPORT_PLUGIN2(JeePointer, JeePointerStelPluginInterface)


////////////////////////////////////////////////////////////////////////////////
// Constructor and destructor
JeePointer::JeePointer()
{
	setObjectName("JeePointer");
	font.setPixelSize(12);
}

JeePointer::~JeePointer()
{

}


////////////////////////////////////////////////////////////////////////////////
// Methods inherited from the StelModule class
// init(), update(), draw(), setStelStyle(), getCallOrder()
void JeePointer::init()
{
	initialized = false;

	qDebug() << "JeePointer::init()";

	try
	{
		movementMgr = GETSTELMODULE(StelMovementMgr);

		databus.set_interface(INTERFACE_A);

		int r = databus.open(0x0403, 0x6001);
		//if(r < 0) {
		//	std::ostringstream tmpErr;
		//	tmpErr << "FTDI set_interface  " << r << ": " << databus.error_string();
		//	throw std::runtime_error(tmpErr.str());
		//}

	}
	catch (std::exception &e)
	{
		qWarning() << "JeePointer::init() error: " << e.what();
		return;
	}
	

	databus.reset();
	databus.bitbang_disable();
	databus.set_baud_rate(57600);
	databus.set_latency(0);
	databus.set_read_chunk_size(32 * JEEPOINTER_BUF_SIZE);
	databus.set_write_chunk_size(32 * JEEPOINTER_BUF_SIZE);
	databus.flush(databus.Input);
		
	movementMgr->setEquatorialMount(false);

	initialized = true;
}

void JeePointer::deinit()
{
	qDebug() << "JeePointer::deinit()";
}

void JeePointer::update(double deltaTime)
{
	if(! initialized)
		return;

	int raw_x, raw_y;
	if(! getAxes(&raw_x, &raw_y)) // No new axial data? Pass through, 
		return;                     // allowing the arrow keys to work.

	//raw_x = 0;

	// Quantize values (input on both axes is -256 to 256)
	// Altitude: -90 to 90 degrees  (vertical angle)
	// Azimuth :   0 to 360 degrees (horizontal angle)
	double deg_azi = ((256 + raw_x) / 512.0f) * 360.0f;
	double deg_alt = -90.0f + ((256 + raw_y) / 512.0f) * 180;

	// Translate alt/azi angles to J200 & orient viewport (stolen from scripting system)
	Vec3d aim;
	double dAlt = StelUtils::getDecAngle(QString::number(deg_alt));
	double dAzi = M_PI - StelUtils::getDecAngle(QString::number(deg_azi));

	StelUtils::spheToRect(dAzi, dAlt, aim);
	float duration = 0; // Instantly move
	movementMgr->moveToJ2000(StelApp::getInstance().getCore()->altAzToJ2000(aim, StelCore::RefractionOff), duration);


}

void JeePointer::draw(StelCore* core)
{
	if(! initialized)
		return;

	StelPainter painter2D(core->getProjection2d());

	int w = core->getProjection2d()->getViewportWidth();
	int h = core->getProjection2d()->getViewportHeight();
	
	painter2D.setColor(1, 1, 1, 0.7);
	painter2D.setFont(font);
	painter2D.drawText((.5 * w) - 70, 
			   h - (2 * 12), 
			   "JeePointer mode");
	
	
}

double JeePointer::getCallOrder(StelModuleActionName actionName) const
{
	if (actionName == StelModule::ActionDraw)
		return StelApp::getInstance().getModuleMgr().getModule("LandscapeMgr")->getCallOrder(actionName) + 3.;
	return 0.;
}

bool JeePointer::configureGui(bool show)
{
	//This plug-in has no GUI for configuration
	return false;
}

int JeePointer::getAxes(int *raw_x, int *raw_y)
{

	int x = *raw_x;
	int y = *raw_y;
						
	int16_t x_new, y_new, z_new;
	x_new = x;
	y_new = y;

	int rb = databus.read(buf, JEEPOINTER_BUF_SIZE);
	if(rb > 0)
	{
		if(rb == JEEPOINTER_BUF_SIZE)
		{

			if(buf[JEEPOINTER_BUF_SIZE - 1] == 10 && 
			   buf[JEEPOINTER_BUF_SIZE - 2] == 13)
			{

				x_new = (buf[1] << 8) ^ buf[0];
				y_new = (buf[3] << 8) ^ buf[2];
				z_new = (buf[5] << 8) ^ buf[4];

			}
			else 
			{
				qWarning() << "Packet of right length but wrong signature";
			}
		}
		else if((unsigned) rb < JEEPOINTER_BUF_SIZE) 
		{ // Underrun? 
			qWarning() << "Packet of unknown size " << rb;
		}
		else 
		{
			abort();
		}
	} else if(rb < 0){
		qWarning() << "RX Error " << rb << ": " << databus.error_string();
	}

	// No new data?
	if(x == x_new || y == y_new) 
		return(0);

	x = x_new;
	y = y_new;

	// Clip
	if(x < -256)
		x = -256;
	if(x > 256)
		x = 256;

	if(y < -256)
		y = -256;
	if(y > 256)
		y = 256;

	*raw_x = x;
	*raw_y = y;

	return(1);
}

