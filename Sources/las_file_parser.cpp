#include "las_file_parser.h"

#include "las_curve.h"
#include "supported_sections.h"

#include <math.h>

#include <QStringList>
#include <QMutexLocker>
#include <QThreadPool>

#include <QDebug>

void LAS_File_Parser::run()
{
	try {
		splitSections();
		makeParsers();
		startParsing();
	} catch (std::exception& e) {
		emit parseFailed(QString::fromStdString(e.what()));
	}
}

LAS_File_Parser::LAS_File_Parser(QString fileContent) :
	m_text(std::move(fileContent))
{
}

void LAS_File_Parser::setText(QString text)
{
	m_text = std::move(text);
}

void LAS_File_Parser::stop()
{
	QMutexLocker lock{&m_stop_signal};
	m_stop = true;
	for (auto& [name, parserInfo] : m_parsers)
	{
		(void)name;
		QThreadPool::globalInstance()->clear();
		QObject::disconnect(parserInfo.parser.get(), &LAS_Section_Parser::progressNotifier,
												this, &LAS_File_Parser::sectionParseProgress);
		QObject::disconnect(parserInfo.parser.get(), &LAS_Section_Parser::parseFinished,
												this, &LAS_File_Parser::sectionParseFinished);
		parserInfo.parser->stop();
	}
}

void LAS_File_Parser::splitSections()
{
	/*
	 I would rather use boost spirit for parsing
	 but I'm supposed to show my ability to do that from scratch
	 and while I'm on it why not eliminate any possible data copy
	*/
	auto sectionMarkerIndex = m_text.indexOf(m_sectionMarker);
	auto it = m_text.begin();
	while (sectionMarkerIndex != -1)
	{
		{
			QMutexLocker lock{&m_stop_signal};
			if (m_stop) return;
			if (sectionMarkerIndex) {
				QStringView newSection {it, m_text.begin() + sectionMarkerIndex - 1};
				m_sections.emplace_back(newSection);
				it = m_text.begin() + sectionMarkerIndex;
			}
			sectionMarkerIndex++;
			/*
		 At this point there is no real way to tell
		 actual progress although this operation can take a noticable amount
		 of time to complete, depending on the file size
		 so let's just assume it's 5%
		*/
			m_progress = (sectionMarkerIndex / m_text.length()) * 0.05f;
			sectionMarkerIndex = m_text.indexOf(m_sectionMarker, sectionMarkerIndex);
			// in qt string serach returns -1 if not found
			if (sectionMarkerIndex < 0)
			{
				QStringView lastSection {it, m_text.end()};
				m_sections.emplace_back(lastSection);
				m_progress = 5.0f;
			}
		}
		emit progressNotifier(static_cast<int>(m_progress));
	}
	if (!m_sections.size())
		throw std::logic_error("no sections found");
}

void LAS_File_Parser::makeParsers()
{
	for (auto s : m_sections)
	{
		QMutexLocker lock{&m_stop_signal};
		if (m_stop) return;
		auto parser = LAS_Section_Parser::makeSectionParser(s);
		if (parser)
		{
			QObject::connect(parser.get(), &LAS_Section_Parser::progressNotifier,
											 this, &LAS_File_Parser::sectionParseProgress);
			QObject::connect(parser.get(), &LAS_Section_Parser::parseFinished,
											 this, &LAS_File_Parser::sectionParseFinished);
			parser->setAutoDelete(false);
			ParserInfo& pInfo = m_parsers[parser->sectionName()];
			pInfo.parser = std::move(parser);
			m_totalSize += pInfo.parser->sectionBody().size();
			m_sectionsToParse.emplace_back(pInfo.parser->sectionName());
		}
	}
}

void LAS_File_Parser::startParsing()
{
	for (auto& [name, pInfo]: m_parsers)
	{
		QMutexLocker lock{&m_stop_signal};
		if (m_stop) return;
		(void)name;
		// account for 5% designated to splitting sections
		pInfo.progCoef = pInfo.parser->sectionBody().size() / m_totalSize * 0.85f;
		QThreadPool::globalInstance()->start(pInfo.parser.get());
	}
}

void LAS_File_Parser::composeCurves()
{
	// hardcode
	auto& curveNames = m_parsers.at(s_curve_names_section).parsedData;
	auto& curveValues = m_parsers.at(s_curve_data_section).parsedData;
    LAS2XLS::Curves out{};
    //out.reserve(curveNames.size());
	/*
	 * A bunch of ugly code that could've been avoided with boost spirit
	 * and better design
	 */

	float totalSize = curveValues.size();
	float currentProg = 0.0f;
	float adjProgress = 0.0f;
	for (size_t column = 0;
			 column < curveNames.size();
			 column++)
	{
		QMutexLocker lock{&m_stop_signal};
		if (m_stop) return;
        auto curve = out.add_curves();
		auto& cn = curveNames[column];
#pragma message("Warning possible dangling pointer")
        //auto std_name = cn.toString().toStdString();
        //curve->set_name(std::move(std_name));
        curve->set_name(cn.toUtf8().toStdString());
        //std::vector<QString> values{};
        //values.reserve(curveValues.size() / curveNames.size());
		for (size_t row = 0;
				 row < curveValues.size();
				 row += curveNames.size())
		{
			auto& value = curveValues[row + column];
            curve->add_values(value.toUtf8().toStdString());
            //values.emplace_back(value.data(), static_cast<int>(value.size()));
			adjProgress = 10.0f * (currentProg++ / totalSize);
		}
        //out.emplace_back(name, values);
		emit progressNotifier(static_cast<int>(ceil(m_progress + adjProgress)));
	}
	emit parseFinished(std::move(out));
}

void LAS_File_Parser::sectionParseProgress(QStringView sectionName,
																					 long long progress)
{
	auto& pInfo = m_parsers[sectionName];
	float delta = progress - pInfo.currentProg;
	m_progress += (delta / pInfo.parser->sectionBody().size()) *
								pInfo.progCoef * 100.0f;
	pInfo.currentProg = progress;
	emit progressNotifier(static_cast<int>(ceil(m_progress)));
}

void LAS_File_Parser::sectionParseFinished(
		QStringView sectionName,
		std::vector<QStringView> result)
{
	QMutexLocker lock(&m_section_lock);
	m_parsers[sectionName].parsedData = std::move(result);
	auto it = std::find(m_sectionsToParse.begin(),
											m_sectionsToParse.end(),
											sectionName);
	/*
	 it may throw here but if it does something is wrong and
	 we should throw here
	*/
	m_sectionsToParse.erase(it);
	if (!m_sectionsToParse.size()) {
		composeCurves();
	}
}
