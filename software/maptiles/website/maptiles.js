// Initialize libraries
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
          rectRange.setBounds(new google.maps.LatLngBounds(startPoint, endPoint));
        }
      }
    }
  });

  document.addEventListener('mouseup', function(event) {
    if (isMarking) {
      isMarking = false;
      map.setOptions({gestureHandling: "auto"});
      if (rectRange != null) {
        rectRange.setOptions({clickable: true, editable: true, draggable: true})
      }
    }
  });
}

initMap();
