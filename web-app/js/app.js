// codes for recieving messages
const STATUS_CODE = 's';
const TEMP_VALUE  = 't';
const TEMP2_VALUE = 'k';

// string values for status codes
const STATUS_VAL = [
  'Connected, heater is off', // 0
  'Connected, heater is on',  // 1
  'Heater 1 is off',          // 2
  'Heater 1 is on',           // 3
  'Heater 2 is off',          // 4
  'Heater 2 is on',           // 5
  'Heaters OFF, Maximum safe temp exceeded', // 6
  'Disconnected',             // 7
  'Connected'                 // 8
];

//codes for sending messages
const CHANGE_TEMP = 'c';
const HEATER_PWR  = 'p';

// heater power states
const HEATER_ON   = '1';
const HEATER_OFF  = '0';

$('#togBtn').change(function() {
  if (this.checked) {
    sendUartMessage(HEATER_PWR + ' ' + HEATER_ON);
  } else {
    sendUartMessage(HEATER_PWR + ' ' + HEATER_OFF);
  }
});

function onKnobValChange(val) {
  sendUartMessage(CHANGE_TEMP + ' ' + val);
}

function onNewConnection() {
  sendUartMessage(STATUS_CODE + ' 8');
  
  $('#st-data span').text($('.knob').val()); // set our set temp text box value to the knob
  sendUartMessage(CHANGE_TEMP + ' ' + $('.knob').val());
}

function parseUartMessage(msg) {
  let msgParts = msg.split(' ');
  
  if (msgParts[0] == STATUS_CODE) {
    $('#status-data span').text(STATUS_VAL[msgParts[1]]); // set the status text area to the data
    
    if (msgParts[1] == '0') {
      $('#togBtn').prop('checked', false);
    } else if (msgParts[1] == '1') {
      $('#togBtn').prop('checked', true);
    }
    
  } else if (msgParts[0] == TEMP_VALUE) {
    $('#at-data span').text(msgParts[1]); // set the actual temp to the message data
  }
}

function sendUartMessage(msg) {
  nusSendString(msg);
}

function setStatusText(index) {
  $('#status-data span').text(STATUS_VAL[index]);
}