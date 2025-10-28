#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <stdexcept>
#include <optional>
#include <cstdint>

// Forward-declare para no exponer <sqlite3.h> en todos lados
struct sqlite3;
struct sqlite3_stmt;

namespace db {

    // Fila = nombreColumna -> valor (como string)
    using Row  = std::unordered_map<std::string, std::string>;
    using Rows = std::vector<Row>;

    // Error de BD
    struct DbError : public std::runtime_error {
     using std::runtime_error::runtime_error;
    };

    // RAII para statements preparados
    class Statement {
        public:
            Statement(sqlite3* db, const std::string& sql);
            ~Statement();

            // bind por índice (1-based)
            void bind(int idx, int64_t v);
            void bind(int idx, double v);
            void bind(int idx, const std::string& v);
            void bindNull(int idx);

            // step() devuelve true si hay fila disponible
            bool step();
            // resetear para re-ejecutar
            void reset();

            // herramientas de lectura
            int colCount() const;
            std::string colName(int i) const;
            bool isNull(int i) const;
            std::string getText(int i) const;
            int64_t getInt64(int i) const;
            double getDouble(int i) const;

        private:
            sqlite3_stmt* stmt_ = nullptr;
    };

    // Conexión singleton simple (abre una sola vez y se comparte)
    class Connection {
        public:
            static Connection& instance();

            // Abre (o reabre) la BD si aún no está abierta
            void open(const std::string& path);
            // Cierra explícitamente (opcional; se cierra en el destructor del proceso)
            void close();

            // Atajos cómodos:
            // 1) Ejecutar DDL / DML (CREATE/INSERT/UPDATE/DELETE)
            void exec(const std::string& sql);

            // 2) Ejecutar y devolver todas las filas como vector<Row>
            Rows query(const std::string& sql);

            // 3) Escalar (primera columna de la primera fila) como string
            std::optional<std::string> scalar(const std::string& sql);

            // 4) Preparar un Statement para binds/loops
            Statement prepare(const std::string& sql);

            // Devuelve el puntero sqlite3* (por si necesitas API de bajo nivel)
            sqlite3* raw();

        private:
            Connection() = default;
            ~Connection() = default;
            Connection(const Connection&) = delete;
            Connection& operator=(const Connection&) = delete;

            sqlite3* db_ = nullptr;
            std::mutex mu_;
    };

} // namespace db