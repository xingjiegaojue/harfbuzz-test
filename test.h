#ifndef TEST_H
#define TEST_H

#include <QtTest/QtTest>
#include <QObject>

class tst_QScriptEngine : public QObject
{
	Q_OBJECT

public:
	tst_QScriptEngine();
	virtual ~tst_QScriptEngine();

public slots:
	void initTestCase();
	void cleanupTestCase();

private slots:
	void arabic();
};

#endif // TEST_H