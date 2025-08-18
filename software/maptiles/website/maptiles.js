// Initialize the map and geocoder
let map;
let geocoder;

// Calculate SHA-256 hash with Web Crypto API
async function sha256(str) {
  const encoder = new TextEncoder();
  const data = encoder.encode(str);
  
  const hashBuffer = await crypto.subtle.digest('SHA-256', data);
  
  return Array.from(new Uint8Array(hashBuffer))
    .map(byte => byte.toString(16).padStart(2, '0'))
    .join('').toLowerCase();
}

// RSA-OAEP encrypt with Web Crypto API
async function rsaOaepEncrypt(plaintext) {
    if (public_key) {
        const encoder = new TextEncoder();
        const data = encoder.encode(plaintext);
        const encrypted = await window.crypto.subtle.encrypt(
            { name: 'RSA-OAEP', hash: { name: 'SHA-256' } },
            public_key,
            data
        );
        return btoa(String.fromCharCode(...new Uint8Array(encrypted)));
    } else {
        throw {
            message: "public_key not set",
        };
    }
}

// Get the user's current location or use a default location
//@ts-ignore
async function getHomeLocation() {
    return new Promise((resolve) => {
        if (navigator.geolocation && window.isSecureContext) {
            navigator.geolocation.getCurrentPosition(
                (position) => {
                    resolve({
                        lat: position.coords.latitude,
                        lng: position.coords.longitude,
                    });
                },
                (error) => {
                    console.error(error);
                    resolve({ lat: 22.832500, lng: 114.700556 });
                }
            );
        } else {
            resolve({ lat: 22.832500, lng: 114.700556 });
        }
    });
}

/**
 * Converts geographic coordinates to tile indices at a specific zoom level
 * @param {number} lng Longitude (-180 to 180)
 * @param {number} lat Latitude (-85.05112878 to 85.05112878)
 * @param {number} zoom Zoom level (0 to 30)
 * @returns {number[]} Tile indices [x, y]
 */
function coodinateToTileIndex(lng, lat, zoom) {
    // Validate input parameters
    if (lng < -180 || lng > 180 || lat < -85.05112878 || lat > 85.05112878 || 
        zoom < 0 || zoom > 30) {
        return [NaN, NaN];  // Invalid parameters
    }
    
    const n = Math.pow(2, zoom);  // Number of tiles along each axis
    const epsilon = 1e-10;        // Floating-point precision tolerance
    
    // Calculate X index (longitude component)
    let x = ((lng + 180) / 360) * n;
    if (x >= n) x = n - epsilon;  // Handle 180° longitude edge case
    const xIndex = Math.min(Math.floor(x), n - 1);
    
    // Calculate Y index (latitude component using Mercator projection)
    const latRad = lat * Math.PI / 180;  // Convert to radians
    // Mercator projection formula
    const y = n * (0.5 - Math.log(Math.tan(latRad) + 1 / Math.cos(latRad)) / (2 * Math.PI));
    let yIndex;
    if (y < 0) {
        yIndex = 0;                // North pole
    } else if (y >= n) {
        yIndex = n - 1;             // South pole
    } else {
        yIndex = Math.min(Math.floor(y), n - 1);
    }
    
    return [xIndex, yIndex];
}

/**
 * Calculates the total number of map tiles covering a geographic area across zoom levels
 * @param {number} left Left boundary longitude (-180 to 180)
 * @param {number} top Top boundary latitude (-85.05112878 to 85.05112878)
 * @param {number} right Right boundary longitude (-180 to 180)
 * @param {number} bottom Bottom boundary latitude (-85.05112878 to 85.05112878)
 * @param {number} zoom_min Minimum zoom level (0 to 30)
 * @param {number} zoom_max Maximum zoom level (0 to 30)
 * @returns {number} Total tile count (negative values indicate errors)
 */
function calculateTileCount(left, top, right, bottom, zoom_min, zoom_max) {
    // Validate longitude parameters
    if (left < -180 || left > 180 || right < -180 || right > 180) {
        return -1;  // Longitude out of range
    }
    // Validate latitude parameters
    if (top < -85.05112878 || top > 85.05112878 || 
        bottom < -85.05112878 || bottom > 85.05112878) {
        return -2;  // Latitude out of range
    }
    // Validate zoom range
    if (zoom_min < 0 || zoom_max < 0 || zoom_min > 25 || zoom_max > 25 || zoom_min > zoom_max) {
        return -3;  // Invalid zoom range
    }
    // Validate latitude order
    if (top < bottom) {
        return -4;  // Top latitude must be greater than bottom latitude
    }
    
    // Handle International Date Line crossing (recursive split)
    if (left > right) {
        // Split into two regions: [left, 180] and [-180, right]
        const part1 = calculateTileCount(left, top, 180, bottom, zoom_min, zoom_max);
        const part2 = calculateTileCount(-180, top, right, bottom, zoom_min, zoom_max);
        
        // Propagate errors from recursive calls
        if (part1 < 0) return part1;
        if (part2 < 0) return part2;
        return part1 + part2;
    }
    
    let totalTiles = 0;  // Accumulator for total tile count
    
    // Process each zoom level in the specified range
    for (let z = zoom_min; z <= zoom_max; z++) {
        const n = Math.pow(2, z);  // Tiles per dimension at current zoom
        
        // Calculate tile indices for area corners
        const [x1, y1] = coodinateToTileIndex(left, top, z);      // Top-left corner
        const [x2, y2] = coodinateToTileIndex(right, bottom, z);  // Bottom-right corner
        
        // Check for calculation errors
        if (isNaN(x1) || isNaN(y1) || isNaN(x2) || isNaN(y2)) {
            return -5;  // Tile index calculation error
        }
        
        // Determine tile range boundaries with safe ordering
        const xStart = Math.min(x1, x2);
        const xEnd = Math.max(x1, x2);
        const yStart = Math.min(y1, y2);
        const yEnd = Math.max(y1, y2);
        
        // Clamp values to valid tile index range [0, n-1]
        const clampedXStart = Math.max(0, Math.min(xStart, n - 1));
        const clampedXEnd = Math.max(0, Math.min(xEnd, n - 1));
        const clampedYStart = Math.max(0, Math.min(yStart, n - 1));
        const clampedYEnd = Math.max(0, Math.min(yEnd, n - 1));
        
        // Calculate tile counts in both dimensions
        const xCount = Math.max(0, clampedXEnd - clampedXStart + 1);
        const yCount = Math.max(0, clampedYEnd - clampedYStart + 1);
        
        // Add tiles for current zoom level to total
        totalTiles += xCount * yCount;
    }
    
    return totalTiles;
}

var current_window = null;

function hideWindow(window_id) {
    const window = document.getElementById(window_id);
    window.dispatchEvent(new Event("hide"));
    current_window = null;
    window.style.visibility = "hidden";
}

function showWindow(window_id) {
    if (current_window) {
        hideWindow(current_window);
    }
    const window = document.getElementById(window_id);
    window.style.visibility = "visible";
    current_window = window_id;
    window.dispatchEvent(new Event("show"));
}

// Initialize the map
async function initMap() {
    // The home location
    let home_loc = await getHomeLocation();
    // Request needed libraries.
    //@ts-ignore
    const { Map } = await google.maps.importLibrary("maps");
    const { Geocoder } = await google.maps.importLibrary("geocoding");
    // Create the map
    map = new Map(document.getElementById("map"), {
        mapId: "9d77b79e6c9e13c9977ae6cd",
        zoom: 12,
        center: home_loc,
        mapTypeId: "terrain",
        mapTypeControl: true,
        mapTypeControlOptions: {
            position: google.maps.ControlPosition.TOP_RIGHT,
        },
        scaleControl: true,
        fullscreenControl: false,
        streetViewControl: false,
    });

    // Create the geocoder
    geocoder = new Geocoder();
    // Add the geocoder control to the map
    map.controls[google.maps.ControlPosition.LEFT_TOP].push(document.getElementById("geocoder_control"));
    // Add event listeners to the geocoder control address input
    document.getElementById("geocoder_address_input").addEventListener("keydown", (event) => {
        if (event.key === "Enter") {
            event.preventDefault();
            document.getElementById("geocoder_search_button").click();
        }
    });
    // Add event listeners to the geocoder control search button
    document.getElementById("geocoder_search_button").addEventListener("click", () => {
        let address = document.getElementById("geocoder_address_input").value;
        if (address) {
            geocoder.geocode({ address }, (results, status) => {
                if (status === "OK") {
                    map.setCenter(results[0].geometry.location);
                } else {
                    alert("Can't find the address: " + address);
                }
            });
        }
    });

    // Task class definition
    class MapTiles extends google.maps.OverlayView {
        constructor(map) {
            super();
            this.canvas = document.getElementById("maptiles_canvas");

            this.rectangle = new google.maps.Rectangle({
                map: map,
                strokeColor: "#0000FF",
                strokeOpacity: 0.75,
                strokeWeight: 2,
                fillColor: "#0000FF",
                fillOpacity: 0.15,
                clickable: false,
                editable: false,
                draggable: false,
            });

            this.minZoom = 0;
            this.maxZoom = 0;

            this.taskId = null;
            this.taskName = null;
            this.taskPrograss = 0;

            this.setMap(map);
        }

        onAdd() {
            const panes = this.getPanes();
            panes.overlayLayer.appendChild(this.canvas);
        }

        draw() {
            const bounds = this.rectangle.getBounds();
            console.log("bounds: " + bounds);
            if (bounds === undefined || bounds.isEmpty()) {
                this.canvas.style.visibility = 'hidden';
            } else {
                this.canvas.style.visibility = 'visible';
                const projection = this.getProjection();
                const sw = projection.fromLatLngToDivPixel(bounds.getSouthWest());
                const ne = projection.fromLatLngToDivPixel(bounds.getNorthEast());
                this.canvas.style.left = sw.x + 'px';
                this.canvas.style.top = ne.y + 'px';
                this.canvas.style.width = (ne.x - sw.x) + 'px';
                this.canvas.style.height = (sw.y - ne.y) + 'px';
            }
        }

        onRemove() {
            this.canvas.parentNode.removeChild(this.canvas);
            this.canvas.style.visibility = 'hidden';
            this.rectangle.setMap(null);
        }

        getBounds() {
            return this.rectangle.getBounds();
        }

        setBounds(bounds) {
            this.rectangle.setBounds(bounds);
        }

        enableAdjusting(enabled) {
            this.rectangle.setOptions({clickable: enabled, editable: enabled, draggable: enabled});
        }

        reset() {
            this.setBounds(new google.maps.LatLngBounds());
            this.enableAdjusting(false);
            this.taskId = null;
            this.taskName = null;
            this.taskPrograss = 0;
        }

        setTask(task_id, task_name, min_zoom, max_zoom) {
            this.enableAdjusting(false);
            this.taskId = task_id;
            this.taskName = task_name;
            this.minZoom = min_zoom;
            this.maxZoom = max_zoom;
        }
    }

    // Define the map tiles overlay
    let mapTiles = new MapTiles(map);
    let isMarking = false;
    let startPoint = null;
    let endPoint = null;

    // Add map tiles control to the map
    map.controls[google.maps.ControlPosition.TOP_CENTER].push(document.getElementById("maptiles_control"));

    document.getElementById("maptiles_mark_button").addEventListener("click", () => {
        map.setOptions({gestureHandling: "none"});
        mapTiles.reset();
        startPoint = null;
        endPoint = null;
        isMarking = true;
    });

    document.getElementById("maptiles_clear_button").addEventListener("click", () => {
        map.setOptions({gestureHandling: "auto"});
        mapTiles.reset();
        startPoint = null;
        endPoint = null;
        isMarking = false;
    });

    map.addListener('mousedown', function(event) {
        if (isMarking) {
            startPoint = event.latLng;
        }
    });

    map.addListener('mousemove', function(event) {
        if (isMarking) {
            if (startPoint != null) {
                endPoint = event.latLng;
                if (endPoint != null) {
                    const south = Math.min(startPoint.lat(), endPoint.lat());
                    const north = Math.max(startPoint.lat(), endPoint.lat());
                    const west = Math.min(startPoint.lng(), endPoint.lng());
                    const east = Math.max(startPoint.lng(), endPoint.lng());
                    let longitude_diff = east - west;
                    let south_west = null;
                    let north_east = null;
                    if (longitude_diff < 0 && longitude_diff > -180 || longitude_diff > 180) {
                        south_west = new google.maps.LatLng(south, east);
                        north_east = new google.maps.LatLng(north, west);
                    } else {
                        south_west = new google.maps.LatLng(south, west);
                        north_east = new google.maps.LatLng(north, east);
                    }
                    mapTiles.setBounds(new google.maps.LatLngBounds(south_west, north_east));
                }
            }
        }
    });

    document.addEventListener('pointerup', function(event) {
        if (isMarking) {
            isMarking = false;
            map.setOptions({ gestureHandling: "auto"});
            if (mapTiles != null) {
                mapTiles.enableAdjusting(true);
            }
        }
    });

    document.getElementById("maptiles_account_button").addEventListener("click", () => {
        if (user_id == null) {
            showWindow("login_window");
        } else {
            showWindow("profile_window");
        }
    });

    document.getElementById("maptiles_download_button").addEventListener("click", () => {
        if (user_id == null) {
            alert("因为下载任务是异步的，并且耗时较长，所以必须先登录，才能启动并保存下载任务。");
            showWindow("login_window");
            return;
        } else {
            if (mapTiles == null || mapTiles.getBounds().isEmpty()) {
                alert("请先标记地图范围。");
            } else {
                showWindow("download_window");
            }
        }
    });

    document.getElementById("maptiles_task_button").addEventListener("click", () => {
        if (user_id == null) {
            alert("必须先登录才能查看下载任务列表。");
            showWindow("login_window");
            return;
        } else {
            showWindow("task_window");
        }
    });

    function updateTileCount() {
        const bounds = mapTiles.getBounds();
        const west = bounds.getSouthWest().lng();
        const east = bounds.getNorthEast().lng();
        const north = bounds.getNorthEast().lat();
        const south = bounds.getSouthWest().lat();
        const zoom_min = document.getElementById("download_form_zoom_min").value;
        const zoom_max = document.getElementById("download_form_zoom_max").value;
        const tile_count = calculateTileCount(west, north, east, south, parseInt(zoom_min), parseInt(zoom_max));

        let seconds = Math.ceil(tile_count / 10);
        let minuts = Math.floor(seconds / 60); let hours = Math.floor(minuts / 60); let days = Math.floor(hours / 24);
        seconds = seconds % 60; minuts = minuts % 60; hours = hours % 24;
        const elapsed = (days > 0 ? days + "天" : "") + (hours > 0 ? hours + "小时" : "") + (minuts > 0 ? minuts + "分钟" : "") + (seconds > 0 ? seconds + "秒" : "");

        const tile_count_element = document.getElementById("download_form_tile_count");
        if (tile_count > 0) {
            tile_count_element.innerHTML = "需下载 " + tile_count + " 张瓦片，预计耗时:" + elapsed;
        } else {
            tile_count_element.innerHTML = "计算出错，请检查地图范围和缩放级别。";
        }
    }

    document.getElementById("download_window").addEventListener("show", () => {
        const cancelButton = document.getElementById("download_form_cancel");
        const submitButton = document.getElementById("download_form_submit");
        cancelButton.disabled = false;
        submitButton.disabled = false;
        submitButton.innerHTML = '<i class="fa-solid fa-check"></i>&nbsp;提交';

        const bounds = mapTiles.getBounds();
        const west = bounds.getSouthWest().lng();
        const north = bounds.getNorthEast().lat();
        const east = bounds.getNorthEast().lng();
        const south = bounds.getSouthWest().lat();

        document.getElementById("download_form_west").value = west;
        document.getElementById("download_form_east").value = east;
        document.getElementById("download_form_north").value = north;
        document.getElementById("download_form_south").value = south;
        document.getElementById("download_form_bounds").innerHTML = "[" + west.toFixed(6) + ", " + south.toFixed(6) + ", " + east.toFixed(6) + ", " + north.toFixed(6) + "]";
        updateTileCount();
    });

    document.getElementById("download_form_zoom_min").addEventListener("input", () => {
        updateTileCount();
    });

    document.getElementById("download_form_zoom_max").addEventListener("input", () => {
        updateTileCount();
    });

    document.getElementById("download_form").addEventListener("submit", async function(event) {
        event.preventDefault();
        const cancelButton = document.getElementById("download_form_cancel");
        const submitButton = document.getElementById("download_form_submit");
        cancelButton.disabled = true;
        submitButton.disabled = true;
        submitButton.innerHTML = '<i class="fa-solid fa-hourglass-half"></i>&nbsp;提交中...';

        const form_data = new FormData(this);
        form_data.append("user_id", user_id);

        try {
            const response = await fetch("", {method: "POST", body: form_data});
            if (response.status == 200) {
                const result = await response.json();
                if (result.code === 0) {
                    alert("下载任务已提交，任务ID：" + result.data.task_id + "。");
                    hideWindow("download_window");
                } else {
                    alert("下载任务提交失败：" + result.message);
                }
            } else {
                alert("下载任务提交失败：" + response.status + " " + response.statusText);
            }
        } catch (error) {
            alert("下载任务提交失败：" + error.message);
        }

        cancelButton.disabled = false;
        submitButton.disabled = false;
        submitButton.innerHTML = '<i class="fa-solid fa-check"></i>&nbsp;提交';
    });

    document.getElementById("download_form").addEventListener("reset", (event) => {
        event.preventDefault();
        hideWindow("download_window");
    });

    document.getElementById("login_window").addEventListener("show", () => {
        const cancelButton = document.getElementById("login_form_cancel");
        const submitButton = document.getElementById("login_form_submit");
        cancelButton.disabled = false;
        submitButton.disabled = false;
        submitButton.innerHTML = '<i class="fa-solid fa-check"></i>&nbsp;登录';
    });

    document.getElementById("login_form").addEventListener("submit", async function(event) {
        event.preventDefault();
        const cancelButton = document.getElementById("login_form_cancel");
        const submitButton = document.getElementById("login_form_submit");
        cancelButton.disabled = true;
        submitButton.disabled = true;
        submitButton.innerHTML = '<i class="fa-solid fa-hourglass-half"></i>&nbsp;登录中...'

        const fomr_data = new FormData(this);
        const email = fomr_data.get("email").trim().toLowerCase();
        const password =fomr_data.get("password").trim();
        const password_pseudo = await sha256(email + ":" + password + "@snailtrail.org");
        const password_encrypt = await rsaOaepEncrypt(password_pseudo);
        fomr_data.set("email", email);
        fomr_data.set("password", password_encrypt);
        fomr_data.delete("password_confirm");

        try {
            const response = await fetch("", {method: "POST", body: fomr_data});
            if (response.status == 200) {
                const result = await response.json();
                if (result.code === 0) {
                    user_id = result.data.user_id;
                    hideWindow("login_window");
                    document.getElementById("maptiles_account_button").innerHTML = '<i class="fa-solid fa-user"></i>&nbsp;详情';
                    alert("用户" + result.data.email + "登录成功，上一次登录时间：" + result.data.last_login_time + "。");
                } else {
                    alert("登录失败：" + result.message);
                }
            } else {
                alert("登录失败：" + response.status + " " + response.statusText);
            }
        } catch (error) {
            alert("登录失败：" + error.message);
        }

        cancelButton.disabled = false;
        submitButton.disabled = false;
        submitButton.innerHTML = '<i class="fa-solid fa-check"></i>&nbsp;登录';
    });

    document.getElementById("login_form").addEventListener("reset", (event) => {
        event.preventDefault();
        hideWindow("login_window");
    });

    document.getElementById("register_link").addEventListener("click", () => {
        hideWindow("login_window");
        showWindow("register_window");
    });

    document.getElementById("reset_password_link").addEventListener("click", () => {
        hideWindow("login_window");
        showWindow("reset_password_window");
    });

    document.getElementById("register_form").addEventListener("submit", async function(event) {
        event.preventDefault();
        const cancelButton = document.getElementById("register_form_cancel");
        const submitButton = document.getElementById("register_form_submit");
        cancelButton.disabled = true;
        submitButton.disabled = true;
        submitButton.innerHTML = '<i class="fa-solid fa-hourglass-half"></i>&nbsp;注册中...'

        const fomr_data = new FormData(this);
        const email = fomr_data.get("email").trim().toLowerCase();
        const password =fomr_data.get("password").trim();
        const password_confirm =fomr_data.get("password_confirm").trim();

        if (password === password_confirm) {
            try {
                const password_pseudo = await sha256(email + ":" + password + "@snailtrail.org");
                const password_encrypt = await rsaOaepEncrypt(password_pseudo);
                fomr_data.set("email", email);
                fomr_data.set("password", password_encrypt);
                fomr_data.delete("password_confirm");

                const response = await fetch("/", { method: "POST", body: fomr_data});
                if (response.status == 200) {
                    const result = await response.json();
                    if (result.code === 0) {
                        alert("注册成功，请登录...");
                        hideWindow("register_window");
                        showWindow("login_window");
                    } else {
                        alert("注册失败：" + result.message);
                    }
                } else {
                    alert("注册失败：" + response.status + " " + response.statusText);
                }
            } catch (error) {
                alert("注册失败：" + error.message);
            }
        } else {
            alert("两次输入的密码不一致，请重新输入！");
        }

        cancelButton.disabled = false;
        submitButton.disabled = false;
        submitButton.innerHTML = '<i class="fa-solid fa-check"></i>&nbsp;注册';
    });

    document.getElementById("register_form").addEventListener("reset", (event) => {
        event.preventDefault();
        hideWindow("register_window");
    });

    document.getElementById("reset_password_form").addEventListener("submit", async function(event) {
        event.preventDefault();
        const cancelButton = document.getElementById("reset_password_form_cancel");
        const submitButton = document.getElementById("reset_password_form_submit");
        cancelButton.disabled = true;
        submitButton.disabled = true;
        submitButton.innerHTML = '<i class="fa-solid fa-hourglass-half"></i>&nbsp;重置中...'
    });

    document.getElementById("reset_password_form").addEventListener("reset", (event) => {
        event.preventDefault();
        hideWindow("reset_password_window");
    });

    document.getElementById("profile_window").addEventListener("show", async function() {
        if (user_id === null) {
            hideWindow("profile_window");
            alert("用户未登录，请先登录");
            showWindow("login_window");
        } else {
            const form_data = new FormData();
            form_data.set("action", "get_profile");
            form_data.set("user_id", user_id);
            try {
                const response = await fetch("/", { method: "POST", body: form_data});
                if (response.status == 200) {
                    const result = await response.json();
                    if (result.code === 0) {
                        document.getElementById("profile_email").innerHTML = result.data.email;
                        document.getElementById("profile_register_time").innerHTML = result.data.register_time;
                        document.getElementById("profile_running_task_count").innerHTML = result.data.running_task_count;
                    } else {
                        alert("获取用户信息失败：" + result.message);
                    }
                } else {
                    alert("获取用户信息失败：" + response.status + " " + response.statusText);
                }
            } catch (error) {
                alert("获取用户信息失败：" + error.message);
            }
        }
    });

    document.getElementById("profile_task_link").addEventListener("click", () => {
        showWindow("task_window");
    });

    document.getElementById("logout_form").addEventListener("submit", async function(event) {
        event.preventDefault();
        const cancelButton = document.getElementById("logout_form_cancel");
        const submitButton = document.getElementById("logout_form_submit");
        cancelButton.disabled = true;
        submitButton.disabled = true;
        submitButton.innerHTML = '<i class="fa-solid fa-hourglass-half"></i>&nbsp;登出中...'

        const form_data = new FormData(this);
        form_data.set("user_id", user_id);

        try {
            const response = await fetch("/", { method: "POST", body: form_data});
            if (response.status == 200) {
                const result = await response.json();
                if (result.code === 0) {
                    user_id = null;
                    document.getElementById("maptiles_account_button").innerHTML = '<i class="fa-solid fa-user"></i>&nbsp;登录';
                    hideWindow("profile_window");
                    document.getElementById("profile_email").innerHTML = "";
                    document.getElementById("profile_register_time").innerHTML = "";
                    document.getElementById("profile_running_task_count").innerHTML = "";
                    alert("登出成功，欢迎下次登录！");
                } else {
                    alert("登出失败：" + result.message);
                }
            } else {
                alert("登出失败：" + response.status + " " + response.statusText);
            }
        } catch (error) {
            alert("登出失败：" + error.message);
        }

        cancelButton.disabled = false;
        submitButton.disabled = false;
        submitButton.innerHTML = '<i class="fa-solid fa-right-from-bracket"></i>&nbsp;登出';
    });

    document.getElementById("logout_form").addEventListener("reset", (event) => {
        event.preventDefault();
        hideWindow("profile_window");
    });
}

initMap();
