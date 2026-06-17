// DatabaseManager.cpp
#include "DatabaseManager.h"
#include <QSqlDatabase>
#include <QSqlQuery>

void DatabaseManager::initDB() {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("exam_seating.db");
    db.open();

    QSqlQuery query;
    query.exec("CREATE TABLE IF NOT EXISTS students ("
               "id TEXT PRIMARY KEY, name TEXT, department TEXT, exam TEXT)");
    query.exec("CREATE TABLE IF NOT EXISTS rooms ("
               "room_id TEXT, row INT, col INT, student_id TEXT)");
}

void DatabaseManager::assignSeat(const QString& studentId,
                                 const QString& roomId, int row, int col) {
    QSqlQuery q;
    q.prepare("INSERT INTO rooms VALUES (?, ?, ?, ?)");
    q.addBindValue(roomId); q.addBindValue(row);
    q.addBindValue(col);    q.addBindValue(studentId);
    q.exec();
}