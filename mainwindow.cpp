#include "mainwindow.h"

#include "./ui_mainwindow.h"

#include <QTimer>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(std::make_unique<Ui::MainWindow>())
{
    ui->setupUi(this);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_timer.start();

    auto *displayTimer = new QTimer(this);
    connect(displayTimer, &QTimer::timeout, this, [this] { ui->Time->setText(QString::asprintf("%05.1f", static_cast<float>(m_timer.elapsed()) / 1000.0f)); });
    displayTimer->start(50);

    ui->mineFieldWidget->getMineCountLabel(ui->mineCount);

    centralWidget()->layout()->setContentsMargins(0, 0, 0, 0);
    setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
}

MainWindow::~MainWindow() = default;
