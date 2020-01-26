#ifndef LAS2XLSCONVERTER_H
#define LAS2XLSCONVERTER_H

#include <map>
#include <vector>
#include <thread>
#include <memory>

#include <QString>
#include <QObject>
#include <QRunnable>
#include <QMutex>

#include "las_section_parser.h"
#include "las_curve.h"

using ParsedCurves = std::map<QString, std::vector<float>>;

class LAS_File_Parser : public QObject, public QRunnable
{
	Q_OBJECT

public:
	LAS_File_Parser() = default;
	virtual ~LAS_File_Parser() = default;
	void run();

public:
	LAS_File_Parser(QString fileContent);

	void setText(QString text);

	std::vector<QStringView> getSections() const {
		return m_sections;
	}

	void stop();

signals:
	void progressNotifier(int progress);
	void sectionsParsed(std::vector<QStringView> sections);
	void parseFinished(std::vector<LAS_Curve> curveData);
	void parseFailed(QString what);

private:
	void splitSections();
	void makeParsers();
	void startParsing();
	void composeCurves();

private slots:
	void sectionParseProgress(QStringView sectionName,
														long long progress);
	void sectionParseFinished(QStringView sectionName,
														std::vector<QStringView> result);

private:
	float m_progress = 0.0;
	bool m_stop = false;
	float m_totalSize = 0.0f;

	QString m_sectionMarker = "~";
	QString m_text;

	QMutex m_stop_signal;
	QMutex m_section_lock;

	std::vector<QStringView> m_sections;
	std::vector<QStringView> m_sectionsToParse;

	struct ParserInfo {
		std::unique_ptr<LAS_Section_Parser> parser = nullptr;
		std::vector<QStringView> parsedData;
		long long currentProg = 0;
		float progCoef = 1.0f;
	};

	std::map<QStringView, ParserInfo> m_parsers;

};

#endif // LAS2XLSCONVERTER_H
