#include "ResultPage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>

ResultPage::ResultPage(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(30, 30, 30, 30);
    layout->setSpacing(14);

    QLabel *title = new QLabel("Seating Results");
    title->setStyleSheet("font-size: 26px; font-weight: bold; color: #cdd6f4;");
    layout->addWidget(title);

    QString btnStyle =
        "QPushButton { background: #313244; color: #cdd6f4;"
        "border-radius: 6px; padding: 8px 16px; font-size: 13px; }"
        "QPushButton:hover { background: #45475a; }";

    QHBoxLayout *controls = new QHBoxLayout();
    QLabel *roomLabel = new QLabel("Room:");
    roomLabel->setStyleSheet("color: #cdd6f4;");
    roomSelector = new QComboBox(this);
    roomSelector->addItems({"Room A", "Room B", "Room C"});
    roomSelector->setStyleSheet(
        "QComboBox { background: #313244; color: #cdd6f4;"
        "border-radius: 6px; padding: 6px 10px; }"
        );
    btnGenerate = new QPushButton("⚡ Generate Seating", this);
    btnExport   = new QPushButton("📤 Export",           this);
    btnGenerate->setStyleSheet(
        "QPushButton { background: #89b4fa; color: #1e1e2e;"
        "border-radius: 6px; padding: 8px 18px; font-weight: bold; }"
        "QPushButton:hover { background: #b4d0f7; }"
        );
    btnExport->setStyleSheet(btnStyle);

    controls->addWidget(roomLabel);
    controls->addWidget(roomSelector);
    controls->addStretch();
    controls->addWidget(btnGenerate);
    controls->addWidget(btnExport);
    layout->addLayout(controls);

    statusLabel = new QLabel("Ready to generate seating...");
    statusLabel->setStyleSheet("color: #6c7086; font-size: 12px;");
    layout->addWidget(statusLabel);

    seatGrid = new QTableWidget(5, 6, this);
    seatGrid->setEditTriggers(QAbstractItemView::NoEditTriggers);
    seatGrid->horizontalHeader()->setStretchLastSection(true);
    seatGrid->setStyleSheet(
        "QTableWidget { background: #1e1e2e; color: #cdd6f4;"
        "  gridline-color: #313244; border: none; font-size: 13px; }"
        "QHeaderView::section { background: #313244; color: #89b4fa;"
        "  padding: 8px; border: none; font-weight: bold; }"
        "QTableWidget::item:selected { background: #45475a; }"
        );

    layout->addWidget(seatGrid);
    setLayout(layout);

    connect(btnGenerate, &QPushButton::clicked, this, &ResultPage::generateSeating);
    connect(btnExport,   &QPushButton::clicked, this, &ResultPage::exportResults);
}

void ResultPage::generateSeating()
{
    statusLabel->setText("⚡ Generating seating plan...");
    // TODO: connect to teammate's seat engine
    // auto result = seatEngine->assignSeats(students, room);
    statusLabel->setText("✅ Seating generated for " +
                         roomSelector->currentText());
    QMessageBox::information(this, "Done",
                             "Seating engine will be connected here.");
}

void ResultPage::exportResults()
{
    // TODO: connect to teammate's export module
    QMessageBox::information(this, "Export",
                             "Export feature will be connected here.");
}