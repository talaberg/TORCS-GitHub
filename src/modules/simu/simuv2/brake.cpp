/***************************************************************************

    file                 : brake.cpp
    created              : Sun Mar 19 00:05:26 CET 2000
    copyright            : (C) 2000-2013 by Eric Espie, Bernhard Wymann
    email                : torcs@free.fr
    version              : $Id: brake.cpp,v 1.8.2.2 2013/08/27 17:18:41 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "sim.h"

void SimBrakeConfig(void *hdle, const char *section, tBrake *brake)
{
	tdble diam, area, mu;

	diam     = GfParmGetNum(hdle, section, PRM_BRKDIAM, (char*)NULL, 0.2f);
	area     = GfParmGetNum(hdle, section, PRM_BRKAREA, (char*)NULL, 0.002f);
	mu       = GfParmGetNum(hdle, section, PRM_MU, (char*)NULL, 0.30f);
	brake->coeff = diam * 0.5 * area * mu;

	brake->I = GfParmGetNum(hdle, section, PRM_INERTIA, (char*)NULL, 0.13f);
	brake->radius = diam/2.0f;
}


void SimBrakeUpdate(tCar *car, tWheel *wheel, tBrake *brake)
{
	brake->Tq = brake->coeff * brake->pressure;

	brake->temp -= fabs(car->DynGC.vel.x) * 0.0001 + 0.0002;
	if (brake->temp < 0 ) brake->temp = 0;
	brake->temp += brake->pressure * brake->radius * fabs(wheel->spinVel) * 0.00000000005;
	if (brake->temp > 1.0) brake->temp = 1.0;
}


void SimBrakeSystemConfig(tCar *car)
{
	void *hdle = car->params;

	car->brkSyst.rep   = GfParmGetNum(hdle, SECT_BRKSYST, PRM_BRKREP, (char*)NULL, 0.5);
	car->brkSyst.coeff = GfParmGetNum(hdle, SECT_BRKSYST, PRM_BRKPRESS, (char*)NULL, 1000000);    
}


void SimBrakeSystemReConfig(tCar *car)
{
	tCarPitSetupValue* v = &car->carElt->pitcmd.setup.brakeRepartition;
	if (SimAdjustPitCarSetupParam(v)) {
		car->brkSyst.rep = v->value;
	}

	v = &car->carElt->pitcmd.setup.brakePressure;
	if (SimAdjustPitCarSetupParam(v)) {
		car->brkSyst.coeff = v->value;
	}
}


void SimBrakeSystemUpdate(tCar *car)
{
	tBrakeSyst	*brkSyst = &(car->brkSyst);
	tdble	ctrl = car->ctrl->brakeCmd;
	if(car->ctrl->BmeAbsEnable)
	{
		ctrl = car->ctrl->BmeAbsbrakeCmd[FRNT_RGT] * brkSyst->coeff;	// ABS-braking front wheels
		car->wheel[FRNT_RGT].brake.pressure = ctrl * brkSyst->rep;
		ctrl = car->ctrl->BmeAbsbrakeCmd[FRNT_LFT] * brkSyst->coeff;
		car->wheel[FRNT_LFT].brake.pressure = ctrl * brkSyst->rep;
		
		ctrl = car->ctrl->BmeAbsbrakeCmd[REAR_RGT] * brkSyst->coeff;	// ABS-braking rear wheels
		car->wheel[REAR_RGT].brake.pressure = ctrl * (1 - brkSyst->rep);
		ctrl = car->ctrl->BmeAbsbrakeCmd[REAR_LFT] * brkSyst->coeff;
		car->wheel[REAR_LFT].brake.pressure = ctrl * (1 - brkSyst->rep);
	}
	else
	{
		ctrl *= brkSyst->coeff;
		car->wheel[FRNT_RGT].brake.pressure = car->wheel[FRNT_LFT].brake.pressure = ctrl * brkSyst->rep;
		car->wheel[REAR_RGT].brake.pressure = car->wheel[REAR_LFT].brake.pressure = ctrl * (1 - brkSyst->rep);
	}
}
