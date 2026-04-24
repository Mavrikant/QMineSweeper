#include "../tutorial.h"

#include <QCoreApplication>
#include <QPushButton>
#include <QSettings>
#include <QSignalSpy>
#include <QtTest>

class TestTutorial : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();
    void cleanup();

    void testStepListIsNonEmptyAndOrdered();
    void testEveryStepHasTitleAndBody();

    void testIsCompletedDefaultsFalse();
    void testMarkCompletedPersists();
    void testClearCompletedWipesFlag();

    void testDialogStartsAtStepZero();
    void testNextAdvancesThroughSteps();
    void testBackDisabledAtStart();
    void testBackGoesBackOneStep();
    void testFinishOnLastStepEmitsCompleted();
    void testSkipButtonEmitsSkipped();
};

void TestTutorial::initTestCase()
{
    QCoreApplication::setOrganizationName("Mavrikant");
    QCoreApplication::setApplicationName("QMineSweeperTestTutorial");
    QSettings::setDefaultFormat(QSettings::IniFormat);
}

void TestTutorial::cleanup()
{
    QSettings s;
    s.clear();
}

void TestTutorial::testStepListIsNonEmptyAndOrdered()
{
    const auto &steps = Tutorial::steps();
    QVERIFY(!steps.isEmpty());
    QVERIFY(steps.size() >= 3); // the walkthrough earns its UI only past a couple of steps
}

void TestTutorial::testEveryStepHasTitleAndBody()
{
    for (const auto &s : Tutorial::steps())
    {
        QVERIFY2(s.title != nullptr && *s.title != '\0', "step title must be non-empty");
        QVERIFY2(s.body != nullptr && *s.body != '\0', "step body must be non-empty");
    }
}

void TestTutorial::testIsCompletedDefaultsFalse() { QVERIFY(!Tutorial::isCompleted()); }

void TestTutorial::testMarkCompletedPersists()
{
    Tutorial::markCompleted();
    QVERIFY(Tutorial::isCompleted());
}

void TestTutorial::testClearCompletedWipesFlag()
{
    Tutorial::markCompleted();
    QVERIFY(Tutorial::isCompleted());
    Tutorial::clearCompleted();
    QVERIFY(!Tutorial::isCompleted());
}

static QPushButton *findButton(QWidget *root, const QString &text)
{
    for (auto *b : root->findChildren<QPushButton *>())
    {
        if (b->text() == text)
        {
            return b;
        }
    }
    return nullptr;
}

void TestTutorial::testDialogStartsAtStepZero()
{
    TutorialDialog dlg;
    QCOMPARE(dlg.currentStep(), 0);
}

void TestTutorial::testNextAdvancesThroughSteps()
{
    TutorialDialog dlg;
    QPushButton *next = findButton(&dlg, QObject::tr("Next"));
    QVERIFY(next);
    next->click();
    QCOMPARE(dlg.currentStep(), 1);
    next->click();
    QCOMPARE(dlg.currentStep(), 2);
}

void TestTutorial::testBackDisabledAtStart()
{
    TutorialDialog dlg;
    QPushButton *back = findButton(&dlg, QObject::tr("Back"));
    QVERIFY(back);
    QVERIFY(!back->isEnabled());
}

void TestTutorial::testBackGoesBackOneStep()
{
    TutorialDialog dlg;
    QPushButton *next = findButton(&dlg, QObject::tr("Next"));
    QVERIFY(next);
    next->click();
    next->click();
    QCOMPARE(dlg.currentStep(), 2);
    QPushButton *back = findButton(&dlg, QObject::tr("Back"));
    QVERIFY(back);
    QVERIFY(back->isEnabled());
    back->click();
    QCOMPARE(dlg.currentStep(), 1);
}

void TestTutorial::testFinishOnLastStepEmitsCompleted()
{
    TutorialDialog dlg;
    QSignalSpy completedSpy(&dlg, &TutorialDialog::completed);
    QSignalSpy skippedSpy(&dlg, &TutorialDialog::skipped);

    const int total = static_cast<int>(Tutorial::steps().size());
    for (int i = 0; i < total - 1; ++i)
    {
        QPushButton *next = findButton(&dlg, QObject::tr("Next"));
        QVERIFY(next);
        next->click();
    }
    QPushButton *finish = findButton(&dlg, QObject::tr("Finish"));
    QVERIFY(finish);
    finish->click();

    QCOMPARE(completedSpy.count(), 1);
    QCOMPARE(skippedSpy.count(), 0);
}

void TestTutorial::testSkipButtonEmitsSkipped()
{
    TutorialDialog dlg;
    QSignalSpy completedSpy(&dlg, &TutorialDialog::completed);
    QSignalSpy skippedSpy(&dlg, &TutorialDialog::skipped);

    QPushButton *skip = findButton(&dlg, QObject::tr("Skip"));
    QVERIFY(skip);
    skip->click();

    QCOMPARE(completedSpy.count(), 0);
    QCOMPARE(skippedSpy.count(), 1);
}

QTEST_MAIN(TestTutorial)
#include "tst_tutorial.moc"
