#ifndef SECTIONPARSER_H
#define SECTIONPARSER_H

#include <memory>
#include <vector>

#include <QString>
#include <QObject>
#include <QRunnable>
#include <QMutex>

class LAS_Section_Parser : public QObject, public QRunnable
{
	Q_OBJECT
protected:
	LAS_Section_Parser() = default;
	LAS_Section_Parser(QStringView sectionName, QStringView sectionBody);

public:
	virtual ~LAS_Section_Parser() override = default;

signals:
	void progressNotifier(QStringView sectionName,
												long long progress);
	void parseFinished(QStringView sectionName,
										 std::vector<QStringView> result);
	void parseFailed(QString what);

public:
	/*
	 I don't think it's a good practice to have
	 a non pure virtual method in an abstract class
	 but oh well, it's convinient sometimes
	*/
	virtual void run() override;

	virtual void stop();

	virtual QStringView sectionName() const {
		return m_sectionName;
	}

	virtual QStringView sectionBody() const {
		return m_sectionBody;
	}
	/*
	 anemic attempt at factory
	 the only reason I use ptr here is to show OOP
	 in our case using RAII would be preferable
	*/
	static std::unique_ptr<LAS_Section_Parser> makeSectionParser(
			QStringView sectionText);

	// pure virtual hence abstract class
	virtual void parseSection() = 0;

protected:

	QStringView m_sectionBody;
	QStringView m_sectionName;

	QMutex m_stop_signal;

	long long m_prog = 0;

	bool m_stop = false;

private:


};

#endif // SECTIONPARSER_H
