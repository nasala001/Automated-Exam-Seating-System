#pragma once

#include <QWidget>
#include <QLabel>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QMessageBox>
#include <QString>

class RoomPage : public QWidget
{
    Q_OBJECT

public:
    explicit RoomPage(QWidget *parent = nullptr);

private slots:
    void addRoom();
    void deleteRoom();

private:
    QTableWidget *roomTable  = nullptr;
    QPushButton  *btnAdd     = nullptr;
    QPushButton  *btnDelete  = nullptr;
};