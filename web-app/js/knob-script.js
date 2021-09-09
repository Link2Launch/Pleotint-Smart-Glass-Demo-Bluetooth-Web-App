const HEATER_ON_COLOR = 'rgb(255, 0, 0)';
const HEATER_OFF_COLOR = 'rgb(135, 206, 235)';
const HEATER_DEF_COLOR = 'rgb(255, 236, 3)';

var knob1Color = HEATER_DEF_COLOR;
var knob2Color = HEATER_DEF_COLOR;

$(function($) {

  $("#heater-1-knob").knob({
    change : function (value) {
      //console.log("change : " + value);
    },
    release : function (value) {
      //console.log(this.$.attr('value'));
      console.log("[KNOB 1 RELEASE] : " + value);
      $('#st1-data span').text(value);
      onKnobValChange(1, value);
    },
    cancel : function () {
      console.log("cancel : ", this);
    },
    format : function(value) {
      return value + '°ᶠ';
    },
    draw : function () {
      
      // "tron" case
      if(this.$.data('skin') == 'tron') {
        
        this.cursorExt = 0.3;
        
        var a = this.arc(this.cv)  // Arc
        , pa                   // Previous arc
        , r = 1;
        
        this.g.lineWidth = this.lineWidth;
        
        if (this.o.displayPrevious) {
          pa = this.arc(this.v);
          this.g.beginPath();
          this.g.strokeStyle = this.pColor;
          this.g.arc(this.xy, this.xy, this.radius - this.lineWidth, pa.s, pa.e, pa.d);
          this.g.stroke();
        }
        
        this.g.beginPath();
        this.g.strokeStyle = knob1Color;
        this.g.arc(this.xy, this.xy, this.radius - this.lineWidth, a.s, a.e, a.d);
        this.g.stroke();
        
        this.g.lineWidth = 2;
        this.g.beginPath();
        this.g.strokeStyle = knob1Color;
        this.g.arc( this.xy, this.xy, this.radius - this.lineWidth + 1 + this.lineWidth * 2 / 3, 0, 2 * Math.PI, false);
        this.g.stroke();
        
        $('#heater-1-knob').css('color', knob1Color);
        
        return false;
      }
    }
  });

  $("#heater-2-knob").knob({
    change : function (value) {
      //console.log("change : " + value);
    },
    release : function (value) {
      //console.log(this.$.attr('value'));
      console.log("[KNOB 2 RELEASE] : " + value);
      $('#st2-data span').text(value);
      onKnobValChange(2, value);
    },
    cancel : function () {
      console.log("cancel : ", this);
    },
    format : function(value) {
      return value + '°ᶠ';
    },
    draw : function () {
      
      // "tron" case
      if(this.$.data('skin') == 'tron') {
        
        this.cursorExt = 0.3;
        
        var a = this.arc(this.cv)  // Arc
        , pa                   // Previous arc
        , r = 1;
        
        this.g.lineWidth = this.lineWidth;
        
        if (this.o.displayPrevious) {
          pa = this.arc(this.v);
          this.g.beginPath();
          this.g.strokeStyle = this.pColor;
          this.g.arc(this.xy, this.xy, this.radius - this.lineWidth, pa.s, pa.e, pa.d);
          this.g.stroke();
        }
        
        this.g.beginPath();
        this.g.strokeStyle = knob2Color;
        this.g.arc(this.xy, this.xy, this.radius - this.lineWidth, a.s, a.e, a.d);
        this.g.stroke();
        
        this.g.lineWidth = 2;
        this.g.beginPath();
        this.g.strokeStyle = knob2Color;
        this.g.arc( this.xy, this.xy, this.radius - this.lineWidth + 1 + this.lineWidth * 2 / 3, 0, 2 * Math.PI, false);
        this.g.stroke();
        
        $('#heater-2-knob').css('color', knob2Color);
        
        return false;
      }
    }
  });

});

