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
#include "RoomPage.h"

RoomPage::RoomPage(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(30, 30, 30, 30);
    layout->setSpacing(14);

    QLabel *title = new QLabel("Rooms", this);
    title->setStyleSheet("font-size: 26px; font-weight: bold; color: #cdd6f4;");
    layout->addWidget(title);

    QString btnStyle =
        "QPushButton { background: #313244; color: #cdd6f4;"
        "border-radius: 6px; padding: 8px 16px; font-size: 13px; }"
        "QPushButton:hover { background: #45475a; }";

    QHBoxLayout *toolbar = new QHBoxLayout();
    btnAdd    = new QPushButton("+ Add Room",    this);
    btnDelete = new QPushButton("Delete Room",   this);
    btnAdd->setStyleSheet(btnStyle);
    btnDelete->setStyleSheet(btnStyle);
    toolbar->addWidget(btnAdd);
    toolbar->addWidget(btnDelete);
    toolbar->addStretch();
    layout->addLayout(toolbar);

    roomTable = new QTableWidget(0, 3, this);
    roomTable->setHorizontalHeaderLabels({"Room ID", "Capacity", "Status"});
    roomTable->horizontalHeader()->setStretchLastSection(true);
    roomTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    roomTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    roomTable->setStyleSheet(
        "QTableWidget { background: #1e1e2e; color: #cdd6f4;"
        "  gridline-color: #313244; border: none; font-size: 13px; }"
        "QHeaderView::section { background: #313244; color: #89b4fa;"
        "  padding: 8px; border: none; font-weight: bold; }"
        "QTableWidget::item:selected { background: #45475a; }"
        );

    layout->addWidget(roomTable);
    setLayout(layout);

    connect(btnAdd,    &QPushButton::clicked, this, &RoomPage::addRoom);
    connect(btnDelete, &QPushButton::clicked, this, &RoomPage::deleteRoom);
}

void RoomPage::addRoom()
{
    bool ok;
    QString roomId = QInputDialog::getText(
        this, "Add Room", "Enter Room ID:", QLineEdit::Normal, "", &ok
        );
    if (!ok || roomId.isEmpty()) return;

    QString cap = QInputDialog::getText(
        this, "Add Room", "Enter Capacity:", QLineEdit::Normal, "30", &ok
        );
    if (!ok) return;

    int row = roomTable->rowCount();
    roomTable->insertRow(row);
    roomTable->setItem(row, 0, new QTableWidgetItem(roomId));
    roomTable->setItem(row, 1, new QTableWidgetItem(cap));
    roomTable->setItem(row, 2, new QTableWidgetItem("Available"));
}

void RoomPage::deleteRoom()
{
    int row = roomTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "No Selection", "Please select a room to delete.");
        return;
    }
    roomTable->removeRow(row);
}