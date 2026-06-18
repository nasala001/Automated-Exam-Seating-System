#include "../include/studentspage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QFrame>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QFont>

static QString tableStyle() {
    return "QTableWidget { border:1px solid #d0d9e8; border-radius:6px; font-size:12px; }"
           "QHeaderView::section { background:#003366; color:white; font-weight:bold; "
           "padding:8px 10px; border:none; border-right:1px solid #1a4a7a; }"
           "QTableWidget::item:selected { background:#dce8f7; color:#003366; }"
           "QTableWidget::item:alternate { background:#f8fafc; }";
}

StudentsPage::StudentsPage(HallModel *model, QWidget *parent)
    : QWidget(parent), m_model(model)
{
    buildUi();
}

void StudentsPage::buildUi() {
    auto *vl = new QVBoxLayout(this);
    vl->setContentsMargins(28,24,28,24);
    vl->setSpacing(16);

    // Header
    auto *titleLbl = new QLabel("Student Registry");
    titleLbl->setStyleSheet("font-size:22px; font-weight:bold; color:#003366;");
    auto *subLbl   = new QLabel("Manage enrolled students and their examination details");
    subLbl->setStyleSheet("font-size:12px; color:#6b7a8d;");
    vl->addWidget(titleLbl);
    vl->addWidget(subLbl);

    auto *sep = new QFrame; sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("background:#d0d9e8;"); sep->setFixedHeight(1);
    vl->addWidget(sep);

    // Toolbar
    auto *toolbar = new QFrame;
    toolbar->setStyleSheet("QFrame { background:#f0f4fa; border:1px solid #e0e7ef; border-radius:6px; }");
    auto *tbl = new QHBoxLayout(toolbar);
    tbl->setContentsMargins(12,10,12,10);
    tbl->setSpacing(10);

    m_searchBox = new QLineEdit;
    m_searchBox->setPlaceholderText("🔍  Search by name or roll number...");
    m_searchBox->setMinimumWidth(240);
    m_searchBox->setStyleSheet(
        "QLineEdit { border:1.5px solid #d0d9e8; border-radius:6px; padding:7px 10px; font-size:12px; }"
        "QLineEdit:focus { border-color:#003366; background:#f8faff; }");
    connect(m_searchBox, &QLineEdit::textChanged, this, &StudentsPage::onSearch);

    m_deptFilter = new QComboBox;
    m_deptFilter->addItem("All Departments");
    m_deptFilter->setMinimumWidth(180);
    m_deptFilter->setStyleSheet(
        "QComboBox { border:1.5px solid #d0d9e8; border-radius:6px; padding:6px 10px; font-size:12px; }"
        "QComboBox:focus { border-color:#003366; }");

    m_semFilter = new QComboBox;
    m_semFilter->addItem("All Semesters");
    m_semFilter->setStyleSheet(
        "QComboBox { border:1.5px solid #d0d9e8; border-radius:6px; padding:6px 10px; font-size:12px; }"
        "QComboBox:focus { border-color:#003366; }");

    connect(m_deptFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &StudentsPage::onFilterChanged);
    connect(m_semFilter,  QOverload<int>::of(&QComboBox::currentIndexChanged), this, &StudentsPage::onFilterChanged);

    tbl->addWidget(m_searchBox);
    tbl->addWidget(new QLabel("Dept:"));
    tbl->addWidget(m_deptFilter);
    tbl->addWidget(new QLabel("Sem:"));
    tbl->addWidget(m_semFilter);
    tbl->addStretch();

    auto mkBtn = [&](const QString &label, const QString &style) -> QPushButton* {
        auto *b = new QPushButton(label);
        b->setStyleSheet(style);
        return b;
    };
    auto *addBtn = mkBtn("+ Add Student",
        "QPushButton { background:#003366; color:white; border:none; border-radius:6px; "
        "padding:8px 14px; font-weight:bold; font-size:12px; }"
        "QPushButton:hover { background:#1a4a7a; }");
    auto *editBtn = mkBtn("Edit",
        "QPushButton { background:white; color:#003366; border:1.5px solid #003366; "
        "border-radius:6px; padding:7px 14px; font-size:12px; }"
        "QPushButton:hover { background:#e8f0fb; }");
    auto *removeBtn = mkBtn("Remove",
        "QPushButton { background:white; color:#8b0000; border:1.5px solid #8b0000; "
        "border-radius:6px; padding:7px 14px; font-size:12px; }"
        "QPushButton:hover { background:#fff0f0; }");
    auto *importBtn = mkBtn("Import CSV",
        "QPushButton { background:#c9a84c; color:#1a2332; border:none; border-radius:6px; "
        "padding:8px 14px; font-weight:bold; font-size:12px; }"
        "QPushButton:hover { background:#b8983e; }");

    connect(addBtn,    &QPushButton::clicked, this, &StudentsPage::onAddStudent);
    connect(editBtn,   &QPushButton::clicked, this, &StudentsPage::onEditStudent);
    connect(removeBtn, &QPushButton::clicked, this, &StudentsPage::onRemoveStudent);
    connect(importBtn, &QPushButton::clicked, this, &StudentsPage::onImportCSV);

    tbl->addWidget(addBtn);
    tbl->addWidget(editBtn);
    tbl->addWidget(removeBtn);
    tbl->addWidget(importBtn);
    vl->addWidget(toolbar);

    // Table
    m_table = new QTableWidget;
    m_table->setColumnCount(8);
    m_table->setHorizontalHeaderLabels({"ID","Roll No.","Name","Department",
                                        "Semester","Program","Subject","Seat"});
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(true);
    m_table->verticalHeader()->hide();
    m_table->setStyleSheet(tableStyle());
    vl->addWidget(m_table);

    // Footer count
    m_countLbl = new QLabel("0 students");
    m_countLbl->setStyleSheet("font-size:11px; color:#6b7a8d;");
    vl->addWidget(m_countLbl);
    setLayout(vl);
}

void StudentsPage::refresh() {
    updateFilters();
    onFilterChanged();
}

void StudentsPage::updateFilters() {
    QString curDept = m_deptFilter->currentText();
    QString curSem  = m_semFilter->currentText();

    m_deptFilter->blockSignals(true);
    m_semFilter->blockSignals(true);
    m_deptFilter->clear();
    m_deptFilter->addItem("All Departments");
    for (auto &d : m_model->departments()) m_deptFilter->addItem(d);
    m_semFilter->clear();
    m_semFilter->addItem("All Semesters");
    for (int s : m_model->semesters()) m_semFilter->addItem(QString("Semester %1").arg(s));

    int di = m_deptFilter->findText(curDept);
    if (di>=0) m_deptFilter->setCurrentIndex(di);
    int si = m_semFilter->findText(curSem);
    if (si>=0) m_semFilter->setCurrentIndex(si);

    m_deptFilter->blockSignals(false);
    m_semFilter->blockSignals(false);
}

void StudentsPage::onSearch() { onFilterChanged(); }

void StudentsPage::onFilterChanged() {
    QString q    = m_searchBox->text().trimmed();
    QString dept = m_deptFilter->currentIndex()>0 ? m_deptFilter->currentText() : "";
    int     sem  = -1;
    if (m_semFilter->currentIndex()>0) {
        QString t = m_semFilter->currentText();
        sem = t.remove("Semester ").toInt();
    }

    QList<Student*> all = q.isEmpty() ? m_model->allStudents()
                                       : m_model->searchStudents(q);
    QList<Student*> filtered;
    for (Student *s : all) {
        if (!dept.isEmpty() && s->department != dept) continue;
        if (sem > 0 && s->semester != sem) continue;
        filtered.append(s);
    }
    populateTable(filtered);
}

void StudentsPage::populateTable(QList<Student*> students) {
    m_table->setRowCount(students.size());
    for (int i=0; i<students.size(); i++) {
        Student *s = students[i];
        auto setC = [&](int col, const QString &text, Qt::Alignment al=Qt::AlignVCenter|Qt::AlignLeft) {
            auto *item = new QTableWidgetItem(text);
            item->setTextAlignment(al);
            m_table->setItem(i, col, item);
        };
        setC(0, QString::number(s->id), Qt::AlignCenter);
        setC(1, s->rollNumber, Qt::AlignCenter);
        setC(2, s->name);
        setC(3, s->department);
        setC(4, QString("Sem %1").arg(s->semester), Qt::AlignCenter);
        setC(5, s->program, Qt::AlignCenter);
        setC(6, s->subject);

        auto *seatItem = new QTableWidgetItem(s->isAssigned() ? s->seatCode() : "Unassigned");
        seatItem->setTextAlignment(Qt::AlignCenter);
        if (s->isAssigned())
            seatItem->setForeground(QColor("#155724"));
        else
            seatItem->setForeground(QColor("#856404"));
        m_table->setItem(i, 7, seatItem);
    }
    m_countLbl->setText(QString("%1 student(s)").arg(students.size()));
}

int StudentsPage::selectedStudentId() const {
    int row = m_table->currentRow();
    if (row < 0) return -1;
    auto *item = m_table->item(row, 0);
    return item ? item->text().toInt() : -1;
}

void StudentsPage::onAddStudent() {
    QDialog dlg(this);
    dlg.setWindowTitle("Add Student");
    dlg.setMinimumWidth(420);
    dlg.setStyleSheet("QDialog { background:#f5f7fa; } QLabel { color:#1a2332; font-size:12px; }");

    auto *fl = new QFormLayout(&dlg);
    fl->setContentsMargins(20,20,20,16);
    fl->setSpacing(12);

    auto mkEdit = [&](const QString &ph) -> QLineEdit* {
        auto *e = new QLineEdit;
        e->setPlaceholderText(ph);
        e->setStyleSheet("QLineEdit { border:1.5px solid #d0d9e8; border-radius:5px; "
                         "padding:7px; font-size:12px; }"
                         "QLineEdit:focus { border-color:#003366; }");
        return e;
    };

    auto *nameEdit    = mkEdit("Full name");
    auto *rollEdit    = mkEdit("e.g. KU20001");
    auto *deptEdit    = mkEdit("e.g. Computer Science");
    auto *semSpin     = new QSpinBox; semSpin->setRange(1,8); semSpin->setValue(1);
    semSpin->setStyleSheet("QSpinBox { border:1.5px solid #d0d9e8; border-radius:5px; padding:6px; font-size:12px; }");
    auto *progEdit    = mkEdit("e.g. BE");
    auto *subjectEdit = mkEdit("e.g. Data Structures");

    fl->addRow("Name *",       nameEdit);
    fl->addRow("Roll Number *",rollEdit);
    fl->addRow("Department *", deptEdit);
    fl->addRow("Semester",     semSpin);
    fl->addRow("Program",      progEdit);
    fl->addRow("Subject",      subjectEdit);

    auto *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    bb->button(QDialogButtonBox::Ok)->setStyleSheet(
        "QPushButton { background:#003366; color:white; border:none; border-radius:5px; padding:7px 16px; font-weight:bold; }"
        "QPushButton:hover { background:#1a4a7a; }");
    bb->button(QDialogButtonBox::Cancel)->setStyleSheet(
        "QPushButton { background:white; color:#6b7a8d; border:1px solid #d0d9e8; border-radius:5px; padding:7px 14px; }");
    connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    fl->addRow(bb);

    if (dlg.exec() == QDialog::Accepted) {
        if (nameEdit->text().trimmed().isEmpty() || rollEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(this,"Required","Name and Roll Number are required.");
            return;
        }
        Student s;
        s.name       = nameEdit->text().trimmed();
        s.rollNumber = rollEdit->text().trimmed();
        s.department = deptEdit->text().trimmed().isEmpty() ? "General" : deptEdit->text().trimmed();
        s.semester   = semSpin->value();
        s.program    = progEdit->text().trimmed().isEmpty() ? "BE" : progEdit->text().trimmed();
        s.subject    = subjectEdit->text().trimmed();
        m_model->addStudent(s);
        refresh();
    }
}

void StudentsPage::onEditStudent() {
    int sid = selectedStudentId();
    if (sid < 0) { QMessageBox::information(this,"Select","Please select a student to edit."); return; }
    Student *s = m_model->findById(sid);
    if (!s) return;

    QDialog dlg(this);
    dlg.setWindowTitle("Edit Student");
    dlg.setMinimumWidth(420);
    dlg.setStyleSheet("QDialog { background:#f5f7fa; }");

    auto *fl = new QFormLayout(&dlg);
    fl->setContentsMargins(20,20,20,16); fl->setSpacing(12);

    auto mkEdit = [&](const QString &val) -> QLineEdit* {
        auto *e = new QLineEdit(val);
        e->setStyleSheet("QLineEdit { border:1.5px solid #d0d9e8; border-radius:5px; "
                         "padding:7px; font-size:12px; }"
                         "QLineEdit:focus { border-color:#003366; }");
        return e;
    };
    auto *nameEdit    = mkEdit(s->name);
    auto *rollEdit    = mkEdit(s->rollNumber);
    auto *deptEdit    = mkEdit(s->department);
    auto *semSpin     = new QSpinBox; semSpin->setRange(1,8); semSpin->setValue(s->semester);
    semSpin->setStyleSheet("QSpinBox { border:1.5px solid #d0d9e8; border-radius:5px; padding:6px; font-size:12px; }");
    auto *progEdit    = mkEdit(s->program);
    auto *subjectEdit = mkEdit(s->subject);

    fl->addRow("Name *",       nameEdit);
    fl->addRow("Roll Number *",rollEdit);
    fl->addRow("Department *", deptEdit);
    fl->addRow("Semester",     semSpin);
    fl->addRow("Program",      progEdit);
    fl->addRow("Subject",      subjectEdit);

    auto *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    bb->button(QDialogButtonBox::Ok)->setStyleSheet(
        "QPushButton { background:#003366; color:white; border:none; border-radius:5px; padding:7px 16px; font-weight:bold; }"
        "QPushButton:hover { background:#1a4a7a; }");
    bb->button(QDialogButtonBox::Cancel)->setStyleSheet(
        "QPushButton { background:white; color:#6b7a8d; border:1px solid #d0d9e8; border-radius:5px; padding:7px 14px; }");
    connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    fl->addRow(bb);

    if (dlg.exec() == QDialog::Accepted) {
        s->name       = nameEdit->text().trimmed();
        s->rollNumber = rollEdit->text().trimmed();
        s->department = deptEdit->text().trimmed();
        s->semester   = semSpin->value();
        s->program    = progEdit->text().trimmed();
        s->subject    = subjectEdit->text().trimmed();
        m_model->updateStudent(*s);
        refresh();
    }
}

void StudentsPage::onRemoveStudent() {
    int sid = selectedStudentId();
    if (sid < 0) { QMessageBox::information(this,"Select","Please select a student to remove."); return; }
    Student *s = m_model->findById(sid);
    if (!s) return;
    if (QMessageBox::question(this,"Confirm",
        QString("Remove student %1 (%2)?").arg(s->name, s->rollNumber),
        QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes) {
        m_model->removeStudent(sid);
        refresh();
    }
}

void StudentsPage::onImportCSV() {
    QString path = QFileDialog::getOpenFileName(this, "Import Students CSV", "", "CSV Files (*.csv)");
    if (path.isEmpty()) return;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return;
    QTextStream ts(&f);
    ts.readLine(); // skip header
    int count = 0;
    while (!ts.atEnd()) {
        QString line = ts.readLine();
        QStringList cols = line.split(',');
        if (cols.size() < 4) continue;
        Student s;
        s.name       = cols[0].trimmed();
        s.rollNumber = cols[1].trimmed();
        s.department = cols[2].trimmed();
        s.semester   = cols.size()>3 ? cols[3].trimmed().toInt() : 1;
        s.program    = cols.size()>4 ? cols[4].trimmed() : "BE";
        s.subject    = cols.size()>5 ? cols[5].trimmed() : "";
        if (!s.name.isEmpty() && !s.rollNumber.isEmpty()) {
            m_model->addStudent(s);
            count++;
        }
    }
    QMessageBox::information(this,"Import Complete",
                             QString("Imported %1 students.").arg(count));
    refresh();
}
