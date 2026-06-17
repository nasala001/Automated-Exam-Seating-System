#include "studentpage.h"
#include "mainwindow.h"
#include "Student.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QFileDialog>
#include <QDir>
#include <QMessageBox>

StudentPage::StudentPage(MainWindow *mainWin, QWidget *parent)
    : QWidget(parent), mainWindow(mainWin)
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

    // Table — 6 columns matching Student struct
    studentTable = new QTableWidget(0, 6, this);
    studentTable->setHorizontalHeaderLabels(
        {"Name", "Exam Roll No", "Reg No", "Program", "Semester", "Subject"});
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

    // Use the backend CSV parser
    std::vector<Student> parsed = parseStudentsFromCSV(filePath.toStdString());

    if (parsed.empty()) {
        QMessageBox::warning(this, "Import Failed",
                             "No students found in the file.\n"
                             "Expected CSV format:\n"
                             "Name,ExamRollNo,RegNo,Program,Semester,Subject");
        return;
    }

    // Store in MainWindow's shared data
    mainWindow->setStudents(parsed);

    // Populate the table
    studentTable->setRowCount(0);
    for (const auto& s : parsed) {
        int row = studentTable->rowCount();
        studentTable->insertRow(row);
        studentTable->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(s.name)));
        studentTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(s.examRollNo)));
        studentTable->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(s.registrationNo)));
        studentTable->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(s.program)));
        studentTable->setItem(row, 4, new QTableWidgetItem(QString::number(s.semester)));
        studentTable->setItem(row, 5, new QTableWidgetItem(QString::fromStdString(s.subject)));
    }

    QMessageBox::information(this, "Import Successful",
                             QString("Loaded %1 students from CSV.").arg(parsed.size()));

    emit studentsLoaded(static_cast<int>(parsed.size()));
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