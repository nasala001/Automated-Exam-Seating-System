#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>

class MainWindow;

class StudentPage : public QWidget
{
    Q_OBJECT
public:
    explicit StudentPage(MainWindow *mainWin, QWidget *parent = nullptr);

signals:
    void studentsLoaded(int count);

private slots:
    void browseFile();
    void filterTable(const QString &text);
    void deleteSelected();

private:
    MainWindow   *mainWindow;
    QTableWidget *studentTable;
    QPushButton  *btnBrowse;
    QPushButton  *btnDelete;
    QLabel       *filePathLabel;
    QLineEdit    *searchBar;

    void loadFile(const QString &filePath);
};