const actuators = {
  peltier: 5,
  humidifier: 5,
  pump: 5,
  tempSensors: 8,
  humiditySensors: 8
};

const socket = new WebSocket("ws://192.168.7.180:80/ws")

socket.onopen = () => {
  console.log("Connected to server");
};

socket.onmessage = (event) => {
  
  let parsed;
  try {
    parsed = JSON.parse(event.data);
  } catch(e) {
    console.error("Invalid JSON", e);
    return;
  }
  
  if (Array.isArray(parsed)) {
    parsed.forEach(message_JSON => {
      processMessage(message_JSON);
    });
  } else {
    console.log("Message from server:", event.data);
    processMessage(parsed);
  }
};
function processMessage(message_JSON) {
  if (!message_JSON || !message_JSON.id) {
    console.warn("Invalid message_JSON:", message_JSON);
    return;
  }
  
  switch (message_JSON.id) {
    case "UpdateStart":
      updateStart();
      break;
    case "UpdateStop":
      updateStop();
      break;
    case "UpdateDevMode":
      updateDeveloperMode(message_JSON);
      break;
    case "UpdateSystemOverview":
      updateSystemOverview(message_JSON);
      break;
    case "UpdateEnvironmentSensor":
      updateEnvironmentSensor(message_JSON);
      break;
    case "UpdatePumpStatus":
      updatePumpStatus(message_JSON);
      break;
    case "UpdateWaterLevel":
      updateWaterLevel(message_JSON);
      break;
    case "UpdateActuatorPower":
      updateActuatorPower(message_JSON);
      break;
    case "UpdateActuatorMode":
      updateActuatorMode(message_JSON);
      break;
    case "UpdatePIDInput":
      updatePIDInput(message_JSON);
      break;
    case "UpdatePIDSetpoint":
      updatePIDSetpoint(message_JSON);
      break;
    case "UpdatePIDActual":
      updatePIDActual(message_JSON);
      break;
    default:
      console.warn("Unknown message id:", message_JSON.id);
  }
}

socket.onclose = (event) => {
  console.log(`Connection closed, code: ${event.code}, reason: ${event.reason}`);
};

socket.onerror = (error) => {
  console.error("WebSocket error:", error);
};

const pidValues = {};
const setpointValues = {};

//// BEGIN LOGGING FUNCTIONS -> WEBSITE TO SOCKET FUNCTIONS

function logStartAction(){
  console.log(`Start button pressed |` , new Date().toISOString());
  let sending_obj = {};
  sending_obj.id = "LogStart";
  socket.send(JSON.stringify(sending_obj));
}

function logStopAction(){
  console.log(`Stop button pressed |` , new Date().toISOString());
  let sending_obj = {};
  sending_obj.id = "LogStop";
  socket.send(JSON.stringify(sending_obj));
}

function logDeveloperMode(){
  console.log(`Developer Mode Request Sent |` , new Date().toISOString());
  let sending_obj = {};
  sending_obj.id = "LogDeveloperMode"
  socket.send(JSON.stringify(sending_obj));
}

function logPumpAction(name){
  console.log(`${name} toggled |` , new Date().toISOString());
  let sending_obj = {};
  sending_obj.id = "LogPumpStatus";
  sending_obj.name = name;
  socket.send(JSON.stringify(sending_obj));
}

function logActuatorPowerStatus(name){
  console.log(`${name} power status toggled |` , new Date().toISOString());
  let sending_obj = {};
  sending_obj.id = "LogActuatorPower";
  sending_obj.name = name; 
  socket.send(JSON.stringify(sending_obj));
}

function logActuatorModeStatus(name){
  console.log(`${name} mode status toggled |` , new Date().toISOString());
  let sending_obj = {};
  sending_obj.id = "LogActuatorMode";
  sending_obj.name = name; 
  socket.send(JSON.stringify(sending_obj));
}

function logActuatorPID(name, P, I, D){
  console.log(`${name} PID parameters updated. P : ${P}, I : ${I}, D : ${D} |`, new Date().toISOString())
  let sending_obj = {};
  sending_obj.id = "LogActuatorPID";
  sending_obj.name = name; 
  sending_obj.P = P; 
  sending_obj.I = I; 
  sending_obj.D = D; 
  socket.send(JSON.stringify(sending_obj));
}

function logActuatorSetpoint(name, setpoint){
  console.log(`${name} setpoint updated: ${setpoint} |`, new Date().toISOString())
  let sending_obj = {};
  sending_obj.id = "LogActuatorSetpoint";
  sending_obj.name = name; 
  sending_obj.value = setpoint; 
  socket.send(JSON.stringify(sending_obj));
}

// INTERFACE TO CONTROL PANEL, EXTERIOR TO WEBSITE FUNCTIONS

function updateStart(){
  console.log(`Processing request to start device`);
  startBtn.classList.add("active");
  stopBtn.classList.remove("active");
  console.log("APPROVED!") 
}

function updateStop(){
  console.log(`Processing request to stop device`);
  stopBtn.classList.add("active");
  startBtn.classList.remove("active");
  console.log("APPROVED!") 
}

function updateDeveloperMode(command){
  console.log(`Processing request to update developer mode to ${command.state}`);
  const btn = document.getElementById('devFeatureBtn');
  if(!btn) return; 

  btn.state = command.state;
  btn.textContent = command.state === "On" ? "Disable Developer Mode": "Enable Developer Mode";
  btn.classList.toggle("active", command.state === "On");
  if(command.state == "On"){
    disableControls();
  }
  else{
    enableControls();
  }
  console.log("APPROVED!")
}

function updateSystemOverview(command){
  console.log(`Processing request to change ${command.name} to ${command.value}`)
  const updateElement = document.getElementById(command.name);
  if(!updateElement) return;

  updateElement.textContent = command.value; 
  console.log("APPROVED!")
}

function updateEnvironmentSensor(command) {
  console.log(`Processing request to update sensor ${command.name} to ${command.value}`);
  const sensor = document.querySelector(`.sensor[data-id='${command.name}']`);
  if (!sensor) return;

  const valueDiv = sensor.querySelector('.value');
  if (!valueDiv) return;

  valueDiv.textContent = command.value;
  console.log("APPROVED!");
}

function updatePumpStatus(command){
  console.log(`Processing request to update pump ${command.name} to ${command.state}`);
  const btn = document.querySelector(`.toggle-btn[data-id='${command.name}']`);
  if(!btn) return;

  btn.dataset.state = command.state;
  btn.textContent = command.state === "On" ? "On" : "Off";
  btn.classList.toggle("active", command.state === "On");
  console.log("APPROVED!")
}

function updateWaterLevel(command){
  console.log(`Processing request to update water level ${command.name} to ${command.state}`);
  const btn = document.querySelector(`.toggle-btn[data-id='${command.name}']`);
  if(!btn) return;

  btn.dataset.state = command.state;
  btn.textContent = command.state === "On" ? "High" : "Low";
  btn.classList.toggle("active", command.state === "On");
  console.log("APPROVED!")
}

function updateActuatorPower(command){
  console.log(`Processing request to update ${command.name} to ${command.state}`);
  const btn = document.querySelector(`[data-id="togglePower_${command.name}"]`);
  if(!btn) return;

  btn.dataset.state = command.state;
  btn.textContent = command.state === "On" ? "On" : "Off";
  btn.classList.toggle("active", command.state === "On");
  console.log("APPROVED!")
}

function updateActuatorMode(command){
  console.log(`Processing request to update ${command.name} to ${command.state}`);
  const btn = document.querySelector(`[data-id="toggleMode_${command.name}"]`);
  if(!btn) return;

  btn.dataset.state = command.state;
  btn.textContent = command.state === "Auto" ? "Auto" : "Manual";
  btn.classList.toggle("active", command.state === "Auto");
  console.log("APPROVED!")
}

function updatePIDInput(command) {
  console.log(`Processing request to update ${command.name} PID to P: ${command.P} I: ${command.I} D: ${command.D}`);

  const baseSelector = `.pid-inputs[data-id='updatePID_${command.name}']`;
  const container = document.querySelector(baseSelector);
  if (!container) return;

  if (command.P !== undefined) {
    const pInput = container.querySelector(`input[data-param='P']`);
    if (pInput) pInput.value = parseFloat(command.P).toFixed(2);
  }

  if (command.I !== undefined) {
    const iInput = container.querySelector(`input[data-param='I']`);
    if (iInput) iInput.value = parseFloat(command.I).toFixed(2);
  }

  if (command.D !== undefined) {
    const dInput = container.querySelector(`input[data-param='D']`);
    if (dInput) dInput.value = parseFloat(command.D).toFixed(2);
  }

  console.log("APPROVED!");
}

function updatePIDSetpoint(command) {
  console.log(`Processing request to update ${command.name} setpoint to ${command.value}`);
  const input = document.querySelector(`input[data-param="setpoint_${command.name}"]`);
  if (!input) return;
  
  input.value = command.value; 
  console.log("APPROVED!");
}

function updatePIDActual(command){
  console.log(`Processing request to update ${command.name} setpoint to ${command.value}`)
  const actual = document.querySelector(`input[data-param="actual_${command.name}"]`);
  if(!actual) return;

  actual.value = command.value;
  console.log("APPROVED!")
}

////////////////////////////////////////////////////////////

function setupStartStopButtons() {
  const startBtn = document.getElementById("startBtn");
  const stopBtn = document.getElementById("stopBtn");
  stopBtn.classList.add("active");

  startBtn.addEventListener("click", () => {
    if (!startBtn.classList.contains("active")) {
      logStartAction();
    }
  });

  stopBtn.addEventListener("click", () => {
    if (!stopBtn.classList.contains("active")) {
      logStopAction();
    }
  });
}

function disableControls() {
  document.querySelectorAll('.toggle-btn').forEach(btn => {
    btn.disabled = true;
    btn.classList.add('disabled');    
  });
  document.querySelectorAll('.mode-btn').forEach(btn => {
    btn.disabled = true;
    btn.classList.add('disabled');    
  });
}

function enableControls() {
  document.querySelectorAll('.toggle-btn').forEach(btn => {
    btn.disabled = false;
    btn.classList.remove('disabled');
  });
  document.querySelectorAll('.mode-btn').forEach(btn => {
    btn.disabled = false;
    btn.classList.remove('disabled');
  });
}

function createPumpWaterRow(index) {
  const container = document.createElement("div");
  container.className = "pump-water-row";
  const pumpLabel = document.createElement("div");
  pumpLabel.className = "label";
  pumpLabel.textContent = `Pump${index}`;

  const pumpBtn = document.createElement("button");
  pumpBtn.textContent = "Off";
  pumpBtn.className = "toggle-btn";
  pumpBtn.dataset.state = "Off";
  pumpBtn.setAttribute("data-id", `Pump${index}`);

  const waterLabel = document.createElement("div");
  waterLabel.className = "label";
  waterLabel.textContent = `Water_Level_Sensor_${index}`;

  const waterBtn = document.createElement("button");
  waterBtn.textContent = "Low";
  waterBtn.className = "toggle-btn";
  waterBtn.dataset.state = "Off";
  waterBtn.classList.remove("active")
  waterBtn.style.pointerEvents = "none";
  waterBtn.setAttribute("data-id", `waterLevelSensor${index}`);

  pumpBtn.addEventListener("click", () => {
    logPumpAction(pumpLabel.textContent)
  });
  container.appendChild(pumpLabel);
  container.appendChild(pumpBtn);
  container.appendChild(waterLabel);
  container.appendChild(waterBtn);

  return container;
}

function createActuatorElement(type) {
  const container = document.createElement("div");
  container.className = "actuator";

  const title = document.createElement("h4");
  title.textContent = `${type}`;
  container.appendChild(title);

  const controls = document.createElement("div");
  controls.className = "controls";

  // On/Off toggle button
  const toggleBtn = document.createElement("button");
  toggleBtn.className = "toggle-btn";
  toggleBtn.textContent = "Off";
  toggleBtn.dataset.state = "Off";
  toggleBtn.setAttribute("data-id", `togglePower_${type}`);

  toggleBtn.addEventListener("click", () => {
    logActuatorPowerStatus(type)
  });

  // Manual/Auto mode button
  const modeBtn = document.createElement("button");
  modeBtn.className = "mode-btn";
  modeBtn.textContent = "Manual";
  modeBtn.setAttribute("data-id", `toggleMode_${type}`);
  
  modeBtn.addEventListener("click", () => {
    logActuatorModeStatus(type)
  });

  // PID inputs container
  const pidInputs = document.createElement("div");
  pidInputs.className = "pid-inputs disabled";
  pidInputs.setAttribute("data-id", `updatePID_${type}`);
  const pidRefs = {}; // To store inputs references

  ["P", "I", "D"].forEach(param => {
    const input = document.createElement("input");
    input.type = "number";
    input.placeholder = param;
    input.step = "0.01";
    input.min = "0";
    input.setAttribute("data-param", param)
    input.disabled = false;
    
    pidInputs.appendChild(input);
    pidRefs[param] = input;

    input.addEventListener("change", () => {
      const P = parseFloat(pidRefs.P.value) || 0;
      const I = parseFloat(pidRefs.I.value) || 0;
      const D = parseFloat(pidRefs.D.value) || 0;

      if (!pidValues[type]) pidValues[type] = { P: 0, I: 0, D: 0, setpoint: 0 };
      pidValues[type].P = P;
      pidValues[type].I = I;
      pidValues[type].D = D;

      logActuatorPID(type, P, I, D);
    });

  });

  // Setpoint & Actual container
  const pidSetpointActual = document.createElement("div");
  pidSetpointActual.className = "pid-setpoint-actual";

  // Setpoint container
  const setpointContainer = document.createElement("div");
  setpointContainer.className = "setpoint-container";
  const setpointLabel = document.createElement("label");
  setpointLabel.textContent = "Setpoint:";
  const setpointInput = document.createElement("input");
  setpointInput.type = "number";
  setpointInput.placeholder = "Value";
  setpointInput.step = "0.1";
  setpointInput.min = "0";
  setpointInput.disabled = false;
  setpointInput.setAttribute("data-param", `setpoint_${type}`)
  setpointContainer.appendChild(setpointLabel);
  setpointContainer.appendChild(setpointInput);

setpointInput.addEventListener("change", () => {
  const setpoint = parseFloat(setpointInput.value) || 0;
  console.log(setpoint)
  logActuatorSetpoint(type, setpoint)
});

  // Actual value container
  const actualContainer = document.createElement("div");
  actualContainer.className = "actual-container";
  const actualLabel = document.createElement("span");
  actualLabel.textContent = "Actual:";
  const actualValue = document.createElement("span");
  actualValue.textContent = "N/A";
  actualContainer.appendChild(actualLabel);
  actualContainer.appendChild(actualValue);

  pidSetpointActual.appendChild(pidInputs);
  pidSetpointActual.appendChild(setpointContainer);
  pidSetpointActual.appendChild(actualContainer);
  pidSetpointActual.setAttribute("data-param", `actual_${type}`)

  controls.appendChild(toggleBtn);
  controls.appendChild(modeBtn);

  container.appendChild(controls);
  container.appendChild(pidSetpointActual);

  return container;
}

function createSensor(label, id, defaultValue) {
  const div = document.createElement("div");
  div.className = "sensor";
  div.dataset.id = id; 

  const labelDiv = document.createElement("div");
  labelDiv.className = "label";
  labelDiv.textContent = label;

  const valueDiv = document.createElement("div");
  valueDiv.className = "value";
  valueDiv.textContent = defaultValue;

  div.appendChild(labelDiv);
  div.appendChild(valueDiv);
  return div;
}

window.onload = () => {
  const tempSensors = document.getElementById("tempSensors");
  const humiditySensors = document.getElementById("humiditySensors");
  const peltierGroup = document.getElementById("peltierGroup");
  const humidifierGroup = document.getElementById("humidifierGroup");
  const pumpWaterGroup = document.getElementById("pumpWaterGroup");

  for (let i = 1; i <= actuators.tempSensors; i++){
    tempSensors.appendChild(createSensor(`Temp Sensor ${i}`, `tempsensor${i}`, "70°F"))  
    humiditySensors.appendChild(createSensor(`Humidity Sensor ${i}`, `humsensor${i}`, "0% R.H."))    
  }
      
  // Populate Peltier actuators
  peltierGroup.appendChild(createActuatorElement("FrontLeft"));
  peltierGroup.appendChild(createActuatorElement("FrontRight"));
  peltierGroup.appendChild(createActuatorElement("BackLeft"));
  peltierGroup.appendChild(createActuatorElement("BackRight"));
  peltierGroup.appendChild(createActuatorElement("Center"));

  // Populate Humidifier actuators
  for (let i = 1; i <= actuators.humidifier; i++) {
    humidifierGroup.appendChild(createActuatorElement(`Humidity${i}`));
  }

  // Populate Pumps + Water Sensors
  for (let i = 1; i <= actuators.pump; i++) {
    pumpWaterGroup.appendChild(createPumpWaterRow(i));
  }

  setupStartStopButtons();

  const btn = document.getElementById('devFeatureBtn');
  btn.state = "off";
  btn.classList.remove("active");
  btn.textContent = "Enable Developer Mode";

  btn.addEventListener('click', () => {  
    logDeveloperMode()
  });  
};
