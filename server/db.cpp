#include "db.h"
#include <sqlite3.h>
#include <sstream>

namespace db {

// ===== Statement =====

static void throwIf(int rc, sqlite3* db, const char* ctx) {
  if (rc != SQLITE_OK && rc != SQLITE_ROW && rc != SQLITE_DONE) {
    std::ostringstream oss;
    oss << ctx << ": (" << rc << ") " << sqlite3_errmsg(db);
    throw DbError(oss.str());
  }
}

Statement::Statement(sqlite3* db, const std::string& sql) {
  int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt_, nullptr);
  throwIf(rc, db, "sqlite3_prepare_v2");
}
Statement::~Statement() {
  if (stmt_) sqlite3_finalize(stmt_);
}

void Statement::bind(int idx, int64_t v) {
  throwIf(sqlite3_bind_int64(stmt_, idx, v), sqlite3_db_handle(stmt_), "bind int64");
}
void Statement::bind(int idx, double v) {
  throwIf(sqlite3_bind_double(stmt_, idx, v), sqlite3_db_handle(stmt_), "bind double");
}
void Statement::bind(int idx, const std::string& v) {
  throwIf(sqlite3_bind_text(stmt_, idx, v.c_str(), (int)v.size(), SQLITE_TRANSIENT),
          sqlite3_db_handle(stmt_), "bind text");
}
void Statement::bindNull(int idx) {
  throwIf(sqlite3_bind_null(stmt_, idx), sqlite3_db_handle(stmt_), "bind null");
}

bool Statement::step() {
  int rc = sqlite3_step(stmt_);
  if (rc == SQLITE_ROW) return true;
  if (rc == SQLITE_DONE) return false;
  throwIf(rc, sqlite3_db_handle(stmt_), "sqlite3_step");
  return false;
}

void Statement::reset() {
  int rc = sqlite3_reset(stmt_);
  throwIf(rc, sqlite3_db_handle(stmt_), "sqlite3_reset");
  sqlite3_clear_bindings(stmt_);
}

int Statement::colCount() const { return sqlite3_column_count(stmt_); }
std::string Statement::colName(int i) const { return sqlite3_column_name(stmt_, i); }
bool Statement::isNull(int i) const { return sqlite3_column_type(stmt_, i) == SQLITE_NULL; }
std::string Statement::getText(int i) const {
  const unsigned char* t = sqlite3_column_text(stmt_, i);
  return t ? reinterpret_cast<const char*>(t) : std::string();
}
int64_t Statement::getInt64(int i) const { return sqlite3_column_int64(stmt_, i); }
double Statement::getDouble(int i) const { return sqlite3_column_double(stmt_, i); }

// ===== Connection =====

Connection& Connection::instance() {
  static Connection inst;
  return inst;
}

void Connection::open(const std::string& path) {
  std::lock_guard<std::mutex> lk(mu_);
  if (db_) return; // ya abierta
  int rc = sqlite3_open(path.c_str(), &db_);
  if (rc != SQLITE_OK) {
    std::string msg = sqlite3_errmsg(db_);
    sqlite3_close_v2(db_);
    db_ = nullptr;
    throw DbError("sqlite3_open: " + msg);
  }
  // Recomendable para concurrencia de lectores:
  sqlite3_exec(db_, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);
  sqlite3_exec(db_, "PRAGMA synchronous=NORMAL;", nullptr, nullptr, nullptr);
}

void Connection::close() {
  std::lock_guard<std::mutex> lk(mu_);
  if (db_) {
    sqlite3_close_v2(db_);
    db_ = nullptr;
  }
}

void Connection::exec(const std::string& sql) {
  std::lock_guard<std::mutex> lk(mu_);
  if (!db_) throw DbError("exec: DB no abierta");
  char* err = nullptr;
  int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err);
  if (rc != SQLITE_OK) {
    std::string msg = err ? err : "unknown";
    sqlite3_free(err);
    throw DbError("sqlite3_exec: " + msg);
  }
}

Rows Connection::query(const std::string& sql) {
  std::lock_guard<std::mutex> lk(mu_);
  if (!db_) throw DbError("query: DB no abierta");
  Statement st(db_, sql);
  Rows out;
  while (st.step()) {
    Row r;
    int n = st.colCount();
    for (int i = 0; i < n; ++i) {
      r.emplace(st.colName(i), st.isNull(i) ? "" : st.getText(i));
    }
    out.emplace_back(std::move(r));
  }
  return out;
}

std::optional<std::string> Connection::scalar(const std::string& sql) {
  std::lock_guard<std::mutex> lk(mu_);
  if (!db_) throw DbError("scalar: DB no abierta");
  Statement st(db_, sql);
  if (st.step()) {
    return st.isNull(0) ? std::optional<std::string>{} : std::optional<std::string>{st.getText(0)};
  }
  return std::nullopt;
}

Statement Connection::prepare(const std::string& sql) {
  std::lock_guard<std::mutex> lk(mu_);
  if (!db_) throw DbError("prepare: DB no abierta");
  return Statement(db_, sql); // NRVO
}

sqlite3* Connection::raw() { return db_; }

} // namespace db