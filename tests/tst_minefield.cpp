#include "../minefield.h"

#include <QLabel>
#include <QtTest>

class TestMineField : public QObject
{
    Q_OBJECT

  private slots:
    void testConstruction();
    void testGetMineCountLabelInitialValue();
    void testIncrementFlagCount();
};

void TestMineField::testConstruction()
{
    // MineField should construct without crashing
    MineField field;
    QVERIFY(field.isWidgetType());
}

void TestMineField::testGetMineCountLabelInitialValue()
{
    MineField field;
    QLabel label;
    field.getMineCountLabel(&label);
    // Initial mine count label should equal total mines (20) minus flags placed (0)
    QCOMPARE(label.text(), QString("20"));
}

void TestMineField::testIncrementFlagCount()
{
    MineField field;
    QLabel label;
    field.getMineCountLabel(&label);
    QCOMPARE(label.text(), QString("20"));

    field.incrementflagCount();
    QCOMPARE(label.text(), QString("19"));

    field.incrementflagCount();
    QCOMPARE(label.text(), QString("18"));
}

QTEST_MAIN(TestMineField)
#include "tst_minefield.moc"
