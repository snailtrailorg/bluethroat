<?php
    require_once __DIR__ . '/modules/phpseclib/vendor/autoload.php';
    require_once __DIR__ . '/config/config.php';
    require_once __DIR__ . '/database.php';

    use phpseclib3\Crypt\PublicKeyLoader;
    use phpseclib3\Crypt\RSA;

    function rsa_oaep_decrypt($encrypted) {
        try {
            // Suppress editor warnings for dynamic type inference
            /** @var \phpseclib3\Crypt\RSA\PrivateKey $rsaKey */
            $rsaKey = PublicKeyLoader::load(file_get_contents(__DIR__ . '/config/rsakeys/privatekey.pem'));
            $private_key = $rsaKey->withHash('sha256')->withMGFHash('sha256')->withPadding(RSA::ENCRYPTION_OAEP);
            $decrypted = $private_key->decrypt(base64_decode($encrypted));
            return $decrypted;
        } catch (Exception $e) {
            error_log("RSA解密错误：" . $e->getMessage());
            return null;
        }
    }

    function runTask($taskId) {
        $task = Database::getTaskById($taskId);

        if ($task === false) {
            error_log("获取任务失败：" . Database::getErrorMessage());
            return false;
        }

        if (count($task) === 0) {
            error_log("任务不存在");
            return false;
        }

        $task = $task[0];

        $uid = $task['uid'];
        if ($uid !== $_SESSION['user_id']) {
            error_log("用户ID不匹配");
            return false;
        }

        $tid = $task['tid'];
        $west = $task['west'];
        $north = $task['north'];
        $east = $task['east'];
        $south = $task['south'];
        $zoom_min = $task['zoom_min'];
        $zoom_max = $task['zoom_max'];
        $url = $task['url'];
        $folder = $task['folder'] . '/' . $uid . '/' . $tid;
        if (!isset($west) || !isset($north) || !isset($east) || !isset($south) || !isset($zoom_min) || !isset($zoom_max) || !isset($url) || !isset($folder)) {
            error_log("任务参数不完整");
            return false;
        } else {
            global $DOWNLOAD_SCRIPT;
            global $SETSID;
            $command = sprintf('%s -z %d %d -u %s -o %s -t %d %f %f %f %f', $DOWNLOAD_SCRIPT, $zoom_min, $zoom_max, escapeshellarg($url), escapeshellarg($folder), $tid, $west, $north, $east, $south);
            $dummy = [];
            $exec_cmd = sprintf('%s %s > /tmp/download.log 2>&1 &', escapeshellcmd($SETSID), escapeshellcmd($command));
            exec($exec_cmd, $dummy, $code);
            if ($code !== 0) {
                error_log("任务执行失败：" . $code);
                return false;
            } else {
                error_log($exec_cmd);
                return true;
            }
        }
    }

    session_start();

    $public_key = preg_replace('/-----(BEGIN|END)\s+(.*?)\s+KEY-----|\s/', '', file_get_contents(__DIR__ . '/config/rsakeys/publickey.pem'));

    if ($_SERVER['REQUEST_METHOD'] === 'POST') {
        header("Content-Type: application/json");

        if (isset($_POST) && isset($_POST['action'])) {
            switch ($_POST['action']) {
            case 'login':
                {
                    if (!isset($_POST['email']) || !isset($_POST['password'])) {
                        die(json_encode(['code' => __LINE__, 'message' => '请求参数错误']));
                    }

                    $email = $_POST['email'];
                    if (!filter_var($email, FILTER_VALIDATE_EMAIL)) {
                        die(json_encode(['code' => __LINE__, 'message' => '邮箱格式不合法']));
                    }

                    $password_pseudo = rsa_oaep_decrypt($_POST['password']);
                    if (!preg_match('/^[a-f0-9]{64}$/', $password_pseudo)) {
                        die(json_encode(['code' => __LINE__, 'message' => '密码格式不合法']));
                    }

                    $result = Database::getUserByEmail($email);
                    if ($result === false) {
                        die(json_encode(['code' => __LINE__, 'message' => Database::getErrorMessage()]));
                    } else if (count($result) === 0) {
                        die(json_encode(['code' => __LINE__, 'message' => '用户不存在']));
                    }
                    else {
                        $user_id = $result[0]['id'];
                        $email = $result[0]['email'];
                        $role = $result[0]['role'];
                        $register_time = $result[0]['register_time'];
                        $last_login_time = $result[0]['last_login_time'];
                        $password_digest = $result[0]['password'];
                        if (!password_verify($password_pseudo, $password_digest)) {
                            die(json_encode(['code' => __LINE__, 'message' => '密码不匹配']));
                        }
                    }

                    $result = Database::updateUser($user_id);
                    if ($result === false) {
                        die(json_encode(['code' => __LINE__, 'message' => Database::getErrorMessage()]));
                    }

                    $_SESSION['user_id'] = $user_id;
                    echo json_encode(['code' => 0, 'message' => '登录成功', 'data' => ['user_id' => $user_id, 'email' => $email, 'role' => $role, 'register_time' => $register_time, 'last_login_time' => $last_login_time]]);
                }
                break;
            case 'register':
                {
                    if (!isset($_POST['email']) || !isset($_POST['password'])) {
                        die(json_encode(['code' => __LINE__, 'message' => '请求参数错误']));
                    }

                    $email = $_POST['email'];
                    if (!filter_var($email, FILTER_VALIDATE_EMAIL)) {
                        die(json_encode(['code' => __LINE__, 'message' => '邮箱格式不合法']));
                    }

                    $password_pseudo = rsa_oaep_decrypt($_POST['password']);
                    if (!preg_match('/^[a-f0-9]{64}$/', $password_pseudo)) {
                        die(json_encode(['code' => __LINE__, 'message' => '密码格式不合法']));
                    } else {
                        $password_digest = password_hash($password_pseudo, PASSWORD_BCRYPT);
                    }

                    $result = Database::getUserByEmail($email);
                    if ($result === false) {
                        die(json_encode(['code' => __LINE__, 'message' => Database::getErrorMessage()]));
                    } else if (count($result) > 0) {
                        die(json_encode(['code' => __LINE__, 'message' => '邮箱已存在']));
                    }

                    $result = Database::addUser($email, $password_digest);
                    if ($result === false) {
                        die(json_encode(['code' => __LINE__, 'message' => Database::getErrorMessage()]));
                    } else {
                        echo json_encode(['code' => 0, 'message' => '注册成功', 'data' => ['uid' => $result['insert_id'], 'email' => $email]]);
                    }
                }
                break;
            case 'get_profile':
                {
                    if (!isset($_POST['user_id'])) {
                        die(json_encode(['code' => __LINE__, 'message' => '请求参数错误']));
                    }

                    if (!isset($_SESSION['user_id']) || $_SESSION['user_id'] != $_POST['user_id']) {
                        die(json_encode(['code' => __LINE__, 'message' => '用户未登录或会话已过期']));
                    }

                    $result = Database::getUserProfile($_POST['user_id']);
                    if ($result === false) {
                        die(json_encode(['code' => __LINE__, 'message' => Database::getErrorMessage()]));
                    } else if (count($result) === 0) {
                        die(json_encode(['code' => __LINE__, 'message' => '用户不存在']));
                    } else {
                        echo json_encode(['code' => 0, 'message' => '获取用户信息成功', 'data' => $result[0]]);
                    }
                }
                break;
            case 'logout':
                {
                    if (!isset($_POST['user_id'])) {
                        die(json_encode(['code' => __LINE__, 'message' => '请求参数错误']));
                    }

                    if (!isset($_SESSION['user_id']) || $_SESSION['user_id'] != $_POST['user_id']) {
                        die(json_encode(['code' => __LINE__, 'message' => '用户未登录或会话已过期']));
                    }

                    $_SESSION['user_id'] = null;
                    echo json_encode(['code' => 0, 'message' => '登出成功']);
                }
                break;
            case 'download':
                {
                    if (!isset($_POST['user_id']) || !isset($_SESSION['user_id']) || $_SESSION['user_id'] != $_POST['user_id']) {
                        die(json_encode(['code' => __LINE__, 'message' => '用户未登录或会话已过期']));
                    }

                    if (!isset($_POST['west']) || !isset($_POST['north']) || !isset($_POST['east']) || !isset($_POST['south']) || !isset($_POST['task_name']) || !isset($_POST['zoom_min']) || !isset($_POST['zoom_max']) || !isset($_POST['url'])) {
                        die(json_encode(['code' => __LINE__, 'message' => '请求参数错误']));
                    }

                    $west = floatval($_POST['west']);
                    $north = floatval($_POST['north']);
                    $east = floatval($_POST['east']);
                    $south = floatval($_POST['south']);
                    if ($west < -180 || $west > 180 || $north < -85.05112878 || $north > 85.05112878 || $east < -180 || $east > 180 || $south < -85.05112878 || $south > 85.05112878) {
                        die(json_encode(['code' => __LINE__, 'message' => '请求参数错误']));
                    }

                    $zoom_min = intval($_POST['zoom_min']);
                    $zoom_max = intval($_POST['zoom_max']);
                    if ($zoom_min < 1 || $zoom_min > 25 || $zoom_max < 1 || $zoom_max > 25 || $zoom_min > $zoom_max) {
                        die(json_encode(['code' => __LINE__, 'message' => '请求参数错误']));
                    }

                    $task_name = $_POST['task_name'];
                    if ($task_name === '') {
                        die(json_encode(['code' => __LINE__, 'message' => '请求参数错误']));
                    }

                    $url = $_POST['url'];
                    if (!filter_var($url, FILTER_VALIDATE_URL)) {
                        die(json_encode(['code' => __LINE__, 'message' => '请求参数错误']));
                    }

                    $result = Database::addTask($_POST['user_id'], $task_name, $west, $north, $east, $south, $zoom_min, $zoom_max, $url, $TASK_ROOT_FOLDER);
                    if ($result === false) {
                        die(json_encode(['code' => __LINE__, 'message' => Database::getErrorMessage()]));
                    } else {
                        if (!runTask($result['insert_id'])) {
                            die(json_encode(['code' => __LINE__, 'message' => '运行任务失败', 'data' => ['task_id' => $result['insert_id']]]));
                        } else {
                            echo json_encode(['code' => 0, 'message' => '添加任务成功', 'data' => ['task_id' => $result['insert_id']]]);
                        }
                    }
                }
                break;
            default:
                die(json_encode(['code' => __LINE__, 'message' => '请求参数错误']));
                break;
            }
        } else {
            die(json_encode(['code' => __LINE__, 'message' => '请求参数错误']));
        }
    } else {
?>
<!DOCTYPE html>
<html>
    <head>
        <title>地图瓦片标记和下载</title>
        <meta name="robots" content="noindex, nofollow">
        <meta charset="utf-8">
        <script>
            var public_key = null;
            var user_id = null;
        </script>
        <script src="https://kit.fontawesome.com/d1f7300f56.js" crossorigin="anonymous"></script>
        <script type="module" src="./maptiles.js"></script>
        <style>
            html,body{height:100%;margin:0;padding:0;}
            #map {height: 100%;}
            #maptiles_canvas {position:absolute;background-color:rgba(0, 0, 0, 0.5);visibility:hidden}
            .map-control{display:flex;align-items:center;position:absolute;margin:10px}
            .pop-window{visibility:hidden;display:flex;flex-direction:column;background-color:#D7D7D3;border-radius:10px;position:fixed;top:50%;left:50%;transform:translate(-50%,-50%);z-index:1000}
            .title-bar{display:flex;align-items:center;justify-content:center;background-color:darkblue;height:40px;border-top-left-radius:10px;border-top-right-radius:10px}
            .content{display:grid;grid-template-columns:auto auto;grid-gap:10px;margin:10px}
            .footer-bar{display:flex;align-items:center;justify-content:flex-end;margin:10px;border-bottom-left-radius:10px;border-bottom-right-radius:10px}
            .label-grid{display:flex;align-items:center;justify-content: flex-end;}
            .input-grid{display:flex;align-items:center;justify-content: flex-start;}
            .input,.button{height:40px;padding:0 10px;box-sizing:border-box}
            .input,.button,.label{font-size:large;font-family:'Roboto',Arial,sans-serif;white-space:nowrap;}
            .title{font-size:large;color:white;font-family:'Roboto',Arial,sans-serif;}
            .button{width:88px}
            .button:hover{background-color:lightblue}
            .left-margin{margin-left:10px;}
            .link{color: darkblue;text-decoration:none;white-space:nowrap;}
            .link:hover{cursor:pointer;color:blue;text-decoration:underline;}
            .progress-container{width:100px;background-color:dimgray;border-radius:4px;overflow:hidden;}
            .progress-bar{height:24px;background-color:#4CAF50;width:0%;text-align:center;line-height:24px;color:white;transition:width 0.5s ease;}
        </style>
    </head>

    <body>
        <div id="map" width=200 height=200></div>

        <div id="maptiles_canvas"></div>

        <div class="map-control" id="geocoder_control">
            <input class="input" id="geocoder_address_input" placeholder="搜索一个位置..." aria-label="Search input">
            <button class="button left-margin" id="geocoder_search_button" aria-label="Search button"><i class="fa-solid fa-magnifying-glass"></i>&nbsp;搜索</button>
        </div>

        <div class="map-control" id="maptiles_control">
            <button class="button" id="maptiles_mark_button" aria-label="Mark button"><i class="fa-solid fa-vector-square"></i>&nbsp;标记</button>
            <button class="button left-margin" id="maptiles_clear_button" aria-label="Clear button"><i class="fa fa-eraser"></i>&nbsp;清除</button>
            <button class="button left-margin" id="maptiles_download_button" aria-label="Download button"><i class="fa fa-download"></i>&nbsp;下载</button>
            <button class="button left-margin" id="maptiles_task_button" aria-label="Task button"><i class="fa-solid fa-list-check"></i>&nbsp;任务</button>
            <button class="button left-margin" id="maptiles_account_button" aria-label="Account button"><i class="fa-solid fa-user"></i>&nbsp;登录</button>
        </div>

        <div class="pop-window" id="download_window">
            <div class="title-bar"><span class="title">下载地图瓦片</span></div>
            <form id="download_form" method="post">
                <input type="hidden" name="action" value="download">
                <input type="hidden" id="download_form_west" name="west" value="114.90398307">
                <input type="hidden" id="download_form_north" name="north" value="22.67742356">
                <input type="hidden" id="download_form_east" name="east" value="114.96930022">
                <input type="hidden" id="download_form_south" name="south" value="22.63853325">
                <div class="content">
                    <div class="label-grid"><span class="label" title="格式：[左侧经度, 顶部纬度, 右侧经度, 底部纬度]，根据选择区域自动计算，用户不可修改">地图区域坐标<i class="fa-regular fa-circle-question" style="color:#4285F4;"></i>：</span></div>
                    <div class="input-grid"><span class="label" id="download_form_bounds">[114.903983, 22.677423, 114.969300, 22.638533]</span></div>
                    <div class="label-grid"><span class="label">下载工作量：</span></div>
                    <div class="input-grid"><span class="label" id="download_form_tile_count">计算中...</span></div>
                    <div class="label-grid"><span class="label">下载任务名称：</span></div>
                    <div class="input-grid"><input class="input" style="width:218px" type="text" name="task_name" placeholder="输入名称以区分不同任务..." maxlength="63" required></div>
                    <div class="label-grid"><span class="label">地图瓦片级别：</span></div>
                    <div class="input-grid">
                        <span class="label">从</span>
                        <input class="input left-margin" id="download_form_zoom_min" name="zoom_min" type="number" value="12" min="1" max="20" aria-label="Minimum zoom level">
                        <span class="label left-margin">至</span>
                        <input class="input left-margin" id="download_form_zoom_max" name="zoom_max" type="number" value="15" min="1" max="20" aria-label="Maximum zoom level">
                    </div>
                    <div class="label-grid"><span class="label" title="URL中必须包含{x}，{y}和{z}，下载过程中将被分别替换为地图瓦片的列序号，行序号和缩放级别。">瓦片服务器URL<i class="fa-regular fa-circle-question" style="color:#4285F4;"></i>：</span></div>
                    <div class="input-grid"><input class="input" style="width:500px" type="text" name="url" placeholder="https://tile.server.com/{z}/{x}/{y}.png?key=SERVER_KEY" maxlength="511" required></div>
                </div>
                <div class="footer-bar">
                    <button id="download_form_cancel" class="button" type="reset"><i class="fa-solid fa-xmark"></i>&nbsp;取消</button>
                    <button id="download_form_submit" class="button left-margin" type="submit"><i class="fa-solid fa-check"></i>&nbsp;提交</button>
                </div>
            </form>
        </div>

        <div class="pop-window" id="task_window">
            <div class="title-bar"><span class="title">下载任务列表</span></div>
            <div class="content">
                <div class="label-grid"><span class="label">小径湾滑翔伞基地周边OSM地图12-18级</span></div>
                <div class="input-grid">
                    <div class="progress-container"><div class="progress-bar" style="width: 75%">75%</div></div>
                    <span class="label link left-margin"><i class="fa-solid fa-circle-info"></i>&nbsp;详情</span>
                    <span class="label link left-margin"><i class="fa-solid fa-download"></i>&nbsp;下载到本地</span>
                </div>
            </div>
            <form id="task_form" method="post">
                <input type="hidden" name="task" value="1">
                <div class="footer-bar">
                    <button id="task_form_previous" class="button" type="submit"><i class="fa-solid fa-arrow-left"></i>&nbsp;上页</button>
                    <button id="task_form_next" class="button left-margin" type="submit"><i class="fa-solid fa-arrow-right"></i>&nbsp;下页</button>
                    <button id="task_form_refresh" class="button left-margin" type="submit"><i class="fa-solid fa-rotate-right"></i>&nbsp;刷新</button>
                    <button id="task_form_cancel" class="button left-margin" type="reset"><i class="fa-solid fa-xmark"></i>&nbsp;取消</button>
                </div>
            </form>
        </div>

        <div class="pop-window" id="login_window">
            <div class="title-bar"><span class="title">登录账号</span></div>
            <form id="login_form" method="post">
                <input type="hidden" name="action" value="login">
                <div class="content">
                    <div class="label-grid"><span class="label">E-MAIL地址：</span></div>
                    <div class="input-grid">
                        <input class="input" style="width:300px" type="text" name="email" placeholder="输入E-MAIL地址作为登录名..." required>
                        <span class="label link left-margin" id="register_link"><i class="fa-solid fa-user-plus"></i>&nbsp;注册账号</span>
                    </div>
                    <div class="label-grid"><span class="label">密码：</span></div>
                    <div class="input-grid">
                        <input class="input" style="width:300px" type="password" name="password" required>
                        <span class="label link left-margin" id="reset_password_link"><i class="fa-solid fa-gear"></i>&nbsp;重设密码</span>
                    </div>
                </div>
                <div class="footer-bar">
                    <button id="login_form_cancel" class="button" type="reset"><i class="fa-solid fa-xmark"></i>&nbsp;取消</button>
                    <button id="login_form_submit" class="button left-margin" type="submit"><i class="fa-solid fa-check"></i>&nbsp;登录</button>
                </div>
            </form>
        </div>

        <div class="pop-window" id="register_window">
            <div class="title-bar"><span class="title">注册账号</span></div>
            <form id="register_form" method="post">
                <input type="hidden" name="action" value="register">
                <div class="content">
                    <div class="label-grid"><span class="label">E-MAIL地址：</span></div>
                    <div class="input-grid"><input class="input" style="width:300px" type="email" name="email" placeholder="输入E-MAIL地址作为登录名..." maxlength="63" required></div>
                    <div class="label-grid"><span class="label" title="密码必须包含至少8个字符，包括大小写字母、数字和特殊字符，&#10;并且不能包含123、abc这类简单连续的数字或字母。">密码<i class="fa-regular fa-circle-question" style="color:#4285F4;"></i>：</span></div>
                    <div class="input-grid"><input class="input" style="width:300px" type="password" name="password" pattern="^(?=.*\d)(?=.*[A-Z])(?=.*[a-z])(?=.*[^\w\s])(?!.*(123|abc|ABC|Abc)).{10,}$" required></div>
                    <div class="label-grid"><span class="label">确认密码：</span></div>
                    <div class="input-grid"><input class="input" style="width:300px" type="password" name="password_confirm" required></div>
                </div>
                <div class="footer-bar">
                    <button id="register_form_cancel" class="button" type="reset"><i class="fa-solid fa-xmark"></i>&nbsp;取消</button>
                    <button id="register_form_submit" class="button left-margin" type="submit"><i class="fa-solid fa-check"></i>&nbsp;注册</button>
                </div>
            </form>
        </div>

        <div class="pop-window" id="reset_password_window">
            <div class="title-bar"><span class="title">重新设置密码</span></div>
            <form id="reset_password_form" method="post">
                <input type="hidden" name="reset_password" value="1">
                <div class="content">
                    <div class="label-grid"><span class="label">E-MAIL地址：</span></div>
                    <div class="input-grid">
                        <input class="input" style="width:300px" type="text" name="email" placeholder="输入E-MAIL地址接收验证码..." required>
                        <span class="label link left-margin" id="send_verification_link"><i class="fa-solid fa-envelope"></i>&nbsp;发送验证码</span>
                    </div>
                    <div class="label-grid"><span class="label">验证码：</span></div>
                    <div class="input-grid"><input class="input" style="width:300px" type="text" name="verification" placeholder="输入验证码..." required></div>
                    <div class="label-grid"><span class="label">密码：</span></div>
                    <div class="input-grid"><input class="input" style="width:300px" type="password" name="password" required></div>
                    <div class="label-grid"><span class="label">确认密码：</span></div>
                    <div class="input-grid"><input class="input" style="width:300px" type="password" name="password_confirm" required></div>
                </div>
                <div class="footer-bar">
                    <button id="reset_password_form_cancel" class="button" type="reset"><i class="fa-solid fa-xmark"></i>&nbsp;取消</button>
                    <button id="reset_password_form_submit" class="button left-margin" type="submit"><i class="fa-solid fa-check"></i>&nbsp;重设</button>
                </div>
            </form>
        </div>

        <div class="pop-window" id="profile_window">
            <div class="title-bar"><span class="title">账号信息</span></div>
            <div class="content">
                <div class="label-grid"><span class="label">E-MAIL地址：</span></div>
                <div class="input-grid"><span class="label" id="profile_email"></span></div>
                <div class="label-grid"><span class="label">注册时间：</span></div>
                <div class="input-grid"><span class="label" id="profile_register_time"></span></div>
                <div class="label-grid"><span class="label">运行中的任务数：</span></div>
                <div class="input-grid">
                    <span class="label" id="profile_running_task_count">0</span>
                    <span class="label link left-margin" id="profile_task_link"><i class="fa-solid fa-list-check"></i>&nbsp;任务列表</span>
                </div>
            </div>
            <div class="footer-bar">
                <form id="logout_form" method="post">
                    <input type="hidden" name="action" value="logout">
                    <button id="logout_form_submit" class="button" type="submit"><i class="fa-solid fa-right-from-bracket"></i>&nbsp;登出</button>
                    <button id="logout_form_cancel" class="button left-margin" type="reset"><i class="fa-solid fa-xmark"></i>&nbsp;关闭</button>
                </form>
            </div>
        </div>

        <script>
            <?php if (isset($_SESSION['user_id'])) { ?>
                user_id = <?php echo $_SESSION['user_id']; ?>;
                document.getElementById("maptiles_account_button").innerHTML = '<i class="fa-solid fa-user"></i>&nbsp;详情';
            <?php } ?>
            <?php if ($public_key != '') { ?>
                window.crypto.subtle.importKey('spki', Uint8Array.from(atob('<?php echo addslashes($public_key); ?>'), c => c.charCodeAt(0)), {name:'RSA-OAEP', hash:{name:'SHA-256'}}, true, ['encrypt']).then(key => { public_key = key;}).catch(error => { console.error('Error importing public key:', error); public_key = null;});
            <?php } ?>
        </script>

        <script>
            (g=>{var h,a,k,p="The Google Maps JavaScript API",c="google",l="importLibrary",q="__ib__",m=document,b=window;b=b[c]||(b[c]={});var d=b.maps||(b.maps={}),r=new Set,e=new URLSearchParams,u=()=>h||(h=new Promise(async(f,n)=>{await (a=m.createElement("script"));e.set("libraries",[...r]+"");for(k in g)e.set(k.replace(/[A-Z]/g,t=>"_"+t[0].toLowerCase()),g[k]);e.set("callback",c+".maps."+q);a.src=`https://maps.${c}apis.com/maps/api/js?`+e;d[q]=f;a.onerror=()=>h=n(Error(p+" could not load."));a.nonce=m.querySelector("script[nonce]")?.nonce||"";m.head.append(a)}));d[l]?console.warn(p+" only loads once. Ignoring:",g):d[l]=(f,...n)=>r.add(f)&&u().then(()=>d[l](f,...n))})
            ({key: "AIzaSyBXn0fuQG1tjxhm8rXwgLMAQloDYXy2nvE", v: "weekly"});
        </script>
    </body>
</html>

<?php
    }
?>