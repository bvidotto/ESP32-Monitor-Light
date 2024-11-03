
const toggleSwitch = document.getElementById("toggleSwitch");
const colorSlider = document.getElementById("colorSlider");
const brightnessSlider = document.getElementById("brightnessSlider");
const url = "http://192.168.1.92" // Change it with your light's URL

chrome.storage.local.get(["toggleState"]).then((result) => toggleSwitch.checked = result.toggleState == 1 ? true : false);
chrome.storage.local.get(["brightnessLevel"]).then((result) => brightnessSlider.value = result.brightnessLevel);
chrome.storage.local.get(["colorLevel"]).then((result) => colorSlider.value = result.colorLevel);

chrome.storage.onChanged.addListener((changes) => {
  for (let [key, { oldValue, newValue }] of Object.entries(changes)) {
    switch(key){
      case "toggleState":
        toggleSwitch.checked = newValue == 1 ? true : false;
        break;
      case "brightnessLevel":
        brightnessSlider.value = newValue;
        break;
      case "colorLevel":
        colorSlider.value = newValue;
        break;
    }
  }
});

toggleSwitch.addEventListener("change", _ => {
  fetch(url + `/toggle`, {method: "PUT"})
  .then(_ => {
    chrome.storage.local.set({ "brightnessLevel" : brightness });
    chrome.storage.local.set({ "colorLevel" : color });
  }); 
});

colorSlider.addEventListener("change", _ => {
    getAndSendSettings();
});


brightnessSlider.addEventListener("change", _ => {
    getAndSendSettings();
});

document.getElementById("maxButton").addEventListener("click", _ => {
  fetch(url + `/max`, {method: "PUT"});
})

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
  fetch(url + `/setSettings?color=${color}&brightness=${brightness}`, {method: "PUT"})
  .then(_ => {
    chrome.storage.local.set({ "brightnessLevel" : brightness });
    chrome.storage.local.set({ "colorLevel" : color });
  });
}

function max(){
  fetch(url + `/max`);
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

async function loadInitialValues(){
  let response = await fetch(url + '/getSettings');
  if (!response.ok) {
    throw new Error(`Response status: ${response.status}`);
  }
  
  const responseBody = await response.json();
  
  document.getElementById("toggleSwitch").checked = responseBody.toggleState;
  document.getElementById("brightnessSlider").value = responseBody.brightnessLevel;
  document.getElementById("colorSlider").value = responseBody.colorLevel;
}

loadInitialValues();