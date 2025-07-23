// Initialize global variables
var user_id = null;
var public_key = null;

// Initialize the map and geocoder
let map;
let geocoder;

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

var current_window = null;

function hideWindow(window_id) {
    const window = document.getElementById(window_id);
    window.style.visibility = "hidden";
    window.dispatchEvent(new Event("hide"));
    current_window = null;
}

function showWindow(window_id) {
    if (current_window) {
        hideWindow(current_window);
    }
    const window = document.getElementById(window_id);
    window.style.visibility = "visible";
    window.dispatchEvent(new Event("show"));
    current_window = window_id;
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
            alert("profile_window还没有实现")
            //showWindow("profile_window");
        }
    });

    document.getElementById("maptiles_download_button").addEventListener("click", () => {
        if (user_id == null) {
            alert("因为下载任务是异步的，并且耗时较长，所以必须先登录，才能启动并保存下载任务。");
            showWindow("login_window");
            return;
        } else {
            showWindow("download_window");
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

    document.getElementById("login_window").addEventListener("show", () => {
        const loginForm = document.getElementById("login_form");
        loginForm.reset();
        loginForm.reset.disabled = false;
        loginForm.submit.disabled = false;
        loginForm.submit.value = '<i class="fa-solid fa-check"></i>&nbsp;登录';
    });

    document.getElementById("login_form").addEventListener("submit", async function(event) {
        event.preventDefault();
        this.reset.disabled = true;
        this.submit.disabled = true;
        this.submit.value = '<i class="fa-solid fa-hourglass-half"></i>&nbsp;登录中...'

        const data = new FormData(this);
    
        try {
            const response = await fetch("", {method: "POST", body: data});
            if (response.status == 200) {
                user_id = response.data.user_id;
                hideWindow("login_window");
            } else {
                alert("登录失败：" + response.data.message);
            }
        } catch (error) {
            alert("登录失败：" + error.message);
        }

        this.submit.disabled = false;
        this.submit.value = '<i class="fa-solid fa-check"></i>&nbsp;登录';
        this.reset.disabled = false;
    });

    document.getElementById("login_form").addEventListener("reset", (event) => {
        event.preventDefault();
        hideWindow("login_window");
    });
}

initMap();
