<?php
    require_once __DIR__ . '/config/config.php';

    class Database {
        private static $m_connection = null;
        private static $m_error_message = '';

        private function __construct() {}
        private function __destruct() {}
        private function __clone() {}

        public static function getErrorMessage(): string {
            $temp_string = self::$m_error_message;
            self::$m_error_message = '';
            return $temp_string;
        }

        private static function set_error_message(string $error_message): void {
            self::$m_error_message = $error_message;
        }

        private static function connect(): bool {
            try {
                self::$m_connection = new mysqli('p:'. DB_CONFIG['host'], DB_CONFIG['user'], DB_CONFIG['pass'], DB_CONFIG['name']);
            } catch (mysqli_sql_exception $e) {
                self::set_error_message("Connect database failed: " . $e->getMessage());
                self::$m_connection = null;
                return false;
            }

            if (self::$m_connection->connect_errno) {
                self::set_error_message("Connect database failed: " . self::$m_connection->connect_error);
                self::$m_connection->close();
                self::$m_connection = null;
                return false;
            }
            
            if (!self::$m_connection->set_charset(DB_CONFIG['charset'])) {
                self::set_error_message("Set database charset failed: " . self::$m_connection->error);
                self::$m_connection->close();
                self::$m_connection = null;
                return false;
            }

            try {
                if (!self::$m_connection->query("SELECT 1")) {
                    throw new mysqli_sql_exception("Query SELECT 1 failed: " . self::$m_connection->error);
                }
            } catch (mysqli_sql_exception $e) {
                self::set_error_message("Check database connection failed: " . $e->getMessage());
                self::$m_connection->close();
                self::$m_connection = null;
                return false;
            }

            return true;
        }

        public static function getUserByEmail($email) {
            $sql = "SELECT id AS uid, email, password, role, register_time, last_login_time FROM users WHERE email = ?";
            return self::query($sql, 's', [$email]);
        }

        public static function getUserById($userId) {
            $sql = "SELECT id AS uid, email, role, register_time, last_login_time FROM users WHERE id = ?";
            return self::query($sql, 'i', [$userId]);
        }

        public static function getUserProfile($userId) {
            $sql = "SELECT u.id AS uid, u.email, u.role, u.register_time, u.last_login_time, COUNT(t.id) AS running_tasks 
                    FROM users u 
                    LEFT JOIN tasks t ON u.id = t.uid AND t.progress < 100 
                    WHERE u.id = ? 
                    GROUP BY u.id";
            return self::query($sql, 'i', [$userId]);
        }

        public static function addUser($email, $password) {
            $sql = "INSERT INTO users (email, password) VALUES (?, ?)";
            return self::query($sql, 'ss', [$email, $password]);
        }

        public static function updateUser($userId) {
            $sql = "UPDATE users SET last_login_time = CURRENT_TIMESTAMP WHERE id = ?";
            return self::query($sql, 'i', [$userId]);
        }

        public static function addTask($userId, $taskName, $west, $north, $east, $south, $zoomMin, $zoomMax, $url, $folder) {
            $sql = "INSERT INTO tasks (uid, name, west, north, east, south, zoom_min, zoom_max, url, folder) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
            return self::query($sql, 'isddddiiss', [$userId, $taskName, $west, $north, $east, $south, $zoomMin, $zoomMax, $url, $folder]);
        }

        public static function getTaskById($taskId) {
            $sql = "SELECT id AS tid, uid, name, west, north, east, south, zoom_min, zoom_max, url, progress, folder FROM tasks WHERE id = ?";
            return self::query($sql, 'i', [$taskId]);
        }

        public static function getUserTasks($userId) {
            $sql = "SELECT id AS tid, uid, name, west, north, east, south, zoom_min, zoom_max, url, progress FROM tasks WHERE uid = ?";
            return self::query($sql, 'i', [$userId]);
        }

        private static function query($sql, $types, $params = []) {
            if (self::$m_connection === null) {
                if (!self::connect()) {
                    return false;
                }
            }
            $stmt = self::$m_connection->prepare($sql);

            if (!$stmt) {
                self::set_error_message("Prepare statement failed: " . self::$m_connection->errno . self::$m_connection->error);
                return false;
            }

            if (!empty($params)) {
                if (!$stmt->bind_param($types, ...$params)) {
                    self::set_error_message("Bind parameters failed: " . $stmt->errno . self::$m_connection->error);
                    return false;
                }
            }

            $result = $stmt->execute();
            if (!$result) {
                self::set_error_message("Query failed: " . $stmt->errno . self::$m_connection->error);
                return false;
            }

            if (stripos($sql, 'SELECT') === 0) {
                $result = $stmt->get_result();
                $data = $result->fetch_all(MYSQLI_ASSOC);
                $stmt->close();
                return $data;
            } else if (stripos($sql, 'INSERT') === 0) {
                $insertId = self::$m_connection->insert_id;
                $rowCount = $stmt->affected_rows;
                $stmt->close();
                return ['insert_id' => $insertId, 'affected_rows' => $rowCount];
            } else {
                $affectedRows = $stmt->affected_rows;
                $stmt->close();
                return ['affected_rows' => $affectedRows];
            }
        }
    }
?>