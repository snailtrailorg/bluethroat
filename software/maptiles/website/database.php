<?php

class Database {
    private static $conn = null;
    private function __construct() {}
    private function __clone() {}
    private function __wakeup() {}
    
    public static function getConnection(): mysqli {
        if (self::$conn === null) {
            $config = [];
            if (!file_exists(__DIR__ . '/config/db_config.php')) {
                error_log('数据库配置文件缺失');
                die(json_encode(['code' => __LINE__, 'message' => '数据库配置文件缺失']));
            } else {
                $config = include __DIR__ . '/config/db_config.php';
            }

            self::$conn = new mysqli($config['host'], $config['user'], $config['pass'], $config['name']);
            
            if (self::$conn->connect_errno) {
                die(json_encode(['code' => __LINE__, 'message' => '数据库连接失败']));
                error_log("数据库连接失败: " . self::$conn->connect_error);
            }
            
            if (!self::$conn->set_charset($config['charset'] ?? 'utf8mb4')) {
                die(json_encode(['code' => __LINE__, 'message' => '数据库字符集设置失败']));
                error_log("字符集设置失败: " . self::$conn->error);
            }
        }
        
        if (!self::$conn->ping()) {
            self::$conn->close();
            self::$conn = null;
            return self::getConnection();
        }
        
        return self::$conn;
    }
    
    public static function closeConnection(): void {
        if (self::$conn !== null) {
            self::$conn->close();
            self::$conn = null;
        }
    }
}
