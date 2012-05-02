/* Tactor plugin interface
 * 	based on the PainterExample plugin by Bogdan Marinov
 * 
 * Copyright (C) 2012 gammy
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

#ifndef _JEESCOPE_HPP_
#define _JEESCOPE_HPP_

#define JEESCOPE_BUF_SIZE (unsigned int) 8

#include "StelModule.hpp"
#include "StelMovementMgr.hpp"
#include "StelApp.hpp"
#include "StelModuleMgr.hpp"
#include "StelCore.hpp"
#include "StelUtils.hpp"
#include "StelPainter.hpp"
#include "VecMath.hpp" //For coordinate vectors

#include <stdint.h>
#include <ftdi.hpp>

#include <stdexcept>

#include <QDebug>
#include <QFont>

class JeeScope : public StelModule
{
	Q_OBJECT

public:
	JeeScope();
	virtual ~JeeScope();
	
	virtual void init();
	virtual void deinit();
	virtual void update(double deltaTime);
	virtual void draw(StelCore * core);
	virtual double getCallOrder(StelModuleActionName actionName) const;
	virtual bool configureGui(bool show);
	///////////////////////////////////////////////////////////////////////////
	virtual int getAxes(int *x, int *y);
	
public slots:
	
private:
	QFont font;
	StelMovementMgr *movementMgr;
	Ftdi::Context databus;
	bool initialized;
	unsigned char buf[JEESCOPE_BUF_SIZE];
};


#include "fixx11h.h"
#include <QObject>
#include "StelPluginInterface.hpp"

//! This class is used by Qt to manage a plug-in interface
class JeeScopeStelPluginInterface : public QObject, public StelPluginInterface
{
	Q_OBJECT
	Q_INTERFACES(StelPluginInterface)
public:
	virtual StelModule* getStelModule() const;
	virtual StelPluginInfo getPluginInfo() const;
};

#endif /*_JEESCOPE_HPP_*/
