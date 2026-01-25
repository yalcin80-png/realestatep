// MergeDatabases.h / .cpp
#include <sqlite3.h>
#include <string>
#include <vector>
#include <unordered_set>
#include <sstream>

static bool Exec(sqlite3* db, const std::string& sql, std::string& err) {
    char* zErr = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &zErr);
    if (rc != SQLITE_OK) {
        err = zErr ? zErr : "sqlite3_exec failed";
        sqlite3_free(zErr);
        return false;
    }
    return true;
}

static bool QueryStrings(sqlite3* db, const std::string& sql, std::vector<std::string>& out, std::string& err) {
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) { err = sqlite3_errmsg(db); return false; }

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        const unsigned char* t = sqlite3_column_text(stmt, 0);
        out.emplace_back(t ? reinterpret_cast<const char*>(t) : "");
    }
    if (rc != SQLITE_DONE) { err = sqlite3_errmsg(db); sqlite3_finalize(stmt); return false; }
    sqlite3_finalize(stmt);
    return true;
}

static bool QueryTableColumns(sqlite3* db, const std::string& schema, const std::string& table,
                              std::vector<std::string>& columns, std::string& err) {
    // PRAGMA schema.table_info(table)
    std::ostringstream oss;
    oss << "PRAGMA " << schema << ".table_info('" << table << "');";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, oss.str().c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) { err = sqlite3_errmsg(db); return false; }

    // table_info: cid, name, type, notnull, dflt_value, pk
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        const unsigned char* name = sqlite3_column_text(stmt, 1);
        if (name) columns.emplace_back(reinterpret_cast<const char*>(name));
    }
    if (rc != SQLITE_DONE) { err = sqlite3_errmsg(db); sqlite3_finalize(stmt); return false; }
    sqlite3_finalize(stmt);
    return true;
}

static bool HasCols(const std::vector<std::string>& cols, std::initializer_list<const char*> need) {
    std::unordered_set<std::string> s(cols.begin(), cols.end());
    for (auto* n : need) {
        if (s.find(n) == s.end()) return false;
    }
    return true;
}

static std::string Join(const std::vector<std::string>& cols, const std::string& sep) {
    std::ostringstream oss;
    for (size_t i = 0; i < cols.size(); ++i) {
        if (i) oss << sep;
        oss << cols[i];
    }
    return oss.str();
}

static std::string QualCols(const std::vector<std::string>& cols, const std::string& alias, const std::string& sep) {
    std::ostringstream oss;
    for (size_t i = 0; i < cols.size(); ++i) {
        if (i) oss << sep;
        oss << alias << "." << cols[i];
    }
    return oss.str();
}

static std::string SetListFromCloud(const std::vector<std::string>& cols,
                                    const std::string& cloudAlias,
                                    const std::string& sep) {
    // col = c.col, ...
    std::ostringstream oss;
    for (size_t i = 0; i < cols.size(); ++i) {
        if (i) oss << sep;
        oss << cols[i] << " = " << cloudAlias << "." << cols[i];
    }
    return oss.str();
}

static std::string ColumnsExcept(const std::vector<std::string>& cols, const std::unordered_set<std::string>& excl) {
    std::vector<std::string> filtered;
    filtered.reserve(cols.size());
    for (auto& c : cols) {
        if (excl.find(c) == excl.end()) filtered.push_back(c);
    }
    return Join(filtered, ",");
}

// Core: merge cloud.<table> into main.<table> using syncId key and updatedAt resolution.
static bool MergeOneTable(sqlite3* db, const std::string& table, const std::vector<std::string>& cols, std::string& err) {
    // We assume syncId is the logical key. Ideally it's UNIQUE/PK.
    // We'll do:
    // 1) INSERT rows missing in local
    // 2) UPDATE rows where cloud.updatedAt > local.updatedAt

    // Avoid touching generated/rowid-like columns if any; we keep everything except nothing special here.
    // If you have an autoincrement "id" that differs per device, do NOT include it in sync tables.
    // We'll just use all columns as-is.
    const std::string colList = Join(cols, ",");
    const std::string cloudCols = QualCols(cols, "c", ",");

    // 1) Insert missing
    {
        std::ostringstream sql;
        sql
            << "INSERT INTO main.\"" << table << "\"(" << colList << ") "
            << "SELECT " << cloudCols << " "
            << "FROM cloud.\"" << table << "\" c "
            << "LEFT JOIN main.\"" << table << "\" l ON l.syncId = c.syncId "
            << "WHERE l.syncId IS NULL;";
        if (!Exec(db, sql.str(), err)) return false;
    }

    // 2) Update newer (including deleted tombstones)
    // SQLite doesn't support UPDATE ... FROM in all builds, so we use correlated subqueries.
    // For each column X: X = (SELECT c.X FROM cloud.table c WHERE c.syncId = main.table.syncId)
    // Only when EXISTS and cloud.updatedAt > main.updatedAt
    {
        std::ostringstream sets;
        for (size_t i = 0; i < cols.size(); ++i) {
            if (i) sets << ", ";
            const auto& col = cols[i];
            sets << "\"" << col << "\" = (SELECT c.\"" << col << "\" FROM cloud.\"" << table
                 << "\" c WHERE c.syncId = main.\"" << table << "\".syncId)";
        }

        std::ostringstream sql;
        sql
            << "UPDATE main.\"" << table << "\" "
            << "SET " << sets.str() << " "
            << "WHERE EXISTS (SELECT 1 FROM cloud.\"" << table << "\" c WHERE c.syncId = main.\"" << table << "\".syncId) "
            << "AND (SELECT c.updatedAt FROM cloud.\"" << table << "\" c WHERE c.syncId = main.\"" << table << "\".syncId) "
            << "    > main.\"" << table << "\".updatedAt;";
        if (!Exec(db, sql.str(), err)) return false;
    }

    return true;
}

// Public API
// localDb must already be opened (sqlite3_open).
// cloudDbPath points to the downloaded DB file from Drive.
bool MergeDatabases(sqlite3* localDb, const std::string& cloudDbPath, std::string& err) {
    if (!localDb) { err = "localDb is null"; return false; }

    // Safety: one big transaction
    if (!Exec(localDb, "BEGIN IMMEDIATE;", err)) return false;

    // Pragmas for safer merge
    // If you have heavy FK constraints across tables, disabling during merge can help.
    // You can remove these if you want strict enforcement.
    Exec(localDb, "PRAGMA foreign_keys=OFF;", err); // ignore failure
    Exec(localDb, "PRAGMA defer_foreign_keys=ON;", err);

    // Attach cloud db
    {
        std::ostringstream oss;
        oss << "ATTACH DATABASE '" << cloudDbPath << "' AS cloud;";
        if (!Exec(localDb, oss.str(), err)) {
            Exec(localDb, "ROLLBACK;", err);
            return false;
        }
    }

    // Get table list from main (exclude sqlite internal)
    std::vector<std::string> tables;
    if (!QueryStrings(localDb,
        "SELECT name FROM sqlite_master WHERE type='table' AND name NOT LIKE 'sqlite_%';",
        tables, err)) {
        Exec(localDb, "DETACH DATABASE cloud;", err);
        Exec(localDb, "ROLLBACK;", err);
        return false;
    }

    // For each table, ensure cloud has it too and that schema has syncId/updatedAt/deleted
    for (const auto& table : tables) {
        // Check cloud has the table
        std::vector<std::string> cloudHas;
        std::ostringstream chk;
        chk << "SELECT name FROM cloud.sqlite_master WHERE type='table' AND name='" << table << "';";
        if (!QueryStrings(localDb, chk.str(), cloudHas, err)) {
            Exec(localDb, "DETACH DATABASE cloud;", err);
            Exec(localDb, "ROLLBACK;", err);
            return false;
        }
        if (cloudHas.empty()) continue; // No matching table in cloud -> skip.

        std::vector<std::string> cols;
        if (!QueryTableColumns(localDb, "main", table, cols, err)) {
            Exec(localDb, "DETACH DATABASE cloud;", err);
            Exec(localDb, "ROLLBACK;", err);
            return false;
        }

        // Only merge tables that follow your sync contract
        if (!HasCols(cols, {"syncId", "updatedAt", "deleted"})) continue;

        if (!MergeOneTable(localDb, table, cols, err)) {
            Exec(localDb, "DETACH DATABASE cloud;", err);
            Exec(localDb, "ROLLBACK;", err);
            return false;
        }
    }

    // Cleanup
    Exec(localDb, "DETACH DATABASE cloud;", err);

    // Restore pragmas
    Exec(localDb, "PRAGMA foreign_keys=ON;", err);

    if (!Exec(localDb, "COMMIT;", err)) {
        Exec(localDb, "ROLLBACK;", err);
        return false;
    }
    return true;
}
