#include "las_curvevalues_parser.h"

#include <QMutexLocker>


LAS_CurveValues_Parser::LAS_CurveValues_Parser(QStringView sectionName, QStringView sectionBody)
{
	m_sectionName = sectionName;
	m_sectionBody = sectionBody;
	m_stop = false;
}

void LAS_CurveValues_Parser::parseSection()
{
	std::vector<QStringView> out;

	auto it = m_sectionBody.begin();
	auto eol = std::find(it, m_sectionBody.end(), "\n");

	/*
	 Let us reserve memory ahead of time
	 since there will be a lot of data,
	 so allocation and copy of std::vector
	 may slow down performance unnecessarily.
	 Assumption: comparing chars across
	 contigious data that can be prefetched into CPU cache
	 is faster than heap memory allocation.
	 Needs more testing though.
	*/
	auto spaceLambda = [](QStringView::const_reference ch) {
		return ch == " " || ch == "\t";
	};
	auto colNum = std::count_if(it, eol,
															spaceLambda);
	auto rowNum = std::count(m_sectionBody.begin(),
													 m_sectionBody.end(),
													 "\n");
	out.reserve(
				static_cast<std::vector<QStringView>::size_type>(
					colNum * rowNum
					)
				);
	while (it != m_sectionBody.end())
	{
		{
			QMutexLocker lock(&m_stop_signal);
			if (m_stop) return;
			while (it != eol)
			{
				auto sep = std::find_if(it, eol, spaceLambda);
				if (sep != eol)
				{
					out.emplace_back(it, sep-1);
					++sep;
				}
				it = sep;
			}
			if (eol != m_sectionBody.end())
			{
				it = ++eol;
				eol = std::find(eol, m_sectionBody.end(), "\n");
			}
		}
		emit progressNotifier(m_sectionName, it - m_sectionBody.begin());
	}
	emit parseFinished(m_sectionName, std::move(out));
}
