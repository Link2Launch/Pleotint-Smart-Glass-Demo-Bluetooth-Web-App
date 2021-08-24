// codes for recieving messages
const STATUS_CODE = 's';
const TEMP_VALUE  = 't';

// string values for status codes
const STATUS_VAL = [
  'Connected, heater is OFF', // 0
  'Connected, heater is ON',  // 1
  'Not connected'             // 2
];

//codes for sending messages
const CHANGE_TEMP = 'c';
const HEATER_ON   = '1';
const HEATER_OFF  = '0';

$('#togBtn').change(function() {
  if (this.checked) {
    sendUartMessage(HEATER_ON);
  } else {
    sendUartMessage(HEATER_OFF);
  }
});

$('#st-data span').change(function {
  
});

function parseUartMessage(msg) {
  let msgParts = msg.trim(' ');
  
  if (msgParts[0] == STATUS_CODE) {
    $('#status-data span').text(STATUS_VAL[msgParts[1]]); // set the status text area to the data
  } else if (msgParts[0] == TEMP_VALUE) {
    $('#at-data span').text(msgParts[1]); // set the actual temp to the message data
  }
}

function sendUartMessage(msg) {
  nusSendString(msg);
}