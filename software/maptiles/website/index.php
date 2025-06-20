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
      .map-control{display:flex;align-items:center;position:absolute;margin:10px}
      .pop-window{display:flex;flex-direction:column;background-color:#D7D7D3;border-radius:10px;position:fixed;top:50%;left:50%;transform:translate(-50%,-50%)}
      .title-bar{display:flex;align-items:center;justify-content:center;background-color:darkblue;height:40px;border-top-left-radius:10px;border-top-right-radius:10px}
      .footer-bar{display:flex;align-items:center;justify-content:flex-end;margin:10px;border-bottom-left-radius:10px;border-bottom-right-radius:10px}
      .content-row{display:flex;align-items:center;margin:10px}
      .input,.button{height:40px;padding:0 10px;box-sizing:border-box}
      .input,.button,.label{font-size:large;font-family:'Roboto',Arial,sans-serif;}
      .title{font-size:large;color:white;font-family:'Roboto',Arial,sans-serif;}
      .button{width:96px}
    </style>
    <script type="module" src="./maptiles.js"></script>
  </head>

  <body>
    <div id="map" width=200 height=200></div>

    <div class="map-control" id="geocoder_control">
      <input class="input" id="geocoder_address_input" placeholder="搜索一个位置..." aria-label="Search input">
      <button class="button" id="geocoder_search_button" aria-label="Search button"><i class="fa-solid fa-magnifying-glass"></i>&nbsp;搜索</button>
    </div>

    <div class="map-control" id="maptiles_control">
      <button class="button" id="maptiles_mark_button" aria-label="Mark button"><i class="fa-solid fa-vector-square"></i>&nbsp;标记</button>
      <button class="button" id="maptiles_clear_button" aria-label="Clear button"><i class="fa fa-eraser"></i>&nbsp;清除</button>
      <button class="button" id="maptiles_download_button" aria-label="Download button"><i class="fa fa-download"></i>&nbsp;下载</button>
    </div>

    <div class="pop-window" id="download_window">
      <form id="download" method="post">
        <div class="title-bar">
            <span class="title">下载地图瓦片</span>
        </div>
        <div class="content-row">
            <span class="label">瓦片级别从</span>
            <input class="input" id="download_min_zoom" type="number" value="12" min="1" max="20" aria-label="Minimum zoom level">
            <span class="label">至</span>
            <input class="input" id="download_max_zoom" type="number" value="16" min="1" max="20" aria-label="Maximum zoom level">
        </div>
        <div class="content-row">
          <span class="label">瓦片服务器URL</span>
          <input class="input" type="text" name="url" placeholder="https://example.com/{z}/{x}/{y}.png?key=AIzaSyBXn0fuQG1tjxhm8rXwgLMAQloDYXy2nvE" required>
        </div>
        <div class="footer-bar">
            <button class="button" type="reset">取消</button>
          <button class="button" type="submit">确定</button>
        </div>
        </form>
    </div>

    <script>(g=>{var h,a,k,p="The Google Maps JavaScript API",c="google",l="importLibrary",q="__ib__",m=document,b=window;b=b[c]||(b[c]={});var d=b.maps||(b.maps={}),r=new Set,e=new URLSearchParams,u=()=>h||(h=new Promise(async(f,n)=>{await (a=m.createElement("script"));e.set("libraries",[...r]+"");for(k in g)e.set(k.replace(/[A-Z]/g,t=>"_"+t[0].toLowerCase()),g[k]);e.set("callback",c+".maps."+q);a.src=`https://maps.${c}apis.com/maps/api/js?`+e;d[q]=f;a.onerror=()=>h=n(Error(p+" could not load."));a.nonce=m.querySelector("script[nonce]")?.nonce||"";m.head.append(a)}));d[l]?console.warn(p+" only loads once. Ignoring:",g):d[l]=(f,...n)=>r.add(f)&&u().then(()=>d[l](f,...n))})
        ({key: "AIzaSyBXn0fuQG1tjxhm8rXwgLMAQloDYXy2nvE", v: "weekly"});</script>
  </body>

</html>
