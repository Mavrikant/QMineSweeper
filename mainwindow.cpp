#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QTimer>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    timer.start();
    QTimer *timer2 = new QTimer(this);
    connect(timer2, &QTimer::timeout, this, [&] { ui->Time->setText(QString::asprintf("%05.1f", (float)timer.elapsed() / 1000.0)); });
    timer2->start(50);

    ui->mineFieldWidget->getMineCountLabel(ui->mineCount);

    centralWidget()->layout()->setContentsMargins(0, 0, 0, 0);
    setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
}

MainWindow::~MainWindow() { delete ui; }
