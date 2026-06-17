#pragma once

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QString>

class DashboardPage : public QWidget
{
    Q_OBJECT

public:
    explicit DashboardPage(QWidget *parent = nullptr);
    void updateStats(int students, int rooms, int assigned, int conflicts);

private:
    QLabel *lblStudents  = nullptr;
    QLabel *lblRooms     = nullptr;
    QLabel *lblAssigned  = nullptr;
    QLabel *lblConflicts = nullptr;

    QWidget* makeStatCard(const QString &label,
                          const QString &value,
                          const QString &color);
};