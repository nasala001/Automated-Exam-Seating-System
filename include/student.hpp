#ifndef STUDENT_HPP
#define STUDENT_HPP

#include "../database/sqlite3.h"

// CSV data database ma pathaune function
bool runDatabaseMigrationEngine(sqlite3* DB);

// Database bata final CSV generate garne function
void exportDatabaseToFinalCSV(sqlite3* DB);

#endif // STUDENT_HPP