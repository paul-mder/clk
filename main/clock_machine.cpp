#include "nvs.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "clock_machine.hpp"
#include "clock_machine_states.hpp"

static const char *TAG = "clock_machine";

// HTML Seite fuer Web-Interface
static const char* HTML_PAGE = R"rawhtml2(
<!DOCTYPE html>
<html lang="de">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no">
<title>CLK</title>
<link href="https://fonts.googleapis.com/css2?family=DM+Serif+Display&family=DM+Sans:wght@300;400;500&display=swap" rel="stylesheet">
<style>
*{box-sizing:border-box;margin:0;padding:0;-webkit-tap-highlight-color:transparent}
:root{--bg:#0a0a0f;--surface:#13131a;--surface2:#1c1c26;--accent:#ff6b35;--accent2:#ff9f1c;--text:#f0eff4;--muted:#6b6b80;--success:#34d399;--border:#2a2a38}
body{background:var(--bg);color:var(--text);font-family:'DM Sans',sans-serif;min-height:100vh;overflow-x:hidden}
.header{padding:24px 20px 12px;display:flex;align-items:center;justify-content:space-between}
.header h1{font-family:'DM Serif Display',serif;font-size:28px;background:linear-gradient(135deg,var(--accent),var(--accent2));-webkit-background-clip:text;-webkit-text-fill-color:transparent}
.current-time{font-size:14px;color:var(--muted);font-variant-numeric:tabular-nums}
.section{padding:0 16px 24px}
.section-label{font-size:11px;letter-spacing:2px;text-transform:uppercase;color:var(--muted);margin-bottom:12px;padding-left:4px}
.today-card{background:var(--surface);border-radius:20px;padding:24px;border:1px solid var(--border)}
.today-row{display:flex;align-items:center;justify-content:space-between;margin-bottom:20px}
.today-label{font-size:18px;font-weight:500}
.today-time{font-family:'DM Serif Display',serif;font-size:48px;text-align:center;color:var(--text);margin-bottom:20px;letter-spacing:-1px}
.toggle{width:56px;height:30px;background:var(--surface2);border-radius:15px;position:relative;cursor:pointer;transition:background .3s;border:1px solid var(--border)}
.toggle.on{background:var(--accent)}
.toggle::after{content:'';position:absolute;width:24px;height:24px;background:white;border-radius:12px;top:2px;left:2px;transition:transform .3s;box-shadow:0 2px 6px rgba(0,0,0,.4)}
.toggle.on::after{transform:translateX(26px)}
.drum-container{display:flex;gap:16px;justify-content:center;align-items:center}
.drum-sep{font-family:'DM Serif Display',serif;font-size:36px;color:var(--muted);padding-bottom:4px}
.drum-wrap{position:relative;width:80px;height:160px;overflow:hidden;border-radius:16px;background:var(--surface2);border:1px solid var(--border)}
.drum-wrap::before,.drum-wrap::after{content:'';position:absolute;left:0;right:0;height:56px;z-index:2;pointer-events:none}
.drum-wrap::before{top:0;background:linear-gradient(to bottom,var(--surface2),transparent)}
.drum-wrap::after{bottom:0;background:linear-gradient(to top,var(--surface2),transparent)}
.drum-selector{position:absolute;top:50%;left:4px;right:4px;height:48px;transform:translateY(-50%);background:var(--surface);border-radius:10px;border:1px solid var(--accent);z-index:1;pointer-events:none}
.drum-list{position:absolute;width:100%;will-change:transform}
.drum-item{height:48px;display:flex;align-items:center;justify-content:center;font-family:'DM Serif Display',serif;font-size:28px;color:var(--muted);transition:color .1s;user-select:none}
.drum-item.active{color:var(--text)}
.days-grid{display:grid;grid-template-columns:repeat(7,1fr);gap:8px}
.day-card{background:var(--surface);border-radius:14px;padding:12px 6px;text-align:center;border:1px solid var(--border);cursor:pointer;transition:all .2s;position:relative;overflow:hidden;-webkit-touch-callout:none;user-select:none}
.day-card.active{background:rgba(255,107,53,.12);border-color:var(--accent)}
.day-card.active .day-name{color:var(--accent)}
.day-card.active .day-time{color:var(--text)}
.day-name{font-size:11px;font-weight:500;color:var(--muted);letter-spacing:.5px;margin-bottom:6px}
.day-time{font-family:'DM Serif Display',serif;font-size:14px;color:var(--muted)}
.day-dot{width:5px;height:5px;border-radius:50%;background:var(--accent);margin:4px auto 0;opacity:0}
.day-card.active .day-dot{opacity:1}
.day-card:active{transform:scale(.95)}
.ripple{position:absolute;border-radius:50%;background:rgba(255,107,53,.3);animation:ripple .6s ease-out forwards;pointer-events:none}
@keyframes ripple{from{transform:scale(0);opacity:1}to{transform:scale(4);opacity:0}}
.modal-overlay{position:fixed;inset:0;background:rgba(0,0,0,.7);z-index:100;display:flex;align-items:flex-end;opacity:0;pointer-events:none;transition:opacity .3s;backdrop-filter:blur(4px)}
.modal-overlay.open{opacity:1;pointer-events:all}
.modal{background:var(--surface);border-radius:28px 28px 0 0;padding:24px 24px 48px;width:100%;transform:translateY(100%);transition:transform .3s cubic-bezier(.4,0,.2,1)}
.modal-overlay.open .modal{transform:translateY(0)}
.modal-title{font-family:'DM Serif Display',serif;font-size:22px;margin-bottom:8px}
.modal-subtitle{color:var(--muted);font-size:14px;margin-bottom:28px}
.modal-actions{display:grid;grid-template-columns:1fr 1fr;gap:12px;margin-top:28px}
.btn{padding:16px;border-radius:14px;border:none;font-size:16px;font-family:'DM Sans',sans-serif;font-weight:500;cursor:pointer;transition:all .15s}
.btn-cancel{background:var(--surface2);color:var(--muted)}
.btn-save{background:var(--accent);color:white}
.btn:active{transform:scale(.97)}
.status-bar{text-align:center;padding:8px;font-size:12px;color:var(--muted)}
</style>
</head>
<body>
<div class="header">
  <h1>CLK</h1>
  <div class="current-time" id="ct">--:--</div>
</div>

<div class="section">
  <div class="section-label">Heute</div>
  <div class="today-card">
    <div class="today-row">
      <span class="today-label" id="today-day">Montag</span>
      <div class="toggle" id="today-toggle" onclick="toggleToday()"></div>
    </div>
    <div class="today-time" id="today-time-display">07:00</div>
    <div class="drum-container" id="today-drums">
      <div class="drum-wrap" id="drum-h"></div>
      <div class="drum-sep">:</div>
      <div class="drum-wrap" id="drum-m"></div>
    </div>
  </div>
</div>

<div class="section">
  <div class="section-label">Wochentage</div>
  <div class="days-grid" id="days-grid"></div>
</div>

<div class="status-bar" id="status">Verbinde...</div>

<div class="modal-overlay" id="modal">
  <div class="modal">
    <div class="modal-title" id="modal-title">Weckzeit</div>
    <div class="modal-subtitle" id="modal-sub">Weckzeit fuer diesen Tag</div>
    <div class="drum-container">
      <div class="drum-wrap" id="mdrum-h"></div>
      <div class="drum-sep">:</div>
      <div class="drum-wrap" id="mdrum-m"></div>
    </div>
    <div class="modal-actions">
      <button class="btn btn-cancel" onclick="closeModal()">Abbrechen</button>
      <button class="btn btn-save" onclick="saveModal()">Speichern</button>
    </div>
  </div>
</div>

<script>
const DAYS=['Mo','Di','Mi','Do','Fr','Sa','So'];
const DAYNAMES=['Montag','Dienstag','Mittwoch','Donnerstag','Freitag','Samstag','Sonntag'];
let state={alarms:[{h:7,m:0,on:false},{h:7,m:0,on:false},{h:7,m:0,on:false},{h:7,m:0,on:false},{h:7,m:0,on:false},{h:8,m:0,on:false},{h:9,m:0,on:false}],todayOn:false,todayH:7,todayM:0};
let modalDay=-1;
let drumH,drumM,mdrumH,mdrumM;

function pad(n){return String(n).padStart(2,'0')}

// --- Drum Picker ---
function makeDrum(el,max,initial,onChange){
  const items=[];
  for(let i=0;i<max;i++)items.push(i);
  let current=initial;
  let startY=0,startOffset=0,offset=0,dragging=false;
  const ITEM_H=48;

  el.innerHTML='<div class="drum-selector"></div><div class="drum-list"></div>';
  const list=el.querySelector('.drum-list');
  // Render with padding items
  const pad3=3;
  for(let i=-pad3;i<max+pad3;i++){
    const v=((i%max)+max)%max;
    const d=document.createElement('div');
    d.className='drum-item';
    d.textContent=pad(v);
    list.appendChild(d);
  }

  function setOffset(o,animate){
    offset=Math.round(o/ITEM_H)*ITEM_H;
    if(animate){list.style.transition='transform .2s cubic-bezier(.4,0,.2,1)'}
    else{list.style.transition='none'}
    const center=el.offsetHeight/2-ITEM_H/2;
    list.style.transform=`translateY(${center-offset}px)`;
    current=((Math.round(offset/ITEM_H)%max)+max)%max;
    list.querySelectorAll('.drum-item').forEach((d,i)=>{
      d.classList.toggle('active',((i-pad3)%max+max)%max===current);
    });
    onChange(current);
  }

  setOffset(initial*ITEM_H,false);

  el.addEventListener('touchstart',e=>{
    dragging=true;list.style.transition='none';
    startY=e.touches[0].clientY;startOffset=offset;
  },{passive:true});
  el.addEventListener('touchmove',e=>{
    if(!dragging)return;
    const dy=startY-e.touches[0].clientY;
    const raw=startOffset+dy;
    const center=el.offsetHeight/2-ITEM_H/2;
    list.style.transform=`translateY(${center-raw}px)`;
    offset=raw;
    current=((Math.round(raw/ITEM_H)%max)+max)%max;
    list.querySelectorAll('.drum-item').forEach((d,i)=>{
      d.classList.toggle('active',((i-pad3)%max+max)%max===current);
    });
    onChange(current);
  },{passive:true});
  el.addEventListener('touchend',()=>{dragging=false;setOffset(offset,true);});

  return{get:()=>current,set:(v,anim=true)=>setOffset(v*ITEM_H,anim)};
}

function initDrums(){
  drumH=makeDrum(document.getElementById('drum-h'),24,state.todayH,v=>{state.todayH=v;document.getElementById('today-time-display').textContent=pad(v)+':'+pad(state.todayM)});
  drumM=makeDrum(document.getElementById('drum-m'),60,state.todayM,v=>{state.todayM=v;document.getElementById('today-time-display').textContent=pad(state.todayH)+':'+pad(v)});
  mdrumH=makeDrum(document.getElementById('mdrum-h'),24,7,()=>{});
  mdrumM=makeDrum(document.getElementById('mdrum-m'),60,0,()=>{});
}

// --- Today ---
function toggleToday(){
  state.todayOn=!state.todayOn;
  document.getElementById('today-toggle').classList.toggle('on',state.todayOn);
  saveToday();
}

function saveToday(){
  fetch('/setalarm?hour='+state.todayH+'&minute='+state.todayM+'&enable='+(state.todayOn?1:0))
    .then(()=>setStatus(state.todayOn?'Wecker aktiv: '+pad(state.todayH)+':'+pad(state.todayM):'Wecker aus'));
}

// Drum changes trigger auto-save with debounce
let saveTimer=null;
function scheduleSave(){clearTimeout(saveTimer);saveTimer=setTimeout(saveToday,800)}

// --- Days Grid ---
function buildDaysGrid(){
  const grid=document.getElementById('days-grid');
  grid.innerHTML='';
  DAYS.forEach((d,i)=>{
    const a=state.alarms[i];
    const card=document.createElement('div');
    card.className='day-card'+(a.on?' active':'');
    card.innerHTML=`<div class="day-name">${d}</div><div class="day-time">${pad(a.h)}:${pad(a.m)}</div><div class="day-dot"></div>`;
    let pressTimer=null;
    card.addEventListener('touchstart',e=>{
      pressTimer=setTimeout(()=>{pressTimer=null;openModal(i);},500);
      addRipple(card,e);
    },{passive:true});
    card.addEventListener('touchend',()=>{
      if(pressTimer){clearTimeout(pressTimer);pressTimer=null;toggleDay(i);}
    });
    card.addEventListener('touchmove',()=>{if(pressTimer){clearTimeout(pressTimer);pressTimer=null;}},{passive:true});
    grid.appendChild(card);
  });
}

function addRipple(el,e){
  const r=document.createElement('div');
  r.className='ripple';
  const rect=el.getBoundingClientRect();
  const size=Math.max(rect.width,rect.height);
  r.style.cssText=`width:${size}px;height:${size}px;left:${e.touches[0].clientX-rect.left-size/2}px;top:${e.touches[0].clientY-rect.top-size/2}px`;
  el.appendChild(r);
  setTimeout(()=>r.remove(),600);
}

function toggleDay(i){
  state.alarms[i].on=!state.alarms[i].on;
  saveDay(i);buildDaysGrid();
}

function saveDay(i){
  const a=state.alarms[i];
  fetch('/setalarm?day='+i+'&hour='+a.h+'&minute='+a.m+'&enable='+(a.on?1:0));
}

// --- Modal ---
function openModal(i){
  modalDay=i;
  const a=state.alarms[i];
  document.getElementById('modal-title').textContent=DAYNAMES[i];
  document.getElementById('modal-sub').textContent='Weckzeit fuer '+DAYNAMES[i]+' einstellen';
  setTimeout(()=>{mdrumH.set(a.h,false);mdrumM.set(a.m,false);},50);
  document.getElementById('modal').classList.add('open');
}

function closeModal(){document.getElementById('modal').classList.remove('open');}

function saveModal(){
  if(modalDay<0)return;
  state.alarms[modalDay].h=mdrumH.get();
  state.alarms[modalDay].m=mdrumM.get();
  saveDay(modalDay);
  buildDaysGrid();
  closeModal();
}

document.getElementById('modal').addEventListener('touchstart',e=>{
  if(e.target===document.getElementById('modal'))closeModal();
});

// --- Status & Time Update ---
function setStatus(msg){document.getElementById('status').textContent=msg;}

function updateStatus(){
  fetch('/status').then(r=>r.json()).then(d=>{
    document.getElementById('ct').textContent=pad(d.hour)+':'+pad(d.minute);
    const today=new Date().getDay();
    const todayIdx=today===0?6:today-1;
    document.getElementById('today-day').textContent=DAYNAMES[todayIdx];
    if(d.alarm_set!==state.todayOn){state.todayOn=d.alarm_set;document.getElementById('today-toggle').classList.toggle('on',state.todayOn);}
    if(!state._init){
      state._init=true;
      state.todayH=d.alarm_hour;state.todayM=d.alarm_minute;
      drumH.set(d.alarm_hour,false);drumM.set(d.alarm_minute,false);
      document.getElementById('today-time-display').textContent=pad(d.alarm_hour)+':'+pad(d.alarm_minute);
    }
    setStatus('Verbunden');
  }).catch(()=>setStatus('Verbindungsfehler'));
}

// Drum save on change
function rewireDrums(){
  drumH=makeDrum(document.getElementById('drum-h'),24,state.todayH,v=>{state.todayH=v;document.getElementById('today-time-display').textContent=pad(v)+':'+pad(state.todayM);scheduleSave();});
  drumM=makeDrum(document.getElementById('drum-m'),60,state.todayM,v=>{state.todayM=v;document.getElementById('today-time-display').textContent=pad(state.todayH)+':'+pad(v);scheduleSave();});
}

window.onload=()=>{
  initDrums();rewireDrums();
  buildDaysGrid();
  updateStatus();
  setInterval(updateStatus,10000);
};
</script>
</body>
</html>
)rawhtml2";

ClockMachine::ClockMachine(RotaryEncoder* encoder_ref) {
    if (readNVSValues() == ESP_ERR_NVS_NOT_FOUND) {
        writeNVSDefaultValues();
        ESP_ERROR_CHECK(readNVSValues());
    }

    wifi_time.init();
    wifi_time.getTime(&stored_time);

    display.init();
    encoder = encoder_ref;

    display.updateContent(D_E_ALARM_TIME, &alarm_time, D_A_OFF);
    last_wifi_connected_status = !wifi_time.isWifiConnected();
    last_interaction_us = esp_timer_get_time();

    state = static_cast<ClockState*>(new TimeState());
    state->enter(this);

    startWebServer();
}

esp_err_t ClockMachine::readNVSValues() {
    nvs_handle_t NVS_handle;
    esp_err_t err = nvs_open(NVS_STORAGE, NVS_READONLY, &NVS_handle);
    if (err != ESP_OK) return err;
    err = nvs_get_u8(NVS_handle, NVS_ALARM_HOUR, &alarm_time.hour);
    if (err != ESP_OK) { nvs_close(NVS_handle); return err; }
    err = nvs_get_u8(NVS_handle, NVS_ALARM_MINUTE, &alarm_time.minute);
    nvs_close(NVS_handle);
    return err;
}

void ClockMachine::writeNVSDefaultValues() {
    alarm_time.hour = 7;
    alarm_time.minute = 0;
    saveAlarmTimeInNVS();
}

void ClockMachine::saveAlarmTimeInNVS() {
    nvs_handle_t NVS_handle;
    ESP_ERROR_CHECK(nvs_open(NVS_STORAGE, NVS_READWRITE, &NVS_handle));
    ESP_ERROR_CHECK(nvs_set_u8(NVS_handle, NVS_ALARM_HOUR, alarm_time.hour));
    ESP_ERROR_CHECK(nvs_set_u8(NVS_handle, NVS_ALARM_MINUTE, alarm_time.minute));
    nvs_close(NVS_handle);
}

void ClockMachine::setState(ClockState& newState) {
    active_timer_us = 0;
    state->exit(this);
    state = &newState;
    state->enter(this);
}

void ClockMachine::checkTimeUpdate(void) {
    clock_time_t current_time;
    wifi_time.getTime(&current_time);
    if ((current_time.hour != stored_time.hour) || (current_time.minute != stored_time.minute)) {
        stored_time.hour = current_time.hour;
        stored_time.minute = current_time.minute;
        if (!display_is_off)
            display.updateContent(D_E_TIME, &stored_time, D_A_ON);
        time_has_changed = true;
    } else {
        time_has_changed = false;
    }
}

clock_time_t ClockMachine::getTimeToAlarm(clock_time_t current_time, clock_time_t alarm_time) {
    clock_time_t time_to_alarm;
    if (current_time.minute > alarm_time.minute) { alarm_time.minute += 60; current_time.hour++; }
    if (current_time.hour > alarm_time.hour) alarm_time.hour += 24;
    time_to_alarm.hour = alarm_time.hour - current_time.hour;
    time_to_alarm.minute = alarm_time.minute - current_time.minute;
    return time_to_alarm;
}

WifiTime* ClockMachine::getWifiTime() { return &wifi_time; }
Display* ClockMachine::getDisplay() { return &display; }
RotaryEncoder* ClockMachine::getEncoder() { return encoder; }

void ClockMachine::triggerTimer(uint16_t timer_ms) {
    active_timer_us = timer_ms * 1000;
    trigger_timestamp_us = esp_timer_get_time();
}

void ClockMachine::checkWifiStatus(bool force_update) {
    bool wifi_connected_status = wifi_time.isWifiConnected();
    if ((last_wifi_connected_status != wifi_connected_status) || force_update) {
        last_wifi_connected_status = wifi_connected_status;
        display_action_t wifi_action = wifi_connected_status ? D_A_ON : D_A_OFF;
        display.updateContent(D_E_WIFI_STATUS, wifi_action);
        // WebServer starten sobald WiFi verbunden
        if (wifi_connected_status && server == NULL)
            startWebServer();
    }
}

void ClockMachine::wakeDisplay() {
    if (display_is_off) {
        display_is_off = false;
        next_input_is_wakeup = true;
        display.setIncreasedBrightness(true);
        // Display neu zeichnen
        display.updateContent(D_E_TIME, &stored_time, D_A_ON);
        display_action_t action = is_alarm_set ? D_A_ON : D_A_OFF;
        display.updateContent(D_E_ALARM_TIME, &alarm_time, action);
        if (is_alarm_set) {
            clock_time_t bed_time = getTimeToAlarm(stored_time, alarm_time);
            display.updateContent(D_E_BED_TIME, &bed_time, D_A_ON);
        }
        checkWifiStatus(true);
    }
}

void ClockMachine::run() {
    // next_input_is_wakeup nur gueltig wenn Display gerade aufgewacht
    if (!display_is_off && next_input_is_wakeup) next_input_is_wakeup = false;
    // Display-Timeout prüfen
    int64_t now = esp_timer_get_time();
    if (!display_is_off && (now - last_interaction_us) > ((int64_t)DISPLAY_TIMEOUT_S * 1000000LL)) {
        display_is_off = true;
        display.setBrightness(0);
    }

    if (active_timer_us > 0 && (esp_timer_get_time() - trigger_timestamp_us) > active_timer_us) {
        active_timer_us = 0;
        state->timerExpired(this);
    } else {
        checkTimeUpdate();
        state->run(this);
    }
    checkWifiStatus(false);
}

void ClockMachine::buttonShortPressed() {
    last_interaction_us = esp_timer_get_time();
    if (display_is_off) { wakeDisplay(); return; }
    if (next_input_is_wakeup) { next_input_is_wakeup = false; return; }
    state->buttonShortPressed(this);
}

void ClockMachine::buttonLongPressed() {
    last_interaction_us = esp_timer_get_time();
    if (display_is_off) { wakeDisplay(); return; }
    if (next_input_is_wakeup) { next_input_is_wakeup = false; return; }
    state->buttonLongPressed(this);
}

void ClockMachine::encoderRotated(rotary_encoder_pos_t position, rotary_encoder_dir_t direction) {
    last_interaction_us = esp_timer_get_time();
    if (display_is_off) { wakeDisplay(); return; }
    if (next_input_is_wakeup) { next_input_is_wakeup = false; return; }
    state->encoderRotated(this, position, direction);
}

// ---- Web Server ----

esp_err_t ClockMachine::handleRoot(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, HTML_PAGE, strlen(HTML_PAGE));
    return ESP_OK;
}

esp_err_t ClockMachine::handleGetStatus(httpd_req_t *req) {
    ClockMachine* cm = (ClockMachine*)req->user_ctx;
    char json[128];
    snprintf(json, sizeof(json),
        "{\"hour\":%d,\"minute\":%d,\"alarm_set\":%s,\"alarm_hour\":%d,\"alarm_minute\":%d}",
        cm->stored_time.hour, cm->stored_time.minute,
        cm->is_alarm_set ? "true" : "false",
        cm->alarm_time.hour, cm->alarm_time.minute);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json, strlen(json));
    return ESP_OK;
}

esp_err_t ClockMachine::handleSetAlarm(httpd_req_t *req) {
    ClockMachine* cm = (ClockMachine*)req->user_ctx;
    char buf[64];
    int ret = httpd_req_get_url_query_str(req, buf, sizeof(buf));
    if (ret == ESP_OK) {
        char val[8];
        uint8_t hour = 7, minute = 0;
        bool enable = true;
        if (httpd_query_key_value(buf, "hour", val, sizeof(val)) == ESP_OK)
            hour = (uint8_t)atoi(val);
        if (httpd_query_key_value(buf, "minute", val, sizeof(val)) == ESP_OK)
            minute = (uint8_t)atoi(val);
        if (httpd_query_key_value(buf, "enable", val, sizeof(val)) == ESP_OK)
            enable = (atoi(val) == 1);

        cm->alarm_time.hour = hour;
        cm->alarm_time.minute = minute;
        cm->is_alarm_set = enable;
        cm->saveAlarmTimeInNVS();

        display_action_t action = enable ? D_A_ON : D_A_OFF;
        cm->display.updateContent(D_E_ALARM_TIME, &cm->alarm_time, action);
        if (enable) {
            clock_time_t bed_time = cm->getTimeToAlarm(cm->stored_time, cm->alarm_time);
            cm->display.updateContent(D_E_BED_TIME, &bed_time, D_A_ON);
        } else {
            cm->display.updateContent(D_E_BED_TIME, NULL, D_A_OFF);
        }
    }
    httpd_resp_send(req, "OK", 2);
    return ESP_OK;
}

void ClockMachine::startWebServer(void) {
    if (server != NULL) return;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t uri_root = { "/", HTTP_GET, handleRoot, this };
        httpd_uri_t uri_status = { "/status", HTTP_GET, handleGetStatus, this };
        httpd_uri_t uri_alarm = { "/setalarm", HTTP_GET, handleSetAlarm, this };
        httpd_register_uri_handler(server, &uri_root);
        httpd_register_uri_handler(server, &uri_status);
        httpd_register_uri_handler(server, &uri_alarm);
        ESP_LOGI(TAG, "WebServer gestartet auf http://clk.local");
    }
}

