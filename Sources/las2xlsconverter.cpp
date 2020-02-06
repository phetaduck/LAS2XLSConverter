#include "las2xlsconverter.h"
#include "ui_las2xlsconverter.h"
#include "xls_exporter.h"
#include "curve.pb.h"

#include <QFileDialog>
#include <QTextCodec>
#include <QThreadPool>
#include <QtConcurrent/QtConcurrentRun>
#include <QProgressDialog>
#include <QMessageBox>
#include <QDebug>

#include <algorithm>

LAS2XLSConverter::LAS2XLSConverter(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::LAS2XLSConverter)
{
	ui->setupUi(this);

	ui->pb_conversion->setRange(0, 100);
	ui->pb_conversion->setVisible(false);
	ui->actionExport_As->setEnabled(false);

	connect(&m_fileLoadWatcher, SIGNAL(finished()),
                    this, SLOT(on_fileLoaded()));

	qRegisterMetaType< std::vector<float> >( "std::vector<float>" );
    qRegisterMetaType< QStringView >( "QStringView"   );
    qRegisterMetaType< LAS2XLS::Curves >( "LAS2XLS::Curves"   );
    qRegisterMetaType< std::vector<QStringView> >( "std::vector<QStringView>"   );
    qRegisterMetaType< LAS2XLS::Curve >( "LAS2XLS::Curve "   );
}

LAS2XLSConverter::~LAS2XLSConverter()
{
	stopParsing();
	delete ui;
}


QString LAS2XLSConverter::loadFile(QString filename)
{
	QString out {};
	QFile lasFile(filename);
	if (lasFile.exists(filename))
	{
		lasFile.open(QFile::ReadOnly);

		QTextCodec *codec = QTextCodec::codecForName("Windows-1251");
		out = codec->toUnicode(lasFile.readAll());

		lasFile.close();
	}
	return out;
}

void LAS2XLSConverter::on_actionOpen_LAS_file_triggered()
{
	QString filename = QFileDialog::getOpenFileName(this, "Select LAS 3.0 file",
																									QDir::currentPath(),
																									"*.las");
	ui->actionOpen_LAS_file->setEnabled(false);
	auto future = QtConcurrent::run(this, &LAS2XLSConverter::loadFile, filename);
	m_fileLoadWatcher.setFuture(future);
}

void LAS2XLSConverter::on_actionExit_triggered()
{
	exit(0);
}

void LAS2XLSConverter::updateTable(QStringList headers)
{
	/*
	List/TableWidgets are easier to use but in production
	I would've opted for model based views
	to avoid full realod each time
	*/
	ui->tw_CurveData->clear();
	ui->tw_CurveData->setColumnCount(headers.size());
	if (ui->tw_CurveData->columnCount())
	{
		// qt insists on using signed ints for indecies
		ui->tw_CurveData->setRowCount(
                    static_cast<int>(m_curves.curves(0).values_size()));
		ui->tw_CurveData->setHorizontalHeaderLabels(headers);
		int column = 0;
		/*
		 I would rather iterate over headers
		 but QStringList doesn't support range iteration
		*/
        for (const auto& curve : m_curves.curves())
		{
            if (headers.contains(QString::fromStdString(curve.name())))
            {
                size_t rowCount = curve.values_size();
				for (size_t y = 0; y < rowCount; y++)
				{
                    QTableWidgetItem* item = new QTableWidgetItem(QString::fromStdString(curve.values(y)));
					ui->tw_CurveData->setItem(
								static_cast<int>(y), column, item);
				}
				column++;
			}
		}
	}
}

void LAS2XLSConverter::updateCurveSelection()
{
	QStringList headers = selectedCurveNames();
	if (!headers.size())
	{
		ui->actionExport_As->setEnabled(false);
	}
	updateTable(headers);
}

QStringList LAS2XLSConverter::selectedCurveNames()
{
	QStringList out;
	for (int i = 0; i < ui->lw_CurveNames->count(); i++)
	{
		auto item = ui->lw_CurveNames->item(i);
		if (item->checkState() == Qt::Checked)
		{
			out << item->text();
		}
	}
	return out;
}

void LAS2XLSConverter::on_progressUpdate(int progress)
{
	QMutexLocker locker(&m_pb_mutex);
	ui->pb_conversion->setValue(progress);
}

void LAS2XLSConverter::clearData()
{
	ui->lw_CurveNames->clear();
	ui->tw_CurveData->clear();
	ui->tw_CurveData->setColumnCount(0);
	ui->tw_CurveData->setRowCount(0);
    m_curves.Clear();
}

void LAS2XLSConverter::on_fileLoaded()
{
	auto text = m_fileLoadWatcher.result();
	if (text.length())
	{
		stopParsing();
		ui->pb_conversion->setVisible(true);
		m_converter.emplace(text);
		m_converter.value().setAutoDelete(false);
		QObject::connect(&m_converter.value(), &LAS_File_Parser::parseFinished,
										 this, &LAS2XLSConverter::on_lasFileParsed,
										 Qt::QueuedConnection);
		QObject::connect(&m_converter.value(), &LAS_File_Parser::progressNotifier,
										 this, &LAS2XLSConverter::on_progressUpdate,
										 Qt::QueuedConnection);
		QObject::connect(&m_converter.value(), &LAS_File_Parser::parseFailed,
										 this, &LAS2XLSConverter::on_fail,
										 Qt::QueuedConnection);
		QThreadPool::globalInstance()->start(&m_converter.value());
	}
	ui->actionOpen_LAS_file->setEnabled(true);
}

void LAS2XLSConverter::on_lasFileParsed(LAS2XLS::Curves curves)
{
	m_curves = curves;
	populateListWidget();
	ui->actionExport_As->setEnabled(true);
}

void LAS2XLSConverter::on_fail(QString what)
{
	stopParsing();
	QMessageBox alert;
	alert.setText(what);
	alert.exec();
}

void LAS2XLSConverter::stopParsing()
{
	if (m_converter.has_value())
	{
		QThreadPool::globalInstance()->clear();
		QObject::disconnect(&m_converter.value(), &LAS_File_Parser::parseFinished,
										 this, &LAS2XLSConverter::on_lasFileParsed);
		QObject::disconnect(&m_converter.value(), &LAS_File_Parser::progressNotifier,
										 this, &LAS2XLSConverter::on_progressUpdate);
		QObject::disconnect(&m_converter.value(), &LAS_File_Parser::parseFailed,
                                         this, &LAS2XLSConverter::on_fail);
		m_converter.value().stop();
		m_converter.reset();
		ui->pb_conversion->setValue(0);
		ui->pb_conversion->setVisible(false);
	}
}

void LAS2XLSConverter::populateListWidget()
{
	ui->lw_CurveNames->clear();
    for (const auto& curve : m_curves.curves())
	{
        QListWidgetItem* item = new QListWidgetItem(QString::fromStdString(curve.name()));
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
		item->setCheckState(Qt::Checked);
		ui->lw_CurveNames->addItem(item);
	}
	updateCurveSelection();
}

void LAS2XLSConverter::on_lw_CurveNames_itemChanged(QListWidgetItem *item)
{
	(void)item;
	updateCurveSelection();
}

void LAS2XLSConverter::stopExportInProgress()
{
	if (m_exporter.has_value())
	{
		m_exporter.value().stop();
		QThreadPool::globalInstance()->clear();
		if (m_xlsSavePD)
		{
			QObject::disconnect(&m_exporter.value(), &XLS_Exporter::fileSaved,
											 m_xlsSavePD, &QProgressDialog::reset);
			QObject::disconnect(&m_exporter.value(), &XLS_Exporter::progressNotifier,
											 m_xlsSavePD, &QProgressDialog::setValue);
			m_xlsSavePD->reset();
			delete m_xlsSavePD;
			m_xlsSavePD = nullptr;
		}
	}
}

void LAS2XLSConverter::on_actionExport_As_triggered()
{
    LAS2XLS::Curves testExport{};
     for (int i = 0; i < ui->lw_CurveNames->count(); i++)
     {
         auto item = ui->lw_CurveNames->item(i);
         if (item->checkState() == Qt::Checked)
         {
             auto curve = testExport.add_curves();
             curve->CopyFrom(m_curves.curves(i));
         }
     }

     mqttImpl.connectToBroker(ui->leHostName->text());
     mqttImpl.publish(ui->leTopic->text(), testExport.SerializeAsString());
     mqttImpl.start(false);
}

void LAS2XLSConverter::on_pbMQTTInit_clicked()
{
    mqttImpl.connectToBroker(ui->leHostName->text());
    connect(&mqttImpl, &MQTT_Impl::MessageRecieved,
            this, &LAS2XLSConverter::onMQTTMsgRecieved);
    mqttImpl.subscribe(ui->leTopic->text());
    connect(&mqttImpl, &MQTT_Impl::MessageRecieved,
            this, &LAS2XLSConverter::onMQTTMsgRecieved);
    mqttImpl.start(true);
}

void LAS2XLSConverter::onMQTTMsgRecieved(std::string msg)
{
    ui->textEdit->append(QString::fromStdString(msg));
}

void LAS2XLSConverter::on_pbPublish_clicked()
{
}
