/**************************************************************************
* This file is part of the Fraqtive program.
* Copyright (C) 2005 Michal Mecinski.
* This program is licensed under the GNU General Public License.
**************************************************************************/

#include "gradient.h"

Gradient::Gradient() :
	_isHsv(false)
{
}

Gradient::Gradient(bool hsv) :
	_isHsv(hsv)
{
}

Gradient::~Gradient()
{
}

void Gradient::fillGradient(QRgb* buffer, int length, double* data) const
{
	double* temp = data ? data : new double[length * 3];
	double* t1 = temp;
	double* t2 = temp + length;
	double* t3 = temp + 2 * length;

	_spline[0].generateSpline(t1, length);
	_spline[1].generateSpline(t2, length);
	_spline[2].generateSpline(t3, length);

	if (_isHsv) {
		for (int i=0; i < length; i++)
			buffer[i] = hsvToRgb(t1[i], t2[i], t3[i]);
	} else {
		for (int i=0; i < length; i++)
			buffer[i] = qRgb((int)(t1[i] * 255.0), (int)(t2[i] * 255.0), (int)(t3[i] * 255.0));
	}

	if (!data)
		delete[] temp;
}

void Gradient::invert()
{
	for (int i=0; i<3; i++)
		_spline[i].invert();
}
