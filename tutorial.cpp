#include "tutorial.h"

#include <QCloseEvent>
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QSizePolicy>
#include <QVBoxLayout>

namespace
{
constexpr auto kSettingsKey = "tutorial/completed";
} // namespace

namespace Tutorial
{
const QVector<Step> &steps()
{
    // QT_TR_NOOP marks these literals so lupdate sees them; tr() at the
    // use site resolves them via the installed QTranslator.
    static const QVector<Step> kSteps = {
        {
            QT_TR_NOOP("Welcome"),
            QT_TR_NOOP("Welcome to QMineSweeper! Click safe cells without hitting any of the hidden mines. This short tour takes about 30 seconds."),
        },
        {
            QT_TR_NOOP("Left-click to reveal"),
            QT_TR_NOOP("Left-click a cell to reveal it. Your very first click is always safe and opens a whole area."),
        },
        {
            QT_TR_NOOP("Numbers count mines"),
            QT_TR_NOOP("A number on a cell tells you how many mines touch it. A \"3\" means three mines are hidden among the eight surrounding cells."),
        },
        {
            QT_TR_NOOP("Flag and question mark"),
            QT_TR_NOOP("Right-click a suspected mine to plant a flag. Right-click again for a \"?\" (a note to yourself), once more to clear."),
        },
        {
            QT_TR_NOOP("Chord click"),
            QT_TR_NOOP("Middle-click (or left+right together) on a satisfied number to auto-open all of its unflagged neighbours at once."),
        },
        {
            QT_TR_NOOP("Explore the menus"),
            QT_TR_NOOP("Good luck! Change difficulty under Game → Difficulty, track your times under Game → Statistics, or switch language under Settings → Language."),
        },
    };
    return kSteps;
}

bool isCompleted()
{
    QSettings settings;
    return settings.value(kSettingsKey, false).toBool();
}

void markCompleted()
{
    QSettings settings;
    settings.setValue(kSettingsKey, true);
}

void clearCompleted()
{
    QSettings settings;
    settings.remove(kSettingsKey);
}
} // namespace Tutorial

TutorialDialog::TutorialDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Tutorial"));
    setModal(true);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 20, 24, 16);
    layout->setSpacing(12);

    m_stepLabel = new QLabel(this);
    QFont stepFont = m_stepLabel->font();
    stepFont.setPointSize(stepFont.pointSize() - 1);
    m_stepLabel->setFont(stepFont);
    m_stepLabel->setStyleSheet(QStringLiteral("color: gray;"));
    layout->addWidget(m_stepLabel);

    m_titleLabel = new QLabel(this);
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(titleFont.pointSize() + 3);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setWordWrap(true);
    layout->addWidget(m_titleLabel);

    m_bodyLabel = new QLabel(this);
    m_bodyLabel->setWordWrap(true);
    m_bodyLabel->setMinimumWidth(380);
    m_bodyLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
    layout->addWidget(m_bodyLabel, /*stretch=*/1);

    auto *buttons = new QHBoxLayout;
    buttons->setSpacing(8);

    m_skipBtn = new QPushButton(tr("Skip"), this);
    m_skipBtn->setAutoDefault(false);
    connect(m_skipBtn, &QPushButton::clicked, this, &QDialog::reject);
    buttons->addWidget(m_skipBtn);

    buttons->addStretch(1);

    m_backBtn = new QPushButton(tr("Back"), this);
    m_backBtn->setAutoDefault(false);
    connect(m_backBtn, &QPushButton::clicked, this, &TutorialDialog::goBack);
    buttons->addWidget(m_backBtn);

    m_nextBtn = new QPushButton(this);
    m_nextBtn->setDefault(true);
    connect(m_nextBtn, &QPushButton::clicked, this, &TutorialDialog::goNext);
    buttons->addWidget(m_nextBtn);

    layout->addLayout(buttons);

    connect(this, &QDialog::rejected, this, &TutorialDialog::skipped);

    render();
}

int TutorialDialog::currentStep() const noexcept { return m_index; }

void TutorialDialog::render()
{
    const auto &all = Tutorial::steps();
    const int total = static_cast<int>(all.size());
    const auto &s = all[m_index];

    m_stepLabel->setText(tr("Step %1 of %2").arg(m_index + 1).arg(total));
    m_titleLabel->setText(tr(s.title));
    m_bodyLabel->setText(tr(s.body));

    m_backBtn->setEnabled(m_index > 0);
    const bool isLast = (m_index == total - 1);
    m_nextBtn->setText(isLast ? tr("Finish") : tr("Next"));

    // Feel less cramped on the first paint.
    adjustSize();
}

void TutorialDialog::goNext()
{
    const int total = static_cast<int>(Tutorial::steps().size());
    if (m_index + 1 < total)
    {
        ++m_index;
        render();
        return;
    }
    m_finished = true;
    emit completed();
    accept();
}

void TutorialDialog::goBack()
{
    if (m_index > 0)
    {
        --m_index;
        render();
    }
}

void TutorialDialog::closeEvent(QCloseEvent *e)
{
    // Closing via the window-close button counts as Skip unless we've
    // already fired completed().
    if (!m_finished)
    {
        emit skipped();
    }
    QDialog::closeEvent(e);
}
