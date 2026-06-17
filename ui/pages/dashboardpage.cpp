#include "dashboardpage.h"

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>

DashboardPage::DashboardPage(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(30, 30, 30, 30);
    layout->setSpacing(20);

    QLabel *title = new QLabel("Dashboard", this);
    title->setStyleSheet("font-size: 26px; font-weight: bold; color: #cdd6f4;");
    layout->addWidget(title);

    QLabel *subtitle = new QLabel("Overview of exam seating status", this);
    subtitle->setStyleSheet("font-size: 13px; color: #6c7086;");
    layout->addWidget(subtitle);

    QHBoxLayout *cards = new QHBoxLayout();
    cards->setSpacing(16);

    // Pass the QLabel** so we can update them later
    cards->addWidget(makeStatCard("Total Students", "0", "#89b4fa", &lblStudents));
    cards->addWidget(makeStatCard("Total Venues",   "0", "#a6e3a1", &lblVenues));
    cards->addWidget(makeStatCard("Total Seats",    "0", "#fab387", &lblSeats));
    cards->addWidget(makeStatCard("Conflicts",      "0", "#f38ba8", &lblConflicts));

    layout->addLayout(cards);
    layout->addStretch();
    setLayout(layout);
}

QWidget* DashboardPage::makeStatCard(const QString &label,
                                     const QString &value,
                                     const QString &color,
                                     QLabel **outValueLabel)
{
    QFrame *card = new QFrame(this);
    card->setMinimumHeight(110);
    card->setStyleSheet(QString(
                            "QFrame {"
                            "  background: #313244;"
                            "  border-radius: 10px;"
                            "  border-left: 5px solid %1;"
                            "}"
                            ).arg(color));

    QVBoxLayout *l = new QVBoxLayout(card);
    l->setContentsMargins(16, 14, 16, 14);

    QLabel *val = new QLabel(value, card);
    val->setStyleSheet(QString(
                           "font-size: 32px; font-weight: bold; color: %1; border: none;"
                           ).arg(color));

    QLabel *lbl = new QLabel(label, card);
    lbl->setStyleSheet("font-size: 13px; color: #a6adc8; border: none;");

    l->addWidget(val);
    l->addWidget(lbl);

    // Save the value label pointer so updateStats() can change it
    if (outValueLabel) *outValueLabel = val;

    return card;
}

void DashboardPage::updateStats(int students, int venues,
                                int totalSeats, int conflicts)
{
    if (lblStudents)  lblStudents ->setText(QString::number(students));
    if (lblVenues)    lblVenues   ->setText(QString::number(venues));
    if (lblSeats)     lblSeats    ->setText(QString::number(totalSeats));
    if (lblConflicts) lblConflicts->setText(QString::number(conflicts));
}