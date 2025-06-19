<?php
    // 数据库连接配置
    $dbConfig = ['host' => 'localhost', 'user' => 'maptiles', 'pass' => '4pEm(YFWXZi5!1NO', 'name' => 'maptiles'];
    // 创建数据库连接
    $conn = new mysqli($dbConfig['host'], $dbConfig['user'], $dbConfig['pass'], $dbConfig['name']);
    // 检查连接
    if ($conn->connect_error) {
        die("数据库连接失败: {$conn->connect_error}");
    }
    // 选择数据库
    $conn->select_db($dbConfig['name']);
?>

<!DOCTYPE html>
<html>

  <head>
    <title>地图瓦片标记和下载</title>
    <meta name="robots" content="noindex, nofollow">
    <script src="https://kit.fontawesome.com/d1f7300f56.js" crossorigin="anonymous"></script>
    <style>
      html,body{height:100%;margin:0;padding:0;}
      #map {height: 100%;}
      .custom_control{display:flex;position:absolute;margin:10px;align-items: center}
      .pop_window{display:flex;flex-direction:column;background-color:white;padding:20px;position:fixed;top:50%;left:50%;transform:translate(-50%,-50%);}
      .custom_text_input,.custom_number_input,.custom_button{height:40px;padding:0 10px;box-sizing:border-box}
      .custom_text_input,.custom_number_input,.custom_button,.custom_label{margin:0 4px;font-size:large}
      .custom_button{width:96px}
      .custom_number_input{width:64px}

        .form-row {
            display: flex;
            align-items: center;
            margin-bottom: 20px;
        }
        
        .form-label {
            flex: 0 0 120px;
            text-align: right;
            padding-right: 20px;
            font-weight: 500;
            color: #34495e;
        }
        
        .input-group {
            display: flex;
            align-items: center;
            flex: 1;
        }
        
        .input-group input {
            flex: 1;
        }
        
        .input-group span {
            padding: 0 10px;
            color: #7f8c8d;
        }
        
        input[type="number"],
        input[type="url"],
        input[type="text"] {
            width: 100%;
            padding: 12px 15px;
            border: 1px solid #ddd;
            border-radius: 6px;
            font-size: 16px;
            transition: all 0.2s ease;
        }
        
        input[type="number"]:focus,
        input[type="url"]:focus,
        input[type="text"]:focus {
            border-color: #3498db;
            outline: none;
            box-shadow: 0 0 0 3px rgba(52, 152, 219, 0.1);
        }
        
        .button-group {
            display: flex;
            justify-content: center;
            gap: 15px;
            margin-top: 30px;
        }
        
        .btn {
            padding: 12px 30px;
            border: none;
            border-radius: 6px;
            font-size: 16px;
            font-weight: 500;
            cursor: pointer;
            transition: all 0.2s ease;
        }
        
        .btn-cancel {
            background-color: #ecf0f1;
            color: #7f8c8d;
        }
        
        .btn-cancel:hover {
            background-color: #e0e7ea;
        }
        
        .btn-submit {
            background-color: #3498db;
            color: white;
        }
        
        .btn-submit:hover {
            background-color: #2980b9;
        }
        
        .form-note {
            margin-top: 25px;
            padding: 15px;
            background-color: #f8f9fa;
            border-radius: 6px;
            border-left: 4px solid #3498db;
            font-size: 14px;
            color: #7f8c8d;
        }
        
        .form-note ul {
            padding-left: 20px;
            margin-top: 8px;
        }
        
        .form-note li {
            margin-bottom: 5px;
        }
        
        @media (max-width: 600px) {
            .form-row {
                flex-direction: column;
                align-items: flex-start;
            }
            
            .form-label {
                flex: 0 0 auto;
                text-align: left;
                width: 100%;
                padding-right: 0;
                margin-bottom: 8px;
            }
            
            .input-group {
                width: 100%;
            }
            
            .button-group {
                flex-direction: column;
            }
            
            .btn {
                width: 100%;
            }
        }
    </style>
    <script type="module" src="./maptiles.js"></script>
  </head>

  <body>
    <div id="map" width=200 height=200></div>

    <div class="custom_control" id="geocoder_control">
      <input class="custom_text_input" id="geocoder_address_input" placeholder="搜索一个位置..." aria-label="Search input">
      <button class="custom_button" id="geocoder_search_button" aria-label="Search button"><i class="fa-solid fa-magnifying-glass"></i>&nbsp;搜索</button>
    </div>

    <div class="custom_control" id="maptiles_control">
      <button class="custom_button" id="maptiles_mark_button" aria-label="Mark button"><i class="fa-solid fa-vector-square"></i>&nbsp;标记</button>
      <button class="custom_button" id="maptiles_clear_button" aria-label="Clear button"><i class="fa fa-eraser"></i>&nbsp;清除</button>
      <span class="custom_label">级别从</span>
      <input class="custom_number_input" id="maptiles_min_zoom_input" type="number" value="12" min="1" max="20" aria-label="Minimum zoom input">
      <span class="custom_label">至</span>
      <input class="custom_number_input" id="maptiles_max_zoom_input" type="number" value="16" min="1" max="20" aria-label="Maximum zoom input">
      <button class="custom_button" id="maptiles_download_button" aria-label="Download button"><i class="fa fa-download"></i>&nbsp;下载</button>
    </div>

    <div class="custom_form_control" id="download_control">
      <span class="custom_label">下载地图瓦片</span>
      <form id="download" method="post">
        <div class="form-row">
          <label class="form-label">级别范围</label>
          <div class="input-group">
            <input type="number" id="levelFrom" name="level_from" min="1" max="10" value="1" required>
            <span>至</span>
            <input type="number" id="levelTo" name="level_to" min="1" max="10" value="5" required>
          </div>
        </div>
        
        <div class="form-row">
          <label class="form-label" for="articleUrl">文章URL</label>
          <input type="url" id="articleUrl" name="article_url" placeholder="https://example.com/article" required>
        </div>

        <div class="button-group">
          <button type="reset" class="btn btn-cancel">取消</button>
          <button type="submit" class="btn btn-submit">确定</button>
        </div>
        </form>
    </div>

    <script>(g=>{var h,a,k,p="The Google Maps JavaScript API",c="google",l="importLibrary",q="__ib__",m=document,b=window;b=b[c]||(b[c]={});var d=b.maps||(b.maps={}),r=new Set,e=new URLSearchParams,u=()=>h||(h=new Promise(async(f,n)=>{await (a=m.createElement("script"));e.set("libraries",[...r]+"");for(k in g)e.set(k.replace(/[A-Z]/g,t=>"_"+t[0].toLowerCase()),g[k]);e.set("callback",c+".maps."+q);a.src=`https://maps.${c}apis.com/maps/api/js?`+e;d[q]=f;a.onerror=()=>h=n(Error(p+" could not load."));a.nonce=m.querySelector("script[nonce]")?.nonce||"";m.head.append(a)}));d[l]?console.warn(p+" only loads once. Ignoring:",g):d[l]=(f,...n)=>r.add(f)&&u().then(()=>d[l](f,...n))})
        ({key: "AIzaSyBXn0fuQG1tjxhm8rXwgLMAQloDYXy2nvE", v: "weekly"});</script>
  </body>

</html>
