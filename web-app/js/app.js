// codes for recieving messages
const STATUS_CODE = 's';
const TEMPA_VALUE  = 't';
const TEMPB_VALUE = 'k';

// string values for status codes
const STATUS_VAL = [
  'Connected, heater is off',                // 0
  'Connected, heater is on',                 // 1
  'Thigh heater is currently off',           // 2
  'Thigh heater is currently on',            // 3
  'Leg heater is currently off',             // 4
  'Leg heater is currently on',              // 5
  'Heaters OFF, Maximum safe temp exceeded', // 6
  'Disconnected',                            // 7
  'Connected'                                // 8
];

//codes for sending messages
const CHANGE_TEMPA = 'c';
const CHANGE_TEMPB = 'd';
const HEATERA_SWITCH  = 'p';
const HEATERB_SWITCH  = 'q';

// heater power states
const HEATER_OFF  = '0';
const HEATER_ON   = '1';


$('#togBtn1').change(function() {
  if (this.checked) {
    sendUartMessage(HEATERA_SWITCH + ' ' + HEATER_ON, 0);
  } else {
    sendUartMessage(HEATERA_SWITCH + ' ' + HEATER_OFF, 0);
    
    knob1Color = HEATER_DEF_COLOR;
    $("#heater-1-knob").trigger('change');
  }
});

$('#togBtn2').change(function() {
  if (this.checked) {
    sendUartMessage(HEATERB_SWITCH + ' ' + HEATER_ON, 0);
  } else {
    sendUartMessage(HEATERB_SWITCH + ' ' + HEATER_OFF, 0);
    
    knob2Color = HEATER_DEF_COLOR;
    $("#heater-2-knob").trigger('change');
  }
});

function onKnobValChange(heater, val) {
  if (heater == 1) {
    sendUartMessage(CHANGE_TEMPA + ' ' + val, 0);
  } else if (heater == 2){
    sendUartMessage(CHANGE_TEMPB + ' ' + val, 0);
  }
}

function onNewConnection() {
  sendUartMessage(STATUS_CODE + ' 8', 0);
  
  $('#st1-data span').text($('#heater-1-knob').val().replace('°ᶠ', '')); // set our set temp text box value to the knob
  sendUartMessage(CHANGE_TEMPA + ' ' + $('#heater-1-knob').val().replace('°ᶠ', ''), 200);
  
  $('#st2-data span').text($('#heater-2-knob').val().replace('°ᶠ', '')); // set our set temp text box value to the knob
  sendUartMessage(CHANGE_TEMPB + ' ' + $('#heater-2-knob').val().replace('°ᶠ', ''), 400);
  
  if ($('#togBtn1').is(':checked')) {
    sendUartMessage(HEATERA_SWITCH + ' ' + HEATER_ON, 600);
  } else {
    sendUartMessage(HEATERA_SWITCH + ' ' + HEATER_OFF, 600);
  }
  
  if ($('#togBtn2').is(':checked')) {
    sendUartMessage(HEATERB_SWITCH + ' ' + HEATER_ON, 800);
  } else {
    sendUartMessage(HEATERB_SWITCH + ' ' + HEATER_OFF, 800);
  }
}

function parseUartMessage(msg) {
  let msgParts = msg.split(' ');
  
  if (msgParts[0] == STATUS_CODE) {
    if (msgParts[1] == '4' || msgParts[1] == '5') {
      $('#status2-data span').text(STATUS_VAL[msgParts[1]]); // set the status text area to the data
    } else {
      $('#status-data span').text(STATUS_VAL[msgParts[1]]);
    }
    
    if (! (msgParts[1] == '2' || msgParts[1] == '3' || msgParts[1] == '4' || msgParts[1] == '5')) {
      $('#status2-data span').text(''); // set the status text area to the data
    }
    
    if (msgParts[1] == '2') {
      knob1Color = HEATER_OFF_COLOR;
    } else if (msgParts[1] == '3') {
      knob1Color = HEATER_ON_COLOR;
    } else if (msgParts[1] == '4') {
      knob2Color = HEATER_OFF_COLOR;
    } else if (msgParts[1] == '5') {
      knob2Color = HEATER_ON_COLOR;
    } else if (msgParts[1] == '6') {
      knob1Color = HEATER_OFF_COLOR;
      knob2Color = HEATER_OFF_COLOR;
    }
    
    if (!$('#togBtn1').is(':checked')) {
      knob1Color = HEATER_DEF_COLOR;
    }
    if (!$('#togBtn2').is(':checked')) {
      knob2Color = HEATER_DEF_COLOR;
    }
    
    $("#heater-1-knob").trigger('change');
    $("#heater-2-knob").trigger('change');
    
  } else if (msgParts[0] == TEMPA_VALUE) {
    $('#at1-data span').text(msgParts[1]); // set the actual temp to the message data
  } else if (msgParts[0] == TEMPB_VALUE) {
    $('#at2-data span').text(msgParts[1]); // set the actual temp to the message data
  }
}

function sendUartMessage(msg, delay) {
  setTimeout(function() {
    nusSendString(msg);
  }, delay);
}

function setStatusText(index) {
  $('#status-data span').text(STATUS_VAL[index]);
  $('#status2-data span').text('');
}

$('#info-button').click(function() {
  $('#splash-screen').toggleClass('splash-screen--show', true);
});

$('#splash-screen').click(function() {
  $('#splash-screen').toggleClass('splash-screen--show', false);
});