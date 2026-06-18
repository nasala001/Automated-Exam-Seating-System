#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include "studentmodel.h"

class StudentsPage : public QWidget {
    Q_OBJECT
public:
    explicit StudentsPage(HallModel *model, QWidget *parent=nullptr);
    void refresh();

signals:
    void navigateToSeat(int studentId);

private slots:
    void onSearch();
    void onAddStudent();
    void onEditStudent();
    void onRemoveStudent();
    void onImportCSV();
    void onFilterChanged();

private:
    HallModel     *m_model;
    QTableWidget  *m_table;
    QLineEdit     *m_searchBox;
    QComboBox     *m_deptFilter;
    QComboBox     *m_semFilter;
    QLabel        *m_countLbl;

    void buildUi();
    void populateTable(QList<Student*> students);
    void updateFilters();
    int  selectedStudentId() const;
};
