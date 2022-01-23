#ifndef MINEFIELD_H
#define MINEFIELD_H

#include <QGridLayout>
#include <QWidget>

class MineField : public QWidget
{
    Q_OBJECT
public:
    explicit MineField(QWidget *parent = nullptr);

signals:


private:
    QGridLayout grid{this};
};

#endif // MINEFIELD_H
