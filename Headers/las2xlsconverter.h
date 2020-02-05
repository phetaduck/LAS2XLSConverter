#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMutexLocker>
#include <QFutureWatcher>

#include <vector>
#include <optional>

#include "las_file_parser.h"
#include "xls_exporter.h"

#include <mqtt_impl.h>

QT_BEGIN_NAMESPACE
namespace Ui { class LAS2XLSConverter; }
QT_END_NAMESPACE

class QListWidgetItem;
class QProgressDialog;

class LAS2XLSConverter : public QMainWindow
{
	Q_OBJECT

public:
	LAS2XLSConverter(QWidget *parent = nullptr);
	~LAS2XLSConverter();

private slots:
	void on_actionOpen_LAS_file_triggered();
	void on_actionExit_triggered();
	void on_lw_CurveNames_itemChanged(QListWidgetItem *item);

	void on_actionExport_As_triggered();

    void on_pbMQTTInit_clicked();

    void on_pbPublish_clicked();

public slots:
	void on_progressUpdate(int progress);
	void on_fileLoaded();
	void on_lasFileParsed(std::vector<LAS_Curve> curves);

	void on_fail(QString what);

	void stopExportInProgress();

    void onMQTTMsgRecieved(QString msg);

private:
	void stopParsing();

	void populateListWidget();
	void updateCurveSelection();

	QStringList selectedCurveNames();

	QString loadFile(QString filename);

private:
	Ui::LAS2XLSConverter *ui;

	QMutex m_tb_mutex;
	QMutex m_pb_mutex;
	QFutureWatcher<QString> m_fileLoadWatcher;
	QProgressDialog* m_xlsSavePD;

    MQTT_Impl mqttImpl = {};

	std::vector<LAS_Curve> m_curves;
	std::optional<LAS_File_Parser> m_converter;
	std::optional<XLS_Exporter> m_exporter;
	void updateTable(QStringList headers);
	void clearData();
};
#endif // MAINWINDOW_H
