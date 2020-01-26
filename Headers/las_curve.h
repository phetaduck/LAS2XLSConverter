#ifndef LAS_CURVE_H
#define LAS_CURVE_H

#include <QString>
#include <vector>

class LAS_Curve
{
public:
	LAS_Curve() = default;
	LAS_Curve(const LAS_Curve& src) = default;
	LAS_Curve(LAS_Curve&& src) = default;
	LAS_Curve& operator=(const LAS_Curve& src) = default;
	LAS_Curve& operator=(LAS_Curve&& src) = default;
	virtual ~LAS_Curve();

	LAS_Curve(QString name, std::vector<QString> values) :
		m_name(std::move(name)), m_values(std::move(values))
	{}

	const QString& name() const {
		return m_name;
	}
	const auto& values() const {
		return m_values;
	}
private:
	QString m_name;
	std::vector<QString> m_values;
};

#endif // LAS_CURVE_H
