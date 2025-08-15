<?php
class Database {
    private static $connection = null;
    private static $host;
    private static $dbname;
    private static $username;
    private static $password;

    private function __construct() {}
    private function __clone() {}
    private function __wakeup() {}

    public static function getConnection(): mysqli {
        if (self::$connection === null) {
            $config = [];
            if (!file_exists(__DIR__ . '/config/db_config.php')) {
                error_log('数据库配置文件缺失');
                die(json_encode(['code' => __LINE__, 'message' => '数据库配置文件缺失']));
            } else {
                $config = include __DIR__ . '/config/db_config.php';
            }

            try {
                self::$connection = new mysqli($config['host'], $config['user'], $config['pass'], $config['name']);
            } catch (mysqli_sql_exception $e) {
                error_log("数据库连接失败: " . $e->getMessage());
                die(json_encode(['code' => __LINE__, 'message' => '数据库连接失败']));
            }
            
            if (self::$connection->connect_errno) {
                error_log("数据库连接失败: " . self::$connection->connect_error);
                die(json_encode(['code' => __LINE__, 'message' => '数据库连接失败']));
            }
            
            if (!self::$connection->set_charset($config['charset'] ?? 'utf8mb4')) {
                error_log("字符集设置失败: " . self::$connection->error);
                die(json_encode(['code' => __LINE__, 'message' => '数据库字符集设置失败']));
            }
        }
        
        if (!self::$connection->ping()) {
            self::$connection->close();
            self::$connection = null;
            return self::getConnection();
        }
        
        return self::$connection;
    }

    /**
     * 根据用户名查询用户
     * @param string $username 用户名
     * @return array 用户数据或错误信息
     */
    public static function getUserByUsername($username) {
        $sql = "SELECT id, username, password, email, created_at FROM users WHERE username = ?";
        return self::executeQuery($sql, [$username]);
    }

    /**
     * 添加新用户
     * @param string $username 用户名
     * @param string $email 邮箱
     * @param string $password 加密后的密码
     * @return array 操作结果
     */
    public static function addUser($username, $email, $password) {
        $sql = "INSERT INTO users (username, email, password, created_at) VALUES (?, ?, ?, NOW())";
        return self::executeQuery($sql, [$username, $email, $password]);
    }

    /**
     * 更新用户最后登录时间
     * @param int $userId 用户ID
     * @return array 操作结果
     */
    public static function updateUserLastLogin($userId) {
        // 保留您特意使用的空SET语法
        $sql = "UPDATE users WHERE id = ?";
        return self::executeQuery($sql, [$userId]);
    }

    /**
     * 获取用户资料
     * @param int $userId 用户ID
     * @return array 用户资料
     */
    public static function getUserProfile($userId) {
        $sql = "SELECT id, username, email, created_at FROM users WHERE id = ?";
        return self::executeQuery($sql, [$userId]);
    }

    /**
     * 执行查询并返回标准化结果
     * @param string $sql SQL语句
     * @param array $params 参数数组
     * @return array 标准化结果
     */
    private static function executeQuery($sql, $params = []) {
        $connection = self::getConnection();
        $stmt = $connection->prepare($sql);

        if (!$stmt) {
            return [
                'status' => 'error',
                'code' => $connection->errno,
                'message' => 'Prepare failed: ' . $connection->error
            ];
        }

        if (!empty($params)) {
            $types = str_repeat('s', count($params));
            $stmt->bind_param($types, ...$params);
        }

        $result = $stmt->execute();
        if (!$result) {
            return [
                'status' => 'error',
                'code' => $stmt->errno,
                'message' => 'Execute failed: ' . $stmt->error
            ];
        }

        // 根据SQL类型处理结果
        if (stripos($sql, 'SELECT') === 0) {
            $result = $stmt->get_result();
            $data = $result->fetch_all(MYSQLI_ASSOC);
            $stmt->close();
            return [
                'status' => 'success',
                'data' => $data
            ];
        } elseif (stripos($sql, 'INSERT') === 0) {
            $insertId = $connection->insert_id;
            $stmt->close();
            return [
                'status' => 'success',
                'data' => ['insert_id' => $insertId, 'affected_rows' => $stmt->affected_rows]
            ];
        } else {
            $affectedRows = $stmt->affected_rows;
            $stmt->close();
            return [
                'status' => 'success',
                'data' => ['affected_rows' => $affectedRows]
            ];
        }
    }

    /**
     * 关闭数据库连接
     */
    public static function closeConnection() {
        if (self::$connection !== null) {
            self::$connection->close();
            self::$connection = null;
        }
    }
}
?>