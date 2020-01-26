#include "las_curvenames_parser.h"

//#include <QRegularExpression>

LAS_CurveNames_Parser::LAS_CurveNames_Parser(QStringView sectionName, QStringView sectionBody)
{
	m_sectionName = sectionName;
	m_sectionBody = sectionBody;
	m_stop = false;
}

void LAS_CurveNames_Parser::parseSection()
{
	std::vector<QStringView> out;
	auto it = m_sectionBody.begin();
	while (it != m_sectionBody.end())
	{
		{
			QMutexLocker lock(&m_stop_signal);
			if (m_stop) return;
			auto eol = std::find(it, m_sectionBody.end(), "\n");
			// Just ignore important data, why not
			if (*it != "#")
			{
				auto sep = std::find(it, eol, ":");
				auto rNameEnd = std::find_if(std::reverse_iterator(sep),
																		 std::reverse_iterator(it),
																		 [](QStringView::const_reference ch)
				{ return ch != " " && ch != "\t" && ch !="."; });
				auto nameEnd = rNameEnd.base();
				if (nameEnd != it)
				{
					out.emplace_back(it, nameEnd);
				}
			}
			it = eol;
			if (it != m_sectionBody.end())
			{
				++it;
			}
		}
		emit progressNotifier(m_sectionName, it - m_sectionBody.begin());
	}
	emit parseFinished(m_sectionName, std::move(out));

	/*
	 * Perfectly functional regexp search,
	 * unfortunately we can't use regex with
	 * QStringView yet besides a lot of regex
	 * implementations are notoriously slow
	 */
	/*
	QRegularExpression re("^(?!#).+(?<cname>[\\w\\s?]\\.?\\s*:)",
												QRegularExpression::UseUnicodePropertiesOption);
	re.setPattern("(?<cname>[A-zА-я\\-\\(\\) ]+)\\.? *:");
	auto match = re.globalMatch(m_sectionText);
	while(match.hasNext())
	{
		auto nextMatch = match.next();
		out.emplace_back(nextMatch.captured("cname"));
	}
	*/
}

