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
    void updateStats(int students, int venues, int totalSeats, int conflicts);

private:
    QLabel *lblStudents  = nullptr;
    QLabel *lblVenues    = nullptr;
    QLabel *lblSeats     = nullptr;
    QLabel *lblConflicts = nullptr;

    QWidget* makeStatCard(const QString &label,
                          const QString &value,
                          const QString &color,
                          QLabel **outValueLabel);
};