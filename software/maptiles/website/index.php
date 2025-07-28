<?php
    function rsa_private_decrypt($password_encrypt_base64) {
        $private_key = file_get_contents('rsakeys/privatekey.pem');
        $password_encrypt = base64_decode($password_encrypt_base64);
        
        if (openssl_private_decrypt($password_encrypt, $decrypted, $private_key, OPENSSL_PKCS1_OAEP_PADDING)) {
            return $decrypted;
        } else {
            return null;
        }
    }

    function update_challenge() {
        $_SESSION['challenge'] = bin2hex(random_bytes(16));
    }

    session_start();

    if (!isset($_SESSION['challenge'])) {
        update_challenge();
    }

    $public_key = preg_replace('/-----(BEGIN|END)\s+(.*?)\s+KEY-----|\s/', '', file_get_contents('rsakeys/publickey.pem'));

    if ($_SERVER['REQUEST_METHOD'] === 'POST') {
        header("Content-Type: application/json");

        if (isset($_POST) && isset($_POST['action'])) {

            if (file_exists(__DIR__ . '/database.php')) {
                require_once __DIR__ . '/database.php';
            } else {
                error_log('数据库连接模块缺失');
                die(json_encode(['code' => __LINE__, 'message' => '数据库连接模块缺失']));
            }

            $conn = Database::getConnection();

            switch ($_POST['action']) {
            case 'register':
                if (!isset($_POST['email']) || !isset($_POST['password'])) {
                    echo json_encode(['code' => __LINE__, 'message' => '请求参数错误']);
                } else {
                    $email = $_POST['email'];
                    $password_encrypt_base64 = $_POST['password'];
                    $password = rsa_private_decrypt($password_encrypt_base64);
                    if ($password === null) {
                        echo json_encode(['code' => __LINE__, 'message' => '密码解密失败']);
                    } else {
                        $stmt = $conn->prepare("SELECT * FROM users WHERE email = ?");
                        $stmt->bind_param("s", $email);
                        $stmt->execute();
                        $result = $stmt->get_result();
                        if ($result->num_rows > 0) {
                            echo json_encode(['code' => __LINE__, 'message' => '邮箱已存在']);
                        } else {
                            $stmt->close();
                            $conn->close();
                            $stmt = $conn->prepare("INSERT INTO users (email, password) VALUES (?, ?)");
                            $stmt->bind_param("ss", $email, $password);
                            $stmt->execute();
                            echo json_encode(['code' => 0, 'message' => '注册成功']);
                        }
                    }
                }
                break;
            }
        } else {
            echo json_encode(['code' => __LINE__, 'message' => '请求参数错误']);
        }
    } else {
?>
        <!DOCTYPE html>
        <html>

            <head>
                <title>地图瓦片标记和下载</title>
                <meta name="robots" content="noindex, nofollow">
                <meta charset="utf-8">
                <script src="https://kit.fontawesome.com/d1f7300f56.js" crossorigin="anonymous"></script>
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
                <script type="module" src="./maptiles.js"></script>
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
                        <input type="hidden" name="download" value="1">
                        <div class="content">
                            <div class="label-grid"><span class="label">下载任务名称：</span></div>
                            <div class="input-grid"><input class="input" style="width:218px" type="text" name="task_name" placeholder="输入名称以区分不同任务..." required></div>
                            <div class="label-grid"><span class="label">地图瓦片级别：</span></div>
                            <div class="input-grid">
                                <span class="label">从</span>
                                <input class="input left-margin" name="min_zoom" type="number" value="12" min="1" max="20" aria-label="Minimum zoom level">
                                <span class="label left-margin">至</span>
                                <input class="input left-margin" name="max_zoom" type="number" value="18" min="1" max="20" aria-label="Maximum zoom level">
                            </div>
                            <div class="label-grid"><span class="label" title="URL中必须包含{x}，{y}和{z}，下载过程中将被分别&#10;替换为地图瓦片的列序号，行序号和缩放级别。">瓦片服务器URL<i class="fa-regular fa-circle-question" style="color:#4285F4;"></i>：</span></div>
                            <div class="input-grid"><input class="input" style="width:500px" type="text" name="url" list="url_list" placeholder="https://tile.server.com/{z}/{x}/{y}.png?key=SERVER_KEY" required></div>
                            <datalist id="url_list">
                                <option value="https://tile.openstreetmap.org/{z}/{x}/{y}.png"></option>
                                <option value="https://tileserver.com/{z}/{x}/{y}.png"></option>
                            </datalist>
                        </div>
                        <div class="footer-bar">
                            <button id="download_form_cancel" class="button" type="reset"><i class="fa-solid fa-xmark"></i>&nbsp;取消</button>
                            <button id="download_form_submit" class="button left-margin" type="submit"><i class="fa-solid fa-check"></i>&nbsp;确定</button>
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
                            <span class="label link left-margin"><i class="fa-solid fa-download"></i>&nbsp;下载</span>
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
                        <input type="hidden" name="login" value="1">
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
                            <div class="input-grid"><input class="input" style="width:300px" type="email" name="email" placeholder="输入E-MAIL地址作为登录名..." required></div>
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
                    <form id="profile_form" method="post">
                        <input type="hidden" name="profile" value="1">
                        <div class="content">
                            <div class="label-grid"><span class="label">E-MAIL地址：</span></div>
                            <div class="input-grid"><span class="label" name="email">someone@example.com</span></div>
                            <div class="label-grid"><span class="label">注册时间</span></div>
                            <div class="input-grid"><span class="label" name="register_time">2025-07-25 08:00:00</span></div>
                            <div class="label-grid"><span class="label">上次登录时间</span></div>
                            <div class="input-grid"><span class="label" name="last_login_time">2025-07-25 08:00:00</span></div>
                        </div>
                        <div class="footer-bar">
                            <button id="profile_form_logout" class="button" type="submit"><i class="fa-solid fa-arrow-right-from-bracket"></i>&nbsp;登出</button>
                            <button id="profile_form_close" class="button left-margin" type="reset"><i class="fa-solid fa-xmark"></i>&nbsp;关闭</button>
                        </div>
                    </form>
                </div>

                <script>
                    <?php if (isset($_SESSION['user_id'])) { ?>
                        user_id = <?php echo $_SESSION['user_id']; ?>;
                    <?php } ?>
                    <?php if ($public_key != '') { ?>
                        window.crypto.subtle.importKey('spki', Uint8Array.from(atob('<?php echo addslashes($public_key); ?>'), c => c.charCodeAt(0)), {name:'RSA-OAEP', hash:'SHA-256'}, true, ['encrypt']).then(key => { public_key = key;}).catch(error => { console.error('Error importing public key:', error); public_key = null;});
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
