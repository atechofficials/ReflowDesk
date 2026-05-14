// JavaScript for ReflowDesk Web Interface
// Version: 1.2.5
// Github: https://github.com/atechofficials/ReflowDesk
// Author: Mrinal @atechofficials
// License: General Public License v3.0

"use strict";

const $ = id => document.getElementById(id);
const tokenKey = "reflowdeskToken";
const themeKey = "reflowdeskTheme";
const validPages = new Set(["dashboard", "profiles", "settings", "device", "logs"]);

const state = {
  token: sessionStorage.getItem(tokenKey) || "",
  setupMode: false,
  ws: null,
  wsReady: false,
  reconnectTimer: 0,
  settings: null,
  profiles: [],
  currentProfile: 0,
  device: null,
  telemetry: null,
  runBaseMs: 0,
  liveChart: null,
  profileChart: null,
  liveSamples: 0,
  dragProfilePoint: -1,
  modalResolve: null,
  modalOpen: false,
  faultAckPromptKey: "",
  theme: localStorage.getItem(themeKey) || "dark"
};

const els = {
  authScreen: $("auth-screen"),
  appShell: $("app-shell"),
  authTitle: $("auth-title"),
  authCopy: $("auth-copy"),
  authForm: $("auth-form"),
  authSubmit: $("auth-submit"),
  pinInput: $("pin-input"),
  authNote: $("auth-note"),
  wsPill: $("ws-pill"),
  ipPill: $("ip-pill"),
  statePill: $("state-pill"),
  pageTitle: $("page-title"),
  deviceLine: $("device-line"),
  sideFw: $("side-fw"),
  profileActive: $("profile-active"),
  activeProfile: $("active-profile"),
  plateTemp: $("plate-temp"),
  targetTemp: $("target-temp"),
  ambientTemp: $("ambient-temp"),
  stageName: $("stage-name"),
  timeLeft: $("time-left"),
  faultName: $("fault-name"),
  ssrState: $("ssr-state"),
  ssrDuty: $("ssr-duty"),
  fanState: $("fan-state"),
  fanRpm: $("fan-rpm"),
  boardFanState: $("board-fan-state"),
  boardFanRpm: $("board-fan-rpm"),
  safetyState: $("safety-state"),
  cooldownLeft: $("cooldown-left"),
  profileTabs: $("profile-tabs"),
  profileCurveName: $("profile-curve-name"),
  deviceInfo: $("device-info"),
  deviceSummary: $("device-summary"),
  eventList: $("event-list"),
  toastStack: $("toast-stack"),
  otaProgress: $("ota-progress"),
  otaNote: $("ota-note"),
  otaCurrentVersion: $("ota-current-version"),
  otaFreeSpace: $("ota-free-space"),
  sidebar: $("sidebar"),
  modal: $("confirm-modal"),
  modalTitle: $("modal-title"),
  modalMessage: $("modal-message"),
  modalCancel: $("modal-cancel"),
  modalConfirm: $("modal-confirm"),
  themeToggle: $("theme-toggle"),
  themeLabel: $("theme-label"),
  themeIcon: $("theme-icon")
};

const limits = {
  preheatTempC: [50, 180],
  soakTempC: [80, 210],
  reflowTempC: [80, 245],
  stageSeconds: [20, 600],
  stageGapC: 10,
  preheatSafeTouchOffsetC: 15
};

const profilePointColors = {
  start: "#8ea2b7",
  preheat: "#ffbd4a",
  soak: "#f97316",
  reflow: "#ff5c7a",
  cool: "#6aa4ff"
};

function clamp(value, low, high) {
  return Math.min(high, Math.max(low, value));
}

function roundStep(value, step) {
  return Math.round(value / step) * step;
}

function quantize(value, low, high, step) {
  const parsed = numberOr(value, low);
  return clamp(roundStep(parsed - low, step) + low, low, high);
}

function numberOr(value, fallback = 0) {
  const parsed = Number(value);
  return Number.isFinite(parsed) ? parsed : fallback;
}

function setRangeVisual(input) {
  if (!input) return;
  const low = Number(input.min || 0);
  const high = Number(input.max || 100);
  const value = clamp(Number(input.value || low), low, high);
  const progress = high > low ? ((value - low) / (high - low)) * 100 : 0;
  input.style.setProperty("--range-progress", `${progress}%`);
}

function syncRangePair(sliderId, valueId, value, low, high, step) {
  const next = quantize(value, low, high, step);
  const slider = $(sliderId);
  const valueEl = $(valueId);
  slider.value = next;
  valueEl.value = next;
  if (valueEl.tagName === "OUTPUT") {
    setText(valueEl, next);
  }
  setRangeVisual(slider);
  return next;
}

function applyTheme(theme) {
  const normalized = theme === "light" ? "light" : "dark";
  state.theme = normalized;
  document.body.classList.toggle("theme-light", normalized === "light");
  document.body.classList.toggle("theme-dark", normalized !== "light");
  localStorage.setItem(themeKey, normalized);
  renderThemeToggle();
}

function renderThemeToggle() {
  if (!els.themeToggle) return;
  const isLight = state.theme === "light";
  setText(els.themeLabel, isLight ? "Dark" : "Light");
  els.themeIcon.className = `mask ${isLight ? "icon-moon" : "icon-sun"}`;
}

function preheatMinTempC() {
  const safeTouch = state.settings ? numberOr(state.settings.safeTouchC, 45) : 45;
  return clamp(Math.round(safeTouch + limits.preheatSafeTouchOffsetC), limits.preheatTempC[0], limits.preheatTempC[1]);
}

function normalizeProfileValues(values) {
  const normalized = { ...values };
  normalized.name = String(normalized.name || "").trim();
  normalized.coolingProfile = normalized.coolingProfile || "normal";
  normalized.preheatSeconds = clamp(roundStep(numberOr(normalized.preheatSeconds, limits.stageSeconds[0]), 5),
    limits.stageSeconds[0], limits.stageSeconds[1]);
  normalized.soakSeconds = clamp(roundStep(numberOr(normalized.soakSeconds, limits.stageSeconds[0]), 5),
    limits.stageSeconds[0], limits.stageSeconds[1]);
  normalized.reflowSeconds = clamp(roundStep(numberOr(normalized.reflowSeconds, limits.stageSeconds[0]), 5),
    limits.stageSeconds[0], limits.stageSeconds[1]);
  const preheatMinC = preheatMinTempC();
  normalized.preheatTempC = clamp(Math.round(numberOr(normalized.preheatTempC, preheatMinC)),
    preheatMinC, limits.preheatTempC[1]);
  const soakMinC = Math.min(limits.soakTempC[1], normalized.preheatTempC + limits.stageGapC);
  normalized.soakTempC = clamp(Math.round(numberOr(normalized.soakTempC, soakMinC)), soakMinC, limits.soakTempC[1]);
  const reflowMinC = Math.min(limits.reflowTempC[1], normalized.soakTempC + limits.stageGapC);
  normalized.reflowTempC = clamp(Math.round(numberOr(normalized.reflowTempC, reflowMinC)), reflowMinC, limits.reflowTempC[1]);
  return normalized;
}

function setText(el, value) {
  if (el) el.textContent = value;
}

function fmtTemp(value, ok = true, decimals = 1) {
  if (!ok || value === null || value === undefined || Number.isNaN(Number(value))) return "--.-C";
  return `${Number(value).toFixed(decimals)}C`;
}

function fmtBool(on) {
  return on ? "ON" : "OFF";
}

function tokenHeaders() {
  return state.token ? { "X-Reflow-Token": state.token } : {};
}

async function api(path, options = {}) {
  const headers = options.headers ? { ...options.headers } : {};
  if (options.auth !== false) Object.assign(headers, tokenHeaders());
  if (options.body && !(options.body instanceof FormData)) headers["Content-Type"] = "application/json";

  const response = await fetch(path, {
    method: options.method || "GET",
    headers,
    body: options.body instanceof FormData ? options.body : (options.body ? JSON.stringify(options.body) : undefined)
  });
  const text = await response.text();
  let data = {};
  if (text) {
    try {
      data = JSON.parse(text);
    } catch (_) {
      data = { message: text };
    }
  }
  if (!response.ok) {
    throw new Error(data.message || `HTTP ${response.status}`);
  }
  return data;
}

function toast(message, level = "info") {
  const item = document.createElement("div");
  item.className = `toast ${level}`;
  item.textContent = message;
  els.toastStack.appendChild(item);
  setTimeout(() => item.remove(), 5200);
}

function logEvent(message, level = "info", timeMs = Date.now()) {
  const row = document.createElement("div");
  row.className = `event-row ${level}`;
  const time = document.createElement("time");
  time.textContent = new Date(timeMs).toLocaleTimeString();
  const text = document.createElement("div");
  text.textContent = message;
  row.append(time, text);
  els.eventList.prepend(row);
  while (els.eventList.children.length > 80) {
    els.eventList.lastElementChild.remove();
  }
}

function showConfirm({ title, message, confirmText = "Confirm", danger = false }) {
  if (state.modalOpen) return Promise.resolve(false);
  state.modalOpen = true;
  setText(els.modalTitle, title);
  setText(els.modalMessage, message);
  setText(els.modalConfirm, confirmText);
  els.modalConfirm.className = danger ? "danger-btn" : "primary-btn";
  els.modal.classList.remove("hidden");
  return new Promise(resolve => {
    state.modalResolve = resolve;
  });
}

function closeConfirm(result) {
  els.modal.classList.add("hidden");
  state.modalOpen = false;
  const resolve = state.modalResolve;
  state.modalResolve = null;
  if (resolve) resolve(result);
}

function showAuth(setupMode, note = "") {
  state.setupMode = setupMode;
  els.appShell.classList.add("locked");
  els.authScreen.classList.remove("hidden");
  els.pinInput.value = "";
  setText(els.authTitle, setupMode ? "Create Web PIN" : "Unlock Device");
  setText(els.authCopy, setupMode ? "Create the local PIN used to protect ReflowDesk controls." : "Enter the local web PIN for this ReflowDesk.");
  setText(els.authSubmit, setupMode ? "Create PIN" : "Sign In");
  setText(els.authNote, note || (setupMode ? "Factory reset clears this PIN." : ""));
  setTimeout(() => els.pinInput.focus(), 100);
}

function unlockApp() {
  els.authScreen.classList.add("hidden");
  els.appShell.classList.remove("locked");
}

async function checkAuth() {
  try {
    const status = await api("/api/auth/status", { auth: Boolean(state.token) });
    if (!status.pinSet) {
      showAuth(true);
      return;
    }
    if (status.authenticated) {
      await startApp();
      return;
    }
    sessionStorage.removeItem(tokenKey);
    state.token = "";
    showAuth(false);
  } catch (err) {
    showAuth(false);
    setText(els.authNote, err.message);
  }
}

async function submitAuth(event) {
  event.preventDefault();
  const pin = els.pinInput.value.trim();
  if (pin.length < 4) {
    setText(els.authNote, "PIN must be at least 4 characters.");
    return;
  }
  try {
    const path = state.setupMode ? "/api/auth/setup" : "/api/auth/login";
    const result = await api(path, { method: "POST", auth: false, body: { pin } });
    state.token = result.token;
    sessionStorage.setItem(tokenKey, state.token);
    await startApp();
  } catch (err) {
    setText(els.authNote, err.message);
  }
}

async function startApp() {
  unlockApp();
  initCharts();
  await Promise.all([loadDeviceInfo(), loadSettings(), loadProfiles()]);
  renderAll();
  switchPage(pageFromHash(), false);
  connectWebSocket();
}

function initCharts() {
  if (state.liveChart || !window.Chart) return;
  const liveCtx = $("live-chart");
  const profileCtx = $("profile-chart");
  const chartOptions = (xTitle, yTitle) => ({
    responsive: true,
    maintainAspectRatio: false,
    animation: false,
    interaction: { intersect: false, mode: "index" },
    scales: {
      x: {
        type: "linear",
        title: { display: true, text: xTitle, color: "#8ea2b7" },
        grid: { color: "rgba(142,162,183,.14)" },
        ticks: { color: "#8ea2b7", callback: value => `${value}s` }
      },
      y: {
        title: { display: true, text: yTitle, color: "#8ea2b7" },
        grid: { color: "rgba(142,162,183,.14)" },
        ticks: { color: "#8ea2b7", callback: value => `${value}C` }
      }
    },
    plugins: { legend: { display: false } }
  });
  state.liveChart = new Chart(liveCtx, {
    type: "line",
    data: {
      datasets: [
        { label: "Plate C", data: [], borderColor: "#37d6d0", backgroundColor: "transparent", pointRadius: 0, tension: .18 },
        { label: "Target C", data: [], borderColor: "#ffbd4a", backgroundColor: "transparent", pointRadius: 0, tension: 0 },
        { label: "Ambient C", data: [], borderColor: "#6aa4ff", backgroundColor: "transparent", pointRadius: 0, tension: .18 }
      ]
    },
    options: chartOptions("Time (s)", "Temperature (C)")
  });
  state.profileChart = new Chart(profileCtx, {
    type: "line",
    data: {
      datasets: [
        {
          label: "Profile Target C",
          data: [],
          borderColor: "#56d364",
          backgroundColor: "rgba(86,211,100,.10)",
          pointBackgroundColor: context => stagePointColor(context.dataIndex),
          pointBorderColor: context => stagePointColor(context.dataIndex),
          pointBorderWidth: 2,
          pointRadius: context => (context.dataIndex === 0 || context.dataIndex === 4 ? 4 : 6),
          pointHoverRadius: context => (context.dataIndex === 0 || context.dataIndex === 4 ? 6 : 8),
          tension: 0,
          fill: true
        }
      ]
    },
    options: {
      ...chartOptions("Time (s)", "Target Temperature (C)"),
      interaction: { intersect: true, mode: "nearest" },
      scales: {
        ...chartOptions("Time (s)", "Target Temperature (C)").scales,
        x: {
          ...chartOptions("Time (s)", "Target Temperature (C)").scales.x,
          min: 0,
          max: 300
        },
        y: {
          ...chartOptions("Time (s)", "Target Temperature (C)").scales.y,
          min: 0,
          max: 260
        }
      },
      plugins: {
        legend: { display: false },
        tooltip: {
          callbacks: {
            label: context => {
              const raw = context.raw || {};
              const label = raw.stage || "Target";
              return `${label}: ${Math.round(context.parsed.y)}C`;
            }
          }
        }
      }
    }
  });
  bindProfileChartDrag();
}

function stagePointColor(index) {
  return [
    profilePointColors.start,
    profilePointColors.preheat,
    profilePointColors.soak,
    profilePointColors.reflow,
    profilePointColors.cool
  ][index] || "#56d364";
}

async function loadDeviceInfo() {
  state.device = await api("/api/device/info");
}

async function loadSettings() {
  state.settings = await api("/api/settings");
}

async function loadProfiles(preserveCurrent = false) {
  const previous = state.currentProfile;
  const data = await api("/api/profiles");
  state.profiles = data.profiles || [];
  state.currentProfile = preserveCurrent ? clamp(previous, 0, Math.max(0, state.profiles.length - 1)) : Number(data.selectedProfileIndex || 0);
}

function renderAll() {
  renderDevice();
  renderRunControls();
  renderSettings();
  renderProfiles();
  if (state.telemetry) renderTelemetry(state.telemetry);
}

function renderDevice() {
  if (!state.device) return;
  document.querySelectorAll("[data-device-model-label]").forEach(el => setText(el, state.device.model || "Device"));
  setText(els.sideFw, `FW ${state.device.firmwareVersion}`);
  setText(els.deviceLine, `${state.device.model} | ${state.device.firmwareVersion}`);
  setText(els.ipPill, `IP: ${state.device.ip || "--"}`);
  setText(els.deviceSummary, `${state.device.name} ${state.device.model}`);
  setText(els.otaCurrentVersion, state.device.firmwareVersion || "--");
  setText(els.otaFreeSpace, `${Math.round((state.device.freeSketchSpace || 0) / 1024)} KB`);
  const info = [
    ["Firmware", state.device.firmwareVersion],
    ["Hardware", state.device.hardwareVersion],
    ["IP", state.device.ip],
    ["MAC", state.device.mac],
    ["WiFi", state.device.wifiMode],
    ["RSSI", `${state.device.rssi} dBm`],
    ["Flash", `${Math.round(state.device.flashSize / 1048576)} MB`],
    ["LittleFS", `${Math.round(state.device.littleFsUsed / 1024)} KB used`],
    ["Free Heap", `${Math.round(state.device.freeHeap / 1024)} KB`],
    ["OTA Free", `${Math.round(state.device.freeSketchSpace / 1024)} KB`]
  ];
  els.deviceInfo.innerHTML = "";
  info.forEach(([label, value]) => {
    const row = document.createElement("div");
    row.innerHTML = `<span>${label}</span><strong>${value ?? "--"}</strong>`;
    els.deviceInfo.appendChild(row);
  });
}

function renderSettings() {
  if (!state.settings) return;
  $("safe-touch").value = state.settings.safeTouchC;
  $("safety-cutoff").value = state.settings.safetyCutoffC;
  syncRangePair("led-brightness-slider", "led-brightness", state.settings.ledBrightness, 0, 100, 5);
  const buzzerLevel = clamp(numberOr(state.settings.buzzerLevel, state.settings.buzzerEnabled ? 3 : 0), 0, 5);
  syncRangePair("buzzer-level", "buzzer-level-value", buzzerLevel, 0, 5, 1);
  $("pid-kp").value = Number(state.settings.kp).toFixed(2);
  $("pid-ki").value = Number(state.settings.ki).toFixed(2);
  $("pid-kd").value = Number(state.settings.kd).toFixed(2);
  $("setup-ap-ssid").value = state.settings.setupApSsid || "";
  $("setup-ap-password").value = state.settings.setupApPassword || "";
}

function renderRunControls() {
  if (!els.activeProfile) return;
  const selectedIndex = state.settings ? Number(state.settings.selectedProfileIndex || 0) :
    (state.telemetry ? Number(state.telemetry.selectedProfileIndex || 0) : 0);
  els.activeProfile.innerHTML = "";
  state.profiles.forEach(profile => {
    const opt = document.createElement("option");
    opt.value = profile.index;
    opt.textContent = `P${profile.slot} ${profile.name}`;
    els.activeProfile.appendChild(opt);
  });
  els.activeProfile.value = selectedIndex;
  const profile = state.profiles[selectedIndex];
  setText(els.profileActive, profile ? `P${profile.slot} ${profile.name}` : "Profile -");
}

function renderProfiles() {
  els.profileTabs.innerHTML = "";
  state.profiles.forEach(profile => {
    const btn = document.createElement("button");
    btn.className = `profile-tab ${profile.index === state.currentProfile ? "active" : ""}`;
    btn.type = "button";
    btn.textContent = `P${profile.slot} ${profile.name}`;
    btn.addEventListener("click", () => {
      state.currentProfile = profile.index;
      renderProfiles();
    });
    els.profileTabs.appendChild(btn);
  });
  const profile = state.profiles[state.currentProfile];
  if (!profile) return;
  writeProfileForm(profile);
  setText(els.profileCurveName, `P${profile.slot} ${profile.name}`);
  renderProfileChart(profile);
}

function writeProfileForm(profile) {
  $("profile-name").value = profile.name;
  $("profile-cooling").value = profile.coolingProfile;
  $("preheat-temp").value = profile.preheatTempC;
  $("preheat-time").value = profile.preheatSeconds;
  $("soak-temp").value = profile.soakTempC;
  $("soak-time").value = profile.soakSeconds;
  $("reflow-temp").value = profile.reflowTempC;
  $("reflow-time").value = profile.reflowSeconds;
}

function readProfileForm() {
  return normalizeProfileValues({
    name: $("profile-name").value.trim(),
    coolingProfile: $("profile-cooling").value,
    preheatTempC: $("preheat-temp").value,
    preheatSeconds: $("preheat-time").value,
    soakTempC: $("soak-temp").value,
    soakSeconds: $("soak-time").value,
    reflowTempC: $("reflow-temp").value,
    reflowSeconds: $("reflow-time").value
  });
}

function updateProfileDraftFromForm() {
  const profile = state.profiles[state.currentProfile];
  if (!profile) return;
  Object.assign(profile, readProfileForm());
  setText(els.profileCurveName, `P${profile.slot} ${profile.name}`);
  renderProfileChart(profile);
}

function renderProfileChart(profile) {
  if (!state.profileChart || !profile) return;
  const values = normalizeProfileValues(profile);
  const safe = state.settings ? numberOr(state.settings.safeTouchC, 45) : 45;
  const start = safe;
  const t1 = numberOr(values.preheatSeconds, limits.stageSeconds[0]);
  const t2 = t1 + numberOr(values.soakSeconds, limits.stageSeconds[0]);
  const t3 = t2 + numberOr(values.reflowSeconds, limits.stageSeconds[0]);
  const coolX = t3 + 90;
  const maxX = Math.max(120, coolX + 20);
  const maxY = Math.max(120, numberOr(values.reflowTempC, 0) + 20, safe + 40);
  state.profileChart.data.datasets[0].data = [
    { x: 0, y: start, stage: "Start" },
    { x: t1, y: numberOr(values.preheatTempC, preheatMinTempC()), stage: "Preheat" },
    { x: t2, y: numberOr(values.soakTempC, preheatMinTempC() + limits.stageGapC), stage: "Soak" },
    { x: t3, y: numberOr(values.reflowTempC, preheatMinTempC() + limits.stageGapC * 2), stage: "Reflow" },
    { x: coolX, y: safe, stage: "Cool" }
  ];
  state.profileChart.options.scales.x.min = 0;
  state.profileChart.options.scales.x.max = maxX;
  state.profileChart.options.scales.y.min = 0;
  state.profileChart.options.scales.y.max = maxY;
  state.profileChart.update();
}

function bindProfileChartDrag() {
  const chart = state.profileChart;
  if (!chart) return;
  const canvas = chart.canvas;
  canvas.addEventListener("pointerdown", event => {
    const points = chart.getElementsAtEventForMode(event, "nearest", { intersect: true }, false);
    if (!points.length) return;
    const index = points[0].index;
    if (index < 1 || index > 3) return;
    state.dragProfilePoint = index;
    canvas.setPointerCapture(event.pointerId);
    event.preventDefault();
  });
  canvas.addEventListener("pointermove", event => {
    if (state.dragProfilePoint < 0) return;
    updateProfileFromChartPointer(event);
  });
  const stopDrag = event => {
    if (state.dragProfilePoint >= 0) {
      state.dragProfilePoint = -1;
      try {
        canvas.releasePointerCapture(event.pointerId);
      } catch (_) {
      }
    }
  };
  canvas.addEventListener("pointerup", stopDrag);
  canvas.addEventListener("pointercancel", stopDrag);
  canvas.addEventListener("pointerleave", stopDrag);
}

function updateProfileFromChartPointer(event) {
  const chart = state.profileChart;
  const idx = state.dragProfilePoint;
  const profile = state.profiles[state.currentProfile];
  if (!chart || !profile || idx < 1) return;
  const rect = chart.canvas.getBoundingClientRect();
  const area = chart.chartArea;
  const px = clamp(event.clientX - rect.left, area.left, area.right);
  const py = clamp(event.clientY - rect.top, area.top, area.bottom);
  const x = chart.scales.x.getValueForPixel(px);
  const y = chart.scales.y.getValueForPixel(py);
  if (!Number.isFinite(x) || !Number.isFinite(y)) return;
  const values = readProfileForm();
  if (idx === 1) {
    values.preheatSeconds = clamp(roundStep(x, 5), limits.stageSeconds[0], limits.stageSeconds[1]);
    values.preheatTempC = clamp(Math.round(y), preheatMinTempC(), limits.preheatTempC[1]);
  } else if (idx === 2) {
    values.soakSeconds = clamp(roundStep(x - values.preheatSeconds, 5), limits.stageSeconds[0], limits.stageSeconds[1]);
    values.soakTempC = clamp(Math.round(y), values.preheatTempC + limits.stageGapC, limits.soakTempC[1]);
  } else if (idx === 3) {
    values.reflowSeconds = clamp(roundStep(x - values.preheatSeconds - values.soakSeconds, 5), limits.stageSeconds[0], limits.stageSeconds[1]);
    values.reflowTempC = clamp(Math.round(y), values.soakTempC + limits.stageGapC, limits.reflowTempC[1]);
  }
  Object.assign(profile, normalizeProfileValues(values));
  writeProfileForm(profile);
  renderProfileChart(profile);
}

function renderTelemetry(data) {
  state.telemetry = data;
  const sensors = data.sensors || {};
  const heater = data.heater || {};
  const fan = data.fan || {};
  const boardFan = data.boardFan || {};
  setText(els.statePill, data.state);
  els.statePill.className = `pill ${data.faultActive ? "bad" : (data.active ? "hot" : "good")}`;
  setText(els.profileActive, `P${Number(data.selectedProfileIndex) + 1} ${data.profileName || ""}`);
  if (els.activeProfile && document.activeElement !== els.activeProfile) {
    els.activeProfile.value = Number(data.selectedProfileIndex || 0);
  }
  setText(els.plateTemp, fmtTemp(sensors.plateC, sensors.plateOk, 1));
  setText(els.targetTemp, fmtTemp(data.targetC, true, 0));
  setText(els.ambientTemp, fmtTemp(sensors.ambientC, sensors.ambientOk, 1));
  setText(els.stageName, data.state);
  setText(els.timeLeft, `${data.timeLeftSeconds || 0}s`);
  setText(els.faultName, data.fault || "None");
  setText(els.ssrState, fmtBool(heater.outputOn));
  setText(els.ssrDuty, `${Math.round(heater.dutyPercent || 0)}% duty`);
  setText(els.fanState, fmtBool(fan.on));
  setText(els.fanRpm, `${fan.rpm || 0} RPM, ${fan.speedPercent || 0}%`);
  setText(els.boardFanState, boardFan.present ? fmtBool(boardFan.on) : "N/A");
  setText(els.boardFanRpm, boardFan.present ? `${boardFan.rpm || 0} RPM, ${boardFan.speedPercent || 0}%` : "--");
  setText(els.safetyState, data.cooldownLocked ? "Locked" : "Ready");
  setText(els.cooldownLeft, data.cooldownRemainingSeconds ? `${data.cooldownRemainingSeconds}s forced fan` : "Safe checks active");
  $("start-btn").disabled = !data.canStart;
  $("abort-btn").disabled = !data.canAbort;
  const powerSafe = data.state === "Idle" && !data.cooldownLocked;
  $("reboot-btn").disabled = !powerSafe;
  $("factory-btn").disabled = !powerSafe;
  addLivePoint(data);
  maybePromptFaultAcknowledge(data);
}

async function maybePromptFaultAcknowledge(data) {
  if (data.state !== "Fault") {
    state.faultAckPromptKey = "";
    return;
  }
  if (!data.canAcknowledge || state.modalOpen) return;
  const key = `${data.fault || "Fault"}:${data.settingsRevision || 0}`;
  if (state.faultAckPromptKey === key) return;
  state.faultAckPromptKey = key;
  const ok = await showConfirm({
    title: "Hardware Fault Ready To Acknowledge",
    message: `${data.fault || "A hardware fault"} has cooled to a safe state. Acknowledge the fault to return ReflowDesk to idle.`,
    confirmText: "Acknowledge",
    danger: false
  });
  if (ok) {
    await command("/api/reflow/ack", "Fault acknowledged.");
  }
}

function addLivePoint(data) {
  if (!state.liveChart) return;
  if (data.active && !state.runBaseMs) {
    state.runBaseMs = data.timeMs;
    clearLiveChart();
  }
  if (!data.active && data.state === "Idle") {
    state.runBaseMs = 0;
  }
  const sensors = data.sensors || {};
  const labelSeconds = state.runBaseMs ? Math.round((data.timeMs - state.runBaseMs) / 1000) : Math.round(data.timeMs / 1000);
  const chart = state.liveChart;
  chart.data.datasets[0].data.push({ x: labelSeconds, y: sensors.plateOk ? Number(sensors.plateC) : null });
  chart.data.datasets[1].data.push({ x: labelSeconds, y: Number(data.targetC || 0) });
  chart.data.datasets[2].data.push({ x: labelSeconds, y: sensors.ambientOk ? Number(sensors.ambientC) : null });
  state.liveSamples += 1;
  while (chart.data.datasets[0].data.length > 600) {
    chart.data.datasets.forEach(ds => ds.data.shift());
  }
  chart.update("none");
}

function clearLiveChart() {
  if (!state.liveChart) return;
  state.liveChart.data.datasets.forEach(ds => ds.data = []);
  state.liveSamples = 0;
  state.liveChart.update("none");
}

function connectWebSocket() {
  if (!state.token) return;
  clearTimeout(state.reconnectTimer);
  const proto = location.protocol === "https:" ? "wss" : "ws";
  state.ws = new WebSocket(`${proto}://${location.hostname}:81/`);
  setWsStatus("Connecting", "");
  state.ws.addEventListener("open", () => {
    setWsStatus("Authenticating", "");
    state.ws.send(JSON.stringify({ type: "auth", token: state.token }));
  });
  state.ws.addEventListener("message", event => {
    let msg;
    try {
      msg = JSON.parse(event.data);
    } catch (_) {
      return;
    }
    if (msg.type === "auth_ok") {
      state.wsReady = true;
      setWsStatus("Live", "good");
    } else if (msg.type === "auth_fail") {
      setWsStatus("Auth Fail", "bad");
      toast("WebSocket authentication failed.", "error");
    } else if (msg.type === "telemetry") {
      renderTelemetry(msg.data || {});
    } else if (msg.type === "event") {
      toast(msg.message, msg.level || "info");
      logEvent(msg.message, msg.level || "info", Date.now());
      refreshAfterEvent(msg.message || "");
    }
  });
  state.ws.addEventListener("close", () => {
    state.wsReady = false;
    setWsStatus("Offline", "bad");
    state.reconnectTimer = setTimeout(connectWebSocket, 2500);
  });
  state.ws.addEventListener("error", () => {
    setWsStatus("Error", "bad");
  });
}

function setWsStatus(label, cls) {
  setText(els.wsPill, `WS: ${label}`);
  els.wsPill.className = `pill ${cls}`;
}

async function refreshAfterEvent(message) {
  if (/settings|profile|factory reset/i.test(message)) {
    try {
      await Promise.all([loadSettings(), loadProfiles(true), loadDeviceInfo()]);
      renderAll();
    } catch (_) {
    }
  }
}

function applySavedSnapshot(result, preferredProfileIndex = state.currentProfile) {
  if (result.settings) {
    state.settings = result.settings;
  }
  if (Array.isArray(result.profiles)) {
    state.profiles = result.profiles;
  }
  if (state.profiles.length) {
    state.currentProfile = clamp(preferredProfileIndex, 0, state.profiles.length - 1);
  }
  renderAll();
}

async function saveSettings() {
  const ledBrightness = syncRangePair("led-brightness-slider", "led-brightness", $("led-brightness").value, 0, 100, 5);
  const buzzerLevel = syncRangePair("buzzer-level", "buzzer-level-value", $("buzzer-level").value, 0, 5, 1);
  const body = {
    safeTouchC: Number($("safe-touch").value),
    safetyCutoffC: Number($("safety-cutoff").value),
    ledBrightness,
    buzzerLevel,
    kp: Number($("pid-kp").value),
    ki: Number($("pid-ki").value),
    kd: Number($("pid-kd").value),
    setupApSsid: $("setup-ap-ssid").value.trim(),
    setupApPassword: $("setup-ap-password").value
  };
  const result = await api("/api/settings", { method: "PUT", body });
  applySavedSnapshot(result);
  toast("Settings saved.", "success");
}

async function changePin() {
  const currentPin = $("current-pin").value.trim();
  const pin = $("new-pin").value.trim();
  const confirmPin = $("confirm-pin").value.trim();
  if (pin.length < 4 || pin.length > 32) {
    throw new Error("New PIN must be 4 to 32 characters.");
  }
  if (pin !== confirmPin) {
    throw new Error("New PIN and confirmation do not match.");
  }
  await api("/api/auth/setup", { method: "POST", body: { currentPin, pin } });
  if (state.ws) {
    state.ws.close();
    state.ws = null;
  }
  clearTimeout(state.reconnectTimer);
  state.wsReady = false;
  sessionStorage.removeItem(tokenKey);
  state.token = "";
  $("current-pin").value = "";
  $("new-pin").value = "";
  $("confirm-pin").value = "";
  showAuth(false, "Web access PIN changed. Please sign in again with the new PIN.");
}

async function saveActiveProfileSelection() {
  if (!els.activeProfile) return;
  const index = Number(els.activeProfile.value);
  const result = await api("/api/settings", { method: "PUT", body: { selectedProfileIndex: index } });
  applySavedSnapshot(result, index);
  toast("Active profile selected.", "success");
}

async function saveProfile() {
  const profile = state.profiles[state.currentProfile];
  if (!profile) return;
  const index = state.currentProfile;
  const body = readProfileForm();
  const result = await api(`/api/profiles/${profile.slot}`, { method: "PUT", body });
  applySavedSnapshot(result, index);
  toast("Profile saved.", "success");
}

async function startReflow() {
  const profileIndex = els.activeProfile ? Number(els.activeProfile.value) :
    (state.settings ? Number(state.settings.selectedProfileIndex || 0) : 0);
  await api("/api/reflow/start", { method: "POST", body: { profileIndex } });
  toast("Reflow started.", "success");
}

async function command(path, success) {
  try {
    await api(path, { method: "POST", body: {} });
    toast(success, "success");
    return true;
  } catch (err) {
    toast(err.message, "error");
    return false;
  }
}

async function confirmedCommand(options) {
  const ok = await showConfirm(options);
  if (!ok) return;
  const succeeded = await command(options.path, options.success);
  if (succeeded && options.homeAfter) {
    clearSavedPage();
  }
}

function uploadOta(event) {
  event.preventDefault();
  const file = $("ota-file").files[0];
  if (!file) {
    toast("Select a firmware.bin file first.", "warning");
    return;
  }
  const form = new FormData();
  form.append("firmware", file);
  const xhr = new XMLHttpRequest();
  xhr.open("POST", "/api/ota/firmware");
  xhr.setRequestHeader("X-Reflow-Token", state.token);
  xhr.upload.onprogress = evt => {
    if (evt.lengthComputable) {
      els.otaProgress.style.width = `${Math.round((evt.loaded / evt.total) * 100)}%`;
    }
  };
  xhr.onload = () => {
    let response = {};
    try {
      response = JSON.parse(xhr.responseText || "{}");
    } catch (_) {
    }
    if (xhr.status >= 200 && xhr.status < 300) {
      clearSavedPage();
      setText(els.otaNote, response.message || "Firmware uploaded.");
      toast(response.message || "Firmware uploaded.", "success");
    } else {
      setText(els.otaNote, response.message || "OTA failed.");
      toast(response.message || "OTA failed.", "error");
    }
  };
  xhr.onerror = () => {
    setText(els.otaNote, "OTA upload failed.");
    toast("OTA upload failed.", "error");
  };
  els.otaProgress.style.width = "0%";
  xhr.send(form);
}

function pageFromHash() {
  const page = (window.location.hash || "").replace(/^#/, "");
  return validPages.has(page) ? page : "dashboard";
}

function clearSavedPage() {
  history.replaceState(null, "", `${window.location.pathname}${window.location.search}`);
}

function switchPage(page, updateLocation = true) {
  if (!validPages.has(page)) {
    page = "dashboard";
  }
  document.querySelectorAll(".page").forEach(el => el.classList.toggle("active", el.id === `page-${page}`));
  document.querySelectorAll(".nav-btn").forEach(btn => btn.classList.toggle("active", btn.dataset.page === page));
  const titles = { dashboard: "Reflow Process", profiles: "Reflow Profiles", settings: "Settings", device: "Device", logs: "Logs" };
  setText(els.pageTitle, titles[page] || "ReflowDesk");
  if (updateLocation && window.location.hash !== `#${page}`) {
    history.replaceState(null, "", `#${page}`);
  }
  els.sidebar.classList.remove("open");
}

function bindProfileFormUpdates() {
  ["profile-name", "profile-cooling", "preheat-temp", "preheat-time", "soak-temp", "soak-time", "reflow-temp", "reflow-time"]
    .forEach(id => {
      const el = $(id);
      el.addEventListener("input", updateProfileDraftFromForm);
      el.addEventListener("change", updateProfileDraftFromForm);
    });
}

function bindEvents() {
  els.authForm.addEventListener("submit", submitAuth);
  $("logout-btn").addEventListener("click", async () => {
    try {
      await api("/api/auth/logout", { method: "POST", body: {} });
    } catch (_) {
    }
    sessionStorage.removeItem(tokenKey);
    state.token = "";
    if (state.ws) state.ws.close();
    showAuth(false);
  });
  document.querySelectorAll(".nav-btn").forEach(btn => btn.addEventListener("click", () => switchPage(btn.dataset.page)));
  $("menu-btn").addEventListener("click", () => els.sidebar.classList.toggle("open"));
  $("start-btn").addEventListener("click", () => startReflow().catch(err => toast(err.message, "error")));
  els.activeProfile.addEventListener("change", () => saveActiveProfileSelection().catch(err => toast(err.message, "error")));
  $("abort-btn").addEventListener("click", () => confirmedCommand({
    title: "Emergency Stop",
    message: "Abort the active reflow process and start forced cooling now?",
    confirmText: "Abort Reflow",
    danger: true,
    path: "/api/reflow/abort",
    success: "Abort requested."
  }));
  document.querySelectorAll("[data-save-settings], #save-settings-btn")
    .forEach(btn => btn.addEventListener("click", () => saveSettings().catch(err => toast(err.message, "error"))));
  $("change-pin-btn").addEventListener("click", () => changePin().catch(err => toast(err.message, "error")));
  $("led-brightness-slider").addEventListener("input", event => {
    syncRangePair("led-brightness-slider", "led-brightness", event.target.value, 0, 100, 5);
  });
  $("led-brightness").addEventListener("input", event => {
    const next = quantize(event.target.value, 0, 100, 5);
    const slider = $("led-brightness-slider");
    slider.value = next;
    setRangeVisual(slider);
  });
  $("led-brightness").addEventListener("change", event => {
    syncRangePair("led-brightness-slider", "led-brightness", event.target.value, 0, 100, 5);
  });
  $("buzzer-level").addEventListener("input", event => {
    syncRangePair("buzzer-level", "buzzer-level-value", event.target.value, 0, 5, 1);
  });
  els.themeToggle.addEventListener("click", () => applyTheme(state.theme === "light" ? "dark" : "light"));
  $("save-profile-btn").addEventListener("click", () => saveProfile().catch(err => toast(err.message, "error")));
  $("ota-form").addEventListener("submit", uploadOta);
  $("reboot-btn").addEventListener("click", () => confirmedCommand({
    title: "Reboot Device",
    message: "Restart ReflowDesk now?",
    confirmText: "Reboot",
    danger: false,
    path: "/api/device/reboot",
    success: "Reboot requested.",
    homeAfter: true
  }));
  $("factory-btn").addEventListener("click", () => confirmedCommand({
    title: "Factory Reset",
    message: "Reset ReflowDesk settings and clear the web PIN?",
    confirmText: "Factory Reset",
    danger: true,
    path: "/api/device/factory-reset",
    success: "Factory reset requested.",
    homeAfter: true
  }));
  $("clear-log-btn").addEventListener("click", () => els.eventList.innerHTML = "");
  els.modalCancel.addEventListener("click", () => closeConfirm(false));
  els.modalConfirm.addEventListener("click", () => closeConfirm(true));
  els.modal.addEventListener("click", event => {
    if (event.target === els.modal) closeConfirm(false);
  });
  bindProfileFormUpdates();
}

bindEvents();
applyTheme(state.theme);
window.addEventListener("hashchange", () => switchPage(pageFromHash(), false));
checkAuth();
