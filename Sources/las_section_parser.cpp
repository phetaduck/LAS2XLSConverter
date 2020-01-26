#include "las_section_parser.h"
#include "las_curvenames_parser.h"
#include "las_curvevalues_parser.h"
#include "supported_sections.h"

#include <exception>
#include <algorithm>

#include <QRegularExpression>

std::pair<QStringView, QStringView> splitSection(QStringView sectionText)
{
	// we need to find eol to separate the first line
	auto eol = std::find(sectionText.begin(), sectionText.end(), "\n");
	if (eol != sectionText.end()) {
		// and get the range of the section name
		auto it = std::find_if(std::reverse_iterator(eol), sectionText.rend(),
															 [](QStringView::const_reference c){
							return c != " " && c != "\t" && c != "\r";});
		if (it != sectionText.rend())
		{
			/*
			 being so frivolous with reverse_iterator
			 is not a good idea in the production code
			 but it works for our case
			*/
			QStringView sectionName {sectionText.rend().base(), it.base()};
			if (++eol != sectionText.end())
			{
				QStringView sectionBody{eol, sectionText.end()};
				return {sectionName, sectionBody};
			}
		}
	}
	throw std::invalid_argument("cannot split section");
}

void LAS_Section_Parser::run()
{
	parseSection();
}

void LAS_Section_Parser::stop()
{
	QMutexLocker lock(&m_stop_signal);
	m_stop = true;
}

std::unique_ptr<LAS_Section_Parser> LAS_Section_Parser::makeSectionParser(QStringView sectionText)
{
	auto [name, body] = splitSection(sectionText);
	// it would be nice not to use hardcoded constants
	if (s_curve_names_section.compare(name) == 0)
	{
		return std::make_unique<LAS_CurveNames_Parser>(name, body);
	}
	else if (s_curve_data_section.compare(name) == 0)
	{
		return std::make_unique<LAS_CurveValues_Parser>(name, body);
	}
	/*
	 in production it would be better
	 to create an exception subclass
	 or to not use exceptions for performance reasons

	 throw std::invalid_argument("unsupported section");

	 But having unsupported sections is expected for now.
	*/
	return nullptr;
}

