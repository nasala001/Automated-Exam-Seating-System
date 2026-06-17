#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>

class StudentPage : public QWidget
{
    Q_OBJECT
public:
    explicit StudentPage(QWidget *parent = nullptr);

private slots:
    void browseFile();
    void filterTable(const QString &text);
    void deleteSelected();

private:
    QTableWidget *studentTable;
    QPushButton  *btnBrowse;
    QPushButton  *btnDelete;
    QLabel       *filePathLabel;
    QLineEdit    *searchBar;

    void loadFile(const QString &filePath);
};