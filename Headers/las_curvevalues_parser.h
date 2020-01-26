#ifndef CURVEDATA_H
#define CURVEDATA_H

#include "las_section_parser.h"

class LAS_CurveValues_Parser : public LAS_Section_Parser
{
	Q_OBJECT
public:
	LAS_CurveValues_Parser() = default;
	LAS_CurveValues_Parser(QStringView sectionName, QStringView sectionBody);
	virtual ~LAS_CurveValues_Parser() override = default;

	virtual void parseSection() override;

};

#endif // CURVEDATA_H
