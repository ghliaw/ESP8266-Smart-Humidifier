const char index_html[] PROGMEM = R"ghliaxxx(
<!DOCTYPE html>
<html>
<head>
  <title>%NAME%</title>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.1.3/dist/css/bootstrap.min.css" rel="stylesheet">
  <script src="https://cdn.jsdelivr.net/npm/bootstrap@5.1.3/dist/js/bootstrap.bundle.min.js"></script>
  <script src="https://ajax.googleapis.com/ajax/libs/jquery/3.6.0/jquery.min.js"></script>
</head>
<body> 
<div class="container mt-3">
  <h1><center>%NAME%</center></h1>
  <br>
  <div class="d-flex justify-content-center mt-3">
    <div class="form-check form-switch">
      <input class="form-check-input" type="checkbox" id="mySwitch" name="mySwitch" style="transform: scale(3);" onchange="switchToggle()" %STATE%>
      <!-- <label class="form-check-label" for="mySwitch">Dark Mode</label> -->
    </div>
  </div>
</div>
<script>
  var gateway = `ws://${window.location.hostname}/ws`;
  var websocket;
  window.addEventListener('load', onLoad);
  function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen    = onOpen;
    websocket.onclose   = onClose;
    websocket.onmessage = onMessage; // <-- add this line
  }
  function onOpen(event) {
    console.log('Connection opened');
  }
  function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
  }
  function onMessage(event) {
    if (event.data == "1"){
      $("#mySwitch").prop("checked", true);
    }
    else{
      x = document.getElementById("mySwitch");
      $("#mySwitch").prop("checked", false);
    }
  }
  function onLoad(event) {
    initWebSocket();
  }
  function switchToggle() {
    websocket.send('toggle');
  }
</script>
</body>
</html>
)ghliaxxx";
