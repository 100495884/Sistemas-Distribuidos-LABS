/*
 * log.x
 * Interfaz RPC para registrar operaciones de usuarios.
 */

struct LogEntry {
    string usuario<>;
    string operacion<>;
    string timestamp<>;
};

program LOGPROG {
    version LOGVERS {
        void LOG_EVENT(LogEntry) = 1;
    } = 1;
} = 0x31230000;
