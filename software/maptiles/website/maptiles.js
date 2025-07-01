// Status enum
export const Status = Object.freeze({
    UNMARKED: "Unmarked",
    MARKING: "Marking",
    ADJUSTING: "Adjusting",
    PROCESSING: "Processing",
    DONE: "Done",
  });

// Task class definition
export class MapTiles extends google.maps.OverlayView {
    constructor(map) {
        super();
        this.map = map;

        this.status = Status.UNMARKED;
        this.startPoint = null;
        this.endPoint = null;
        this.rectRange = null;
        this.minZoom = 12;
        this.maxZoom = 16;

        this.taskId = null;
        this.taskName = null;
        this.taskPrograss = 0;
    }

    getStatus() {
        return this.status;
    }

    setStatus(status) {
        this.status = status;
    }

    getStartPoint() {
        return this.startPoint;
    }

    setStartPoint(startPoint) {
        this.startPoint = startPoint;
    }

    getEndPoint() {
        return this.endPoint;
    }

    setEndPoint(endPoint) {
        this.endPoint = endPoint;
    }

    getBounds() {
        return new google.maps.LatLngBounds(this.startPoint, this.endPoint);
    }

    setBounds(bounds) {
        this.startPoint = bounds.getSouthWest();
        this.endPoint = bounds.getNorthEast();
    }

    startTask(task_id, task_name, min_zoom, max_zoom) {
        this.setStatus(Status.PROCESSING);
        this.taskId = task_id;
        this.taskName = task_name;
        this.minZoom = min_zoom;
        this.maxZoom = max_zoom;
    }

    setTaskProgress(progress) {
        this.taskPrograss = progress;
    }

    onAdd() {
        const div = document.createElement('div');
        div.style.position = 'absolute';
        // 设置其他样式...
        
        this.div_ = div;
        const panes = this.getPanes();
        panes.overlayLayer.appendChild(div); // 添加到覆盖层
      }

    draw() {
        if (!this.div_) {
            return;
        }
        const projection = this.getProjection();
        const bounds = new google.maps.LatLngBounds(this.startPoint, this.endPoint);
        const sw = projection.fromLatLngToDivPixel(bounds.getSouthWest());
        const ne = projection.fromLatLngToDivPixel(bounds.getNorthEast());
        this.div_.style.left = sw.x + 'px';
        this.div_.style.top = ne.y + 'px';
        this.div_.style.width = (ne.x - sw.x) + 'px';
        this.div_.style.height = (sw.y - ne.y) + 'px';
    }

    onRemove() {
        if (this.div_) {
          this.div_.parentNode.removeChild(this.div_);
          this.div_ = null;
        }
      }
}

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

  // Define manager rectangle drawing state
  let startPoint = null;
  let endPoint = null;
  let rectRange = null;
  let isMarking = false;
  // Add map tiles control to the map
  map.controls[google.maps.ControlPosition.TOP_CENTER].push(document.getElementById("maptiles_control"));

  document.getElementById("maptiles_mark_button").addEventListener("click", () => {
    map.setOptions({gestureHandling: "none"});
    if (rectRange != null) {
      rectRange.setMap(null);
      rectRange = null;
    }
    startPoint = null;
    endPoint = null;
    rectRange = new google.maps.Rectangle({
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
    isMarking = true;
  });

  document.getElementById("maptiles_clear_button").addEventListener("click", () => {
    map.setOptions({gestureHandling: "auto"});
    if (rectRange != null) {
      rectRange.setMap(null);
      rectRange = null;
    }
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
      if (startPoint!= null) {
        endPoint = event.latLng;
        if (endPoint!= null) {
          let lng_diff = endPoint.lng() - startPoint.lng();
          if (lng_diff < 0 && lng_diff > -180 || lng_diff > 180) {
            rectRange.setBounds(new google.maps.LatLngBounds(endPoint, startPoint));
          } else {
            rectRange.setBounds(new google.maps.LatLngBounds(startPoint, endPoint));
          }
        }
      }
    }
  });

  document.addEventListener('pointerdown', function(event) {
    if (isMarking) {
      isMarking = false;
      map.setOptions({gestureHandling: "auto"});
      if (rectRange != null) {
        console.log("rectRange.getBounds(): " + rectRange.getBounds());
        rectRange.setOptions({clickable: true, editable: true, draggable: true})
      }
    }
  });

  //map.controls[google.maps.ControlPosition.BOTTOM_CENTER].push(document.getElementById("download_window"));
}

initMap();
