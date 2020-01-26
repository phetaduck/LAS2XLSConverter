#ifndef CURVEPARSER_H
#define CURVEPARSER_H

#include "las_section_parser.h"

class LAS_CurveNames_Parser : public LAS_Section_Parser
{
	Q_OBJECT
public:
	LAS_CurveNames_Parser() = default;
	virtual ~LAS_CurveNames_Parser() override = default;

	LAS_CurveNames_Parser(QStringView sectionName, QStringView sectionBody);

	virtual void parseSection() override;
};

#endif // CURVEPARSER_H
