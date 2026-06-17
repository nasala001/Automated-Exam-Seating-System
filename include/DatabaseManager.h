#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QString>

class DatabaseManager {
public:
    static void initDB();
    static void assignSeat(const QString& studentId, const QString& roomId, int row, int col);
};

#endif // DATABASEMANAGER_H
