#include "xls_exporter.h"

#include "xlsxdocument.h"

#include <QFile>
#include <QTextStream>
#include <QTextCodec>
#include <QRunnable>

XLS_Exporter::~XLS_Exporter() = default;

XLS_Exporter::XLS_Exporter(QString fileName,
													 const std::vector<LAS_Curve>& curves,
													 std::vector<size_t> indecies) :
	m_fileName(std::move(fileName)), m_curves(curves), m_indecies(std::move(indecies))
{}

void XLS_Exporter::stop()
{
	QMutexLocker lock(&m_stop_signal);
	m_stop = true;
}

void XLS_Exporter::run()
{
	saveCurves();
	emit fileSaved();
}

void XLS_Exporter::saveCurves()
{
	QXlsx::Document xlsx;
	float totalSize = m_indecies.size() * m_curves.begin()->values().size();
	float currentProg = 1.0f;
	float adjProgress = 0.0f;
	int column = 1;
	for (auto index : m_indecies) {
		const auto& c = m_curves[index];
		int row = 1;
		xlsx.write(row, column, c.name());
		for (const auto& v : c.values())
		{
			{
				QMutexLocker lock(&m_stop_signal);
				if (m_stop) return;
				xlsx.write(++row, column, v);
				adjProgress = 100.0f * (currentProg++ / totalSize);
			}
			emit progressNotifier(int(adjProgress));
		}
		column++;
	}
	xlsx.saveAs(m_fileName);
}
