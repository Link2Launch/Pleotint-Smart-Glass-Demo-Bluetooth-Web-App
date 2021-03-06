const bleNusServiceUUID  = '6e400001-b5a3-f393-e0a9-e50e24dcca9e';
const bleNusCharRXUUID   = '6e400002-b5a3-f393-e0a9-e50e24dcca9e';
const bleNusCharTXUUID   = '6e400003-b5a3-f393-e0a9-e50e24dcca9e';
const MTU = 20;

var bleDevice;
var bleServer;
var nusService;
var rxCharacteristic;
var txCharacteristic;

var connected = false;

function connectionToggle() {
  if (connected) {
    disconnect();
  } else {
    connect();
  }
}

// Sets button to either Connect or Disconnect
function setConnButtonState(enabled) {
  if (enabled) {
    $("#clientConnectButton").html("Disconnect from<br>" + bleDevice.name);
    $('#function-disabled').toggleClass('functions--enabled', true);
  } else {
    $("#clientConnectButton").html("Connect to heater");
    $('#function-disabled').toggleClass('functions--enabled', false);
    setStatusText(7);
  }
}

function connect() {
  if (!navigator.bluetooth) {
    console.log('WebBluetooth API is not available.\r\n' +
    'Please make sure the Web Bluetooth flag is enabled.');
    alert("Web Bluetooth API is not currently supported on this device.\n" +
          "Please use an Android, OSX, or Windows device running Chrome or Edge.")
    return;
  }
  console.log('Requesting Bluetooth Device...');
  navigator.bluetooth.requestDevice({
    filters: [{services: [bleNusServiceUUID]}],
    // optionalServices: [bleNusServiceUUID],
    acceptAllDevices: false
  })
  .then(device => {
    bleDevice = device;
    console.log('Found ' + device.name);
    console.log('Connecting to GATT Server...');
    $("#clientConnectButton").html("Connecting...");
    bleDevice.addEventListener('gattserverdisconnected', onDisconnected);
    return device.gatt.connect();
  })
  .then(server => {
    console.log('Locate NUS service');
    return server.getPrimaryService(bleNusServiceUUID);
  }).then(service => {
    nusService = service;
    console.log('Found NUS service: ' + service.uuid);
  })
  .then(() => {
    console.log('Locate RX characteristic');
    return nusService.getCharacteristic(bleNusCharRXUUID);
  })
  .then(characteristic => {
    rxCharacteristic = characteristic;
    console.log('Found RX characteristic');
  })
  .then(() => {
    console.log('Locate TX characteristic');
    return nusService.getCharacteristic(bleNusCharTXUUID);
  })
  .then(characteristic => {
    txCharacteristic = characteristic;
    console.log('Found TX characteristic');
  })
  .then(() => {
    console.log('Enable notifications');
    return txCharacteristic.startNotifications();
  })
  .then(() => {
    console.log('Notifications started');
    txCharacteristic.addEventListener('characteristicvaluechanged',
    handleNotifications);
    connected = true;
    console.log(bleDevice.name + ' Connected.');
    onNewConnection();
    setConnButtonState(true);
  })
  .catch(error => {
    console.log('' + error);
    if(bleDevice && bleDevice.gatt.connected)
    {
      bleDevice.gatt.disconnect();
    }
  });
}

function disconnect() {
  if (!bleDevice) {
    console.log('No Bluetooth Device connected...');
    return;
  }
  console.log('Disconnecting from Bluetooth Device...');
  if (bleDevice.gatt.connected) {
    bleDevice.gatt.disconnect();
    connected = false;
    setConnButtonState(false);
    console.log('Bluetooth Device connected: ' + bleDevice.gatt.connected);
  } else {
    console.log('> Bluetooth Device is already disconnected');
  }
}

function onDisconnected() {
  connected = false;
  console.log(bleDevice.name + ' Disconnected.');
  setConnButtonState(false);
}

function handleNotifications(event) {
  let value = event.target.value;
  // Convert raw data bytes to character values and use these to
  // construct a string.
  let str = "";
  for (let i = 0; i < value.byteLength; i++) {
    str += String.fromCharCode(value.getUint8(i));
  }
  console.log('[RECV]: ' + str);
  
  parseUartMessage(str);
}

function nusSendString(s) {
  if(bleDevice && bleDevice.gatt.connected) {
    console.log("[SEND]: " + s);
    let val_arr = new Uint8Array(s.length)
    for (let i = 0; i < s.length; i++) {
      let val = s[i].charCodeAt(0);
      val_arr[i] = val;
    }
    sendNextChunk(val_arr);
  } else {
    console.log('Not connected to a device yet.');
  }
}

function sendNextChunk(a) {
  let chunk = a.slice(0, MTU);
  rxCharacteristic.writeValue(chunk)
  .then(function() {
    if (a.length > MTU) {
      sendNextChunk(a.slice(MTU));
    }
  });
}