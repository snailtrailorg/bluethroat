<?php
	// Executables defineation
	define('SETSID', '/usr/bin/setsid');
	define('PYTHON', '/usr/bin/python3');

	// Task directory path (with validity check)
	$taskDir = realpath(__DIR__ . '/../tasks');
	if ($taskDir === false) {
		die("Error: Task directory does not exist: " . __DIR__ . '/../tasks');
	}
	define('TASK_ROOT_FOLDER', $taskDir);

	// Download script path (with validity check)
	$scriptPath = realpath(__DIR__ . '/../scripts/download_tiles.py');
	if ($scriptPath === false || !is_file($scriptPath)) {
		die("Error: Download script does not exist: " . __DIR__ . '/../scripts/download_tiles.py');
	}
	define('DOWNLOAD_SCRIPT', $scriptPath);

	$DB_CONFIG = ['host' => 'localhost', 'user' => 'maptiles', 'pass' => '4pEm(YFWXZi5!1NO', 'name' => 'maptiles', 'charset' => 'utf8mb4'];
	define('DB_CONFIG', $DB_CONFIG);

	define('DOWNLOAD_OUTPUT_FILE', '/tmp/download_tile.output');
?>