const url = "http://192.168.1.92"  // Change it with your light's URL

fetch(url + '/getSettings')
.then((response) => response.json())
.then((data) => {
            chrome.storage.local.set({ "toggleState" : data.toggleState });
            chrome.storage.local.set({ "brightnessLevel" : data.brightnessLevel });
            chrome.storage.local.set({ "colorLevel" : data.colorLevel });
        });

chrome.commands.onCommand.addListener((command) => {
    switch (command){
        case "toggleSwitch":
            fetch(url + `/toggle`, {method: "PUT"})
            .then(response => response.text())
            .then(data =>chrome.storage.local.set({ "toggleState" : data }))
            break;
        case "increaseBrightness":
            fetch(url + `/brightnessAdd/?value=10`, {method: "PUT"})
            .then(response => response.text())
            .then(data => chrome.storage.local.set({ "brightnessLevel" : data }))
            break;
        case "decreaseBrightness":
            fetch(url + `/brightnessAdd/?value=-10`, {method: "PUT"})
            .then(response => response.text())
            .then(data => chrome.storage.local.set({ "brightnessLevel" : data }))
            break;
        case "increaseColorTemp":
            fetch(url + `/colorAdd/?value=10`, {method: "PUT"})
            .then(response => response.text())
            .then(data => chrome.storage.local.set({ "colorLevel" : data }))
            break;
        case "decreaseColorTemp":
            fetch(url + `/colorAdd/?value=-10`, {method: "PUT"})
            .then(response => response.text())
            .then(data => chrome.storage.local.set({ "colorLevel" : data }))
            break;
    }
  });