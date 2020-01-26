#ifndef XLS_EXPORTER_H
#define XLS_EXPORTER_H

#include <QObject>
#include <QRunnable>
#include <QMutex>

#include <vector>

#include "las_curve.h"

class XLS_Exporter : public QObject, public QRunnable
{
	Q_OBJECT

public:
	XLS_Exporter() = default;
	virtual ~XLS_Exporter() override;

	XLS_Exporter(QString fileName,
							 const std::vector<LAS_Curve>& curves,
							 std::vector<size_t> indecies);

	void stop();
	void run() override;

signals:

	void progressNotifier (int value);
	void fileSaved();

private:
	void saveCurves();

private:
	QString m_fileName;
	const std::vector<LAS_Curve>& m_curves;
	std::vector<size_t> m_indecies;

	QMutex m_stop_signal;
	bool m_stop = false;
};

#endif // XLS_EXPORTER_H
