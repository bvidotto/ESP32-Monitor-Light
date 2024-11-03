function toggle(event){
  if (event.srcElement.checked){
    let color = document.getElementById("colorSlider").value;
    let brightness = document.getElementById("brightnessSlider").value;
    this.sendSettings(color, brightness);
  }
  else{
    this.sendSettings(0, 0);
  }
}

function getAndSendSettings(){
  let toggleState = document.getElementById("toggleSwitch").checked;
  if (toggleState){
    let color = document.getElementById("colorSlider").value;
    let brightness = document.getElementById("brightnessSlider").value;
    this.sendSettings(color, brightness);
  }
}

function sendSettings(color, brightness){
  fetch(`/setSettings?color=${color}&brightness=${brightness}`, {method: "PUT"});
}

function max(){
  fetch(`/max`, {method: "PUT"});
}    

function addScrollWheel(sliderElementId){
  let slider = document.getElementById(sliderElementId);
  slider.addEventListener("wheel", function(e){
    if (e.deltaY < 0){
      slider.stepUp();
    }else{
      slider.stepDown();
    }
    getAndSendSettings();
    e.preventDefault();
    e.stopPropagation();
  });
}
addScrollWheel("colorSlider");
addScrollWheel("brightnessSlider");