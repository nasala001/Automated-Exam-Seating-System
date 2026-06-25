#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>

class ResultPage : public QWidget
{
    Q_OBJECT
public:
    explicit ResultPage(QWidget *parent = nullptr);

private slots:
    void generateSeating();
    void exportResults();

private:
    QTableWidget *seatGrid;
    QComboBox    *roomSelector;
    QPushButton  *btnGenerate;
    QPushButton  *btnExport;
    QLabel       *statusLabel;
};