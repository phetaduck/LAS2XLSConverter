#ifndef LAS_WELL_H
#define LAS_WELL_H

#include "las_curve.h"

class LAS_Well
{
public:
	LAS_Well();

	const auto& curves() {
		return m_curves;
	}

private:
	std::vector<LAS_Curve> m_curves;
};

#endif // LAS_WELL_H
