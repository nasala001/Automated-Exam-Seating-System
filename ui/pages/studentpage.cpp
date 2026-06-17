#include "StudentPage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QFileDialog>
#include <QDir>
#include <QMessageBox>

StudentPage::StudentPage(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(30, 30, 30, 30);
    layout->setSpacing(14);

    // Title
    QLabel *title = new QLabel("Students");
    title->setStyleSheet("font-size: 26px; font-weight: bold; color: #cdd6f4;");
    layout->addWidget(title);

    // Toolbar
    QHBoxLayout *toolbar = new QHBoxLayout();
    btnBrowse  = new QPushButton("📂 Import CSV", this);
    btnDelete  = new QPushButton("🗑 Delete",     this);
    searchBar  = new QLineEdit(this);
    searchBar->setPlaceholderText("🔍  Search student...");
    searchBar->setFixedWidth(220);

    QString btnStyle =
        "QPushButton {"
        "  background: #313244; color: #cdd6f4;"
        "  border-radius: 6px; padding: 8px 16px;"
        "  font-size: 13px;"
        "}"
        "QPushButton:hover { background: #45475a; }";

    btnBrowse->setStyleSheet(btnStyle);
    btnDelete->setStyleSheet(btnStyle);
    searchBar->setStyleSheet(
        "QLineEdit { background: #313244; color: #cdd6f4;"
        "border: 1px solid #45475a; border-radius: 6px; padding: 6px 10px; }"
        );

    toolbar->addWidget(btnBrowse);
    toolbar->addWidget(btnDelete);
    toolbar->addStretch();
    toolbar->addWidget(searchBar);
    layout->addLayout(toolbar);

    // File path label
    filePathLabel = new QLabel("No file selected...", this);
    filePathLabel->setStyleSheet("color: #6c7086; font-size: 12px;");
    layout->addWidget(filePathLabel);

    // Table
    studentTable = new QTableWidget(0, 4, this);
    studentTable->setHorizontalHeaderLabels({"ID", "Name", "Faculty", "Exam"});
    studentTable->horizontalHeader()->setStretchLastSection(true);
    studentTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    studentTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    studentTable->setAlternatingRowColors(true);
    studentTable->setStyleSheet(
        "QTableWidget { background: #1e1e2e; color: #cdd6f4;"
        "  gridline-color: #313244; border: none; font-size: 13px; }"
        "QHeaderView::section { background: #313244; color: #89b4fa;"
        "  padding: 8px; border: none; font-weight: bold; }"
        "QTableWidget::item:selected { background: #45475a; }"
        "QTableWidget::item:alternate { background: #181825; }"
        );

    layout->addWidget(studentTable);
    setLayout(layout);

    connect(btnBrowse, &QPushButton::clicked, this, &StudentPage::browseFile);
    connect(btnDelete, &QPushButton::clicked, this, &StudentPage::deleteSelected);
    connect(searchBar, &QLineEdit::textChanged, this, &StudentPage::filterTable);
}

void StudentPage::browseFile()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "Select CSV File",
        QDir::homePath(),
        "CSV Files (*.csv);;All Files (*)"
        );
    if (filePath.isEmpty()) return;
    loadFile(filePath);
}

void StudentPage::loadFile(const QString &filePath)
{
    filePathLabel->setText("📄 " + filePath);
    // TODO: connect to teammate's CSV reader
    // QList<Student> students = csvReader->importCSV(filePath);
    QMessageBox::information(this, "File Loaded",
                             "File selected:\n" + filePath +
                                 "\n\nCSV reader will be connected here.");
}

void StudentPage::filterTable(const QString &text)
{
    for (int i = 0; i < studentTable->rowCount(); i++) {
        bool match = false;
        for (int j = 0; j < studentTable->columnCount(); j++) {
            QTableWidgetItem *item = studentTable->item(i, j);
            if (item && item->text().contains(text, Qt::CaseInsensitive)) {
                match = true;
                break;
            }
        }
        studentTable->setRowHidden(i, !match);
    }
}

void StudentPage::deleteSelected()
{
    int row = studentTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "No Selection", "Please select a row to delete.");
        return;
    }
    studentTable->removeRow(row);
}