#include <Arduino.h>
#include <WiFi.h>
#include "Portal.h"
#include "ConfigStore.h"

namespace {
  String urlDecode(const String& s) {
    String out; out.reserve(s.length());
    auto hex = [](char x)->int {
      if (x>='0'&&x<='9') return x-'0';
      if (x>='A'&&x<='F') return x-'A'+10;
      if (x>='a'&&x<='f') return x-'a'+10;
      return 0;
    };
    for (uint16_t i=0;i<s.length();i++){
      char c=s[i];
      if (c=='+') out+=' ';
      else if (c=='%' && i+2<s.length()){ out+=(char)((hex(s[i+1])<<4)|hex(s[i+2])); i+=2; }
      else out+=c;
    }
    return out;
  }
  static String htmlEscape(String s) {
    s.replace("&", "&amp;"); s.replace("<", "&lt;"); s.replace(">", "&gt;"); s.replace("\"", "&quot;");
    return s;
  }

  static String jsonEscape(String s) {
    s.replace("\\", "\\\\"); s.replace("\"","\\\"");
    s.replace("\b","\\b");  s.replace("\f","\\f");
    s.replace("\n","\\n");  s.replace("\r","\\r"); s.replace("\t","\\t");
    return s;
  }

  String deviceIdShort() {
    uint8_t mac[6];
    WiFi.macAddress(mac);

    char id[7];
    snprintf(id, sizeof(id), "%02X%02X%02X", mac[3], mac[4], mac[5]);
    return String(id);
  }

  String makeApSsid() {
    return String("demowifi-") + deviceIdShort();
  }

  static String indexPage(const Config* cfg = nullptr) {
    String host = (cfg && cfg->mhost[0]) ? String(cfg->mhost) : "Enter host name here";
    String user = (cfg && cfg->muser[0]) ? String(cfg->muser) : "Enter user name here";
    uint16_t port = (cfg && cfg->mport) ? cfg->mport : 1883;

    String html; html.reserve(12000);
    html += F(
      "<!DOCTYPE html><html><head><meta charset=\"utf-8\"/>"
      "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\" />"
      "<title>LavinMQ arduino setup</title>"
      "<style>:root{color-scheme:dark}body{background:#181818;color:#fff;"
      "font-family:-apple-system,system-ui,Segoe UI,Roboto,Helvetica,Arial,\"Apple Color Emoji\",\"Segoe UI Emoji\";"
      "margin:0;padding:24px}.page-header{display:flex;justify-content:center;margin:32px 0}.page-intro{max-width:800px;"
      "margin:-8px auto 24px;color:#e6e6e6;text-align:center}.form-box{background:#414140;max-width:800px;margin:0 auto 24px;"
      "padding:24px;border-radius:8px;box-sizing:border-box}.form-box h3{margin:0 0 16px;font-weight:600}"
      ".form-box h1{margin:0 0 8px;font-size:22px;line-height:1.25;font-weight:700;color:#fff}.form-box p{margin:0 0 16px;"
      "color:#e6e6e6}.form-box hr{border:0;border-top:1px dashed #5a5a59;opacity:.6;margin:16px 0}.form-grid{display:grid;"
      "grid-template-columns:1fr 1.5fr;gap:12px 16px;align-items:center}.form-grid input,.form-grid select{width:100%;padding:8px 10px;"
      "background:#2a2a2a;border:1px solid #5a5a59;color:#fff;border-radius:6px;box-sizing:border-box}.actions{display:flex;"
      "justify-content:flex-end;margin-top:16px;gap:12px;align-items:center;flex-wrap:wrap}"
      "button{background:#54be7f;color:#141414;border:none;padding:10px 14px;border-radius:6px;cursor:pointer;font-weight:600}"
      "button.secondary{background:#b85a5a;color:#fff}"
      ".msg{margin-left:auto;font-size:.95em;opacity:.95;min-height:1.2em}"
      ".msg.ok{color:#8fe39b}.msg.err{color:#ffb4a3}.muted{opacity:.8}"
      "@media(max-width:560px){.form-grid{grid-template-columns:1fr}"
      ".actions{justify-content:stretch;flex-direction:column;align-items:stretch}"
      ".actions button{width:100%}.msg{margin:8px 0 0 0}}</style>"
      "</head><body>"
      "<div class=\"page-header\"><h2>LavinMQ setup</h2></div>"
      "<div class=\"page-intro\"><p>You're connected to the Beetle ESP32-C6. Configure your LavinMQ instance & Wi-Fi below.</p></div>"

      // MQTT form
      "<form class=\"form-box\" action=\"/savemqtt\" method=\"GET\" id=\"mqttForm\"><h1>Step 1: Connect to your LavinMQ instance</h1>"
      "<hr /><h3>MQTT Connection details</h3><div class=\"form-grid\">"
      "<label for=\"host\">Host:</label><input id=\"host\" type=\"text\" name=\"host\" value=\"");
    html += htmlEscape(host);
    html += F("\" />"
      "<label for=\"port\">Port:</label><input id=\"port\" type=\"number\" name=\"port\" value=\"");
    html += String(port);
    html += F("\" />"
      "<label for=\"user\">Username:</label><input id=\"user\" type=\"text\" name=\"user\" value=\"");
    html += htmlEscape(user);
    html += F("\" />"
      "<label for=\"mqttpass\">Password:</label><input id=\"mqttpass\" type=\"password\" name=\"pass\" placeholder=\"leave blank to keep\" />"
      "</div><div class=\"actions\">"
      "<span id=\"mqttMsg\" class=\"msg\"></span>"
      "<button type=\"submit\">Save MQTT</button>"
      "<button type=\"button\" id=\"delMqttBtn\" class=\"secondary\">Delete MQTT</button>"
      "</div></form>"

      // Wi-Fi form
      "<form class=\"form-box\" action=\"/savewifi\" method=\"GET\" id=\"wifiForm\"><h1>Step 2: Connect to your Wi-Fi network</h1><hr />"
      "<div class=\"form-grid\">"
      "<label for=\"ssid\">Network (SSID):</label>"
      "<select id=\"ssid\" name=\"ssid\"><option>Scanning…</option></select>"
      "<label for=\"wifipass\">Password:</label><input id=\"wifipass\" type=\"password\" name=\"pass\" />"
      "</div>"
      "<div class=\"actions\">"
      "<span id=\"scanStatus\" class=\"muted\" style=\"margin-right:auto\"></span>"
      "<span id=\"wifiMsg\" class=\"msg\"></span>"
      "<button type=\"submit\">Save Wi-Fi</button>"
      "<button type=\"button\" id=\"delWifiBtn\" class=\"secondary\">Delete Wi-Fi</button>"
      "</div></form>"
      "<script>"
      "const ssidSel=document.getElementById('ssid');"
      "const scanStatus=document.getElementById('scanStatus');"
      "const mqttForm=document.getElementById('mqttForm');"
      "const wifiForm=document.getElementById('wifiForm');"
      "const delMqttBtn=document.getElementById('delMqttBtn');"
      "const delWifiBtn=document.getElementById('delWifiBtn');"
      "const mqttMsg=document.getElementById('mqttMsg');"
      "const wifiMsg=document.getElementById('wifiMsg');"

      "function setMsg(el, ok, text){el.className='msg ' + (ok?'ok':'err'); el.textContent=text;}"

      // Wi-Fi scan
      "async function startScan(){"
      "  try{await fetch('/scan?start=1');}catch(e){}"
      "  scanStatus.textContent='Scanning…';"
      "  ssidSel.innerHTML='<option>Scanning…</option>';"
      "  pollScan();"
      "}"
      "async function pollScan(){"
      "  for(;;){"
      "    let r=await fetch('/scan');"
      "    let j=await r.json().catch(()=>({status:'error'}));"
      "    if(j.status==='started'||j.status==='scanning'){await new Promise(res=>setTimeout(res,800));continue;}"
      "    if(j.status!=='done'){scanStatus.textContent='Scan error';return;}"
      "    const list=(j.networks||[]).filter(n=>n.ssid&&n.ssid.length>0);"
      "    if(!list.length){scanStatus.textContent='No networks found'; ssidSel.innerHTML='<option>(none)</option>'; return;}"
      "    list.sort((a,b)=>b.rssi-a.rssi);"
      "    ssidSel.innerHTML='';"
      "    for(const ap of list){const o=document.createElement('option');o.value=ap.ssid;o.textContent=`${ap.ssid} (${ap.rssi} dBm) ${ap.secure?'🔒':''}`;ssidSel.appendChild(o);}"
      "    scanStatus.textContent='';"
      "    return;"
      "  }"
      "}"

      "async function submitAjax(form, msgEl, okText){"
      "  const params=new URLSearchParams(new FormData(form));"
      "  params.append('ajax','1');"
      "  setMsg(msgEl,true,'Working…');"
      "  try{"
      "    const res=await fetch(form.action+'?'+params.toString());"
      "    const ct=res.headers.get('Content-Type')||'';"
      "    if(ct.includes('application/json')){const j=await res.json(); setMsg(msgEl, j.ok!==false, j.message || (res.ok?okText:'Request failed'));}"
      "    else{setMsg(msgEl, res.ok, res.ok?okText:'Request failed');}"
      "  }catch(e){setMsg(msgEl,false,'Connection error');}"
      "}"

      "mqttForm.addEventListener('submit',async(e)=>{e.preventDefault();await submitAjax(mqttForm,mqttMsg,'MQTT settings saved.');});"
      "wifiForm.addEventListener('submit',async(e)=>{e.preventDefault();await submitAjax(wifiForm,wifiMsg,'Wi-Fi saved. Connecting…');});"

      // Delete actions (AJAX)
      "delMqttBtn.addEventListener('click',async()=>{"
      "  setMsg(mqttMsg,true,'Deleting MQTT…');"
      "  try{const r=await fetch('/deletemqtt?ajax=1'); const j=await r.json().catch(()=>({ok:r.ok})); setMsg(mqttMsg,j.ok!==false,(j.message||'MQTT settings deleted.'));}"
      "  catch(e){setMsg(mqttMsg,false,'Connection error');}"
      "});"
      "delWifiBtn.addEventListener('click',async()=>{"
      "  setMsg(wifiMsg,true,'Deleting Wi-Fi…');"
      "  try{const r=await fetch('/deletewifi?ajax=1'); const j=await r.json().catch(()=>({ok:r.ok})); setMsg(wifiMsg,j.ok!==false,(j.message||'Wi-Fi settings deleted.'));}"
      "  catch(e){setMsg(wifiMsg,false,'Connection error');}"
      "});"

      "document.addEventListener('DOMContentLoaded',()=>{startScan();});"
      "</script>"

      "</body></html>"
    );

    return html;
  }
} // namespace

namespace Portal {
  static WiFiServer web(80);
  static bool portalRunning = false;
  static unsigned long portalUntil = 0;

  void startAPPortalWindow() {
    if (!portalRunning) {
      WiFi.mode(WIFI_AP_STA);                 
      const char* ssid = Portal::apSsid();        
      bool ok = WiFi.softAP(ssid);
      Serial.printf("[PORTAL] SoftAP '%s' %s\n", ssid, ok ? "started" : "FAILED");
      if (!ok) { delay(800); ok = WiFi.softAP(ssid); Serial.println(ok ? "[PORTAL] Retry OK" : "[PORTAL] Retry FAILED"); }
      Serial.printf("[STICKER] SSID: %s\n", ssid);
      Serial.printf("[STICKER] ID: %s\n", deviceIdShort().c_str());
      web.begin();
      portalRunning = true;
    }
    portalUntil = millis() + AP_WINDOW_MS;
    IPAddress ip = WiFi.softAPIP();
    Serial.print("[PORTAL] Visit http://"); Serial.println(ip);
    Serial.println("[PORTAL] Tip: Save Wi-Fi to stop AP and join your network.");
  }

  const char* apSsid() {
    static String ssid;
    if (!ssid.length()) ssid = makeApSsid();
    return ssid.c_str();
  }


  void stopAP() {
    if (portalRunning) {
      web.end();
      WiFi.softAPdisconnect(true);
      WiFi.mode(WIFI_STA);
      portalRunning = false;
      Serial.println("[Portal] AP stopped.");
    }
  }

  bool running() { return portalRunning; }

  void maybeStopPortal() {
    if (portalRunning && millis() > portalUntil) {
      stopAP();
      Serial.println("[Portal] Window elapsed — stopping AP.");
    }
  }

  void handleOnce() {
    WiFiClient c = web.available();
    if (!c) return;

    // Visiting the portal extends the window
    portalUntil = millis() + AP_EXTEND_MS;

    String reqLine = c.readStringUntil('\n');
    reqLine.trim();
    // Drain the rest
    while (c.connected() && c.available()) { c.read(); }

    auto send = [&](const char* status, const char* ctype, const String& body){
      c.print(status); c.print("\r\nContent-Type: "); c.print(ctype);
      c.print("\r\nConnection: close\r\nContent-Length: "); c.print(body.length());
      c.print("\r\n\r\n"); c.print(body); c.stop();
    };
    auto send200 = [&](const String& body){ send("HTTP/1.1 200 OK", "text/html", body); };
    auto send400 = [&](const char* msg){
      String body = String("<!doctype html><html><body><h3>") + msg + "</h3></body></html>";
      send("HTTP/1.1 400 Bad Request", "text/html", body);
    };
    auto sendJSON = [&](const String& body){ send("HTTP/1.1 200 OK", "application/json", body); };

    int qStart = reqLine.indexOf(' ') + 1;
    int qEnd = reqLine.indexOf(' ', qStart);
    String path = reqLine.substring(qStart, qEnd);
    int qs = path.indexOf('?');
    String route = qs >= 0 ? path.substring(0, qs) : path;
    String query = qs >= 0 ? path.substring(qs + 1) : "";

    auto getParam = [&](const String& key)->String{
      int pos = 0;
      while (pos >= 0) {
        int amp = query.indexOf('&', pos);
        String pair = amp >= 0 ? query.substring(pos, amp) : query.substring(pos);
        int eq = pair.indexOf('=');
        if (eq > 0) {
          String k = pair.substring(0, eq);
          String v = urlDecode(pair.substring(eq + 1));
          if (k == key) return v;
        }
        if (amp < 0) break; else pos = amp + 1;
      }
      return String("");
    };

    static Config cfgCache{};
    ConfigStore::load(cfgCache);

    // ---- Async Wi-Fi scan endpoint (JSON) ----
    if (route == "/scan") {
      // If explicitly asked to start or if never started, kick off async scan
      String start = getParam("start");
      int n = WiFi.scanComplete();   // -2: not started, -1: running, >=0 done
      if (start.length() || n == -2) {
        WiFi.scanDelete();
        WiFi.scanNetworks(true /*async*/, true /*show_hidden*/);
        return sendJSON(F("{\"status\":\"started\"}"));
      }
      if (n == -1) {
        return sendJSON(F("{\"status\":\"scanning\"}"));
      }
      // Scan finished
      String body = "{\"status\":\"done\",\"networks\":[";
      bool first = true;
      for (int i = 0; i < n; i++) {
        String ssid = WiFi.SSID(i);
        if (!ssid.length()) continue; 
        long rssi = WiFi.RSSI(i);
        int enc = WiFi.encryptionType(i); // 0=open
        if (!first) body += ',';
        first = false;
        body += "{\"ssid\":\"" + jsonEscape(ssid) + "\",\"rssi\":" + String(rssi) + ",\"secure\":";
        body += (enc == 0 ? "false" : "true");
        body += "}";
      }
      body += "]}";
      return sendJSON(body);
    }

    // ---- Save Wi-Fi ----
    if (route == "/savewifi") {
      String ssid = getParam("ssid"); ssid.trim();
      String pass = getParam("pass"); pass.trim();
      String ajax = getParam("ajax");
      if (ssid.length() == 0) return send400("Missing SSID");
      ConfigStore::saveWifiOnly(ssid.c_str(), pass.c_str(), &cfgCache);

      if (ajax.length()) {
        return sendJSON(F("{\"ok\":true,\"message\":\"Wi-Fi saved!\"}"));
      } else {
        send200(F("<!doctype html><html><body><h3>Wi-Fi saved. Connecting…</h3></body></html>"));
        stopAP(); // allow STA to connect immediately
        return;
      }
    }

    // ---- Save MQTT ----
    if (route == "/savemqtt") {
      String host = getParam("host"); host.trim();
      String port = getParam("port"); port.trim();
      String user = getParam("user"); user.trim();
      String pass = getParam("pass"); pass.trim();
      String ajax = getParam("ajax");
      if (host.length() == 0) return send400("Missing host");
      uint16_t p = (uint16_t)(port.length() ? port.toInt() : (cfgCache.mport ? cfgCache.mport : 1883));
      ConfigStore::saveMqttOnly(host.c_str(), p, (user.length()?user.c_str():""), (pass.length()?pass.c_str():nullptr), &cfgCache);

      if (ajax.length()) {
        String body = String("{\"ok\":true,\"message\":\"MQTT saved.\",\"broker\":\"") + jsonEscape(host) + "\",\"port\":" + String(p) + "}";
        return sendJSON(body);
      } else {
        String body = String(F("<!doctype html><html><body><h3>MQTT saved.</h3><p>Broker: "))
                    + htmlEscape(host) + ":" + String(p) + F("</p></body></html>");
        send200(body);
        return;
      }
    }

    // ---- Maintenance ----
    if (route == "/deletewifi") {
      String ajax = getParam("ajax");
      ConfigStore::clearWifi();
      if (ajax.length()) return sendJSON(F("{\"ok\":true,\"message\":\"Wi-Fi settings deleted.\"}"));
      send200(F("<!doctype html><html><body><h3>Wi-Fi settings deleted.</h3></body></html>"));
      return;
    }

    if (route == "/deletemqtt") {
      String ajax = getParam("ajax");
      ConfigStore::clearMqtt();
      if (ajax.length()) return sendJSON(F("{\"ok\":true,\"message\":\"MQTT settings deleted.\"}"));
      send200(F("<!doctype html><html><body><h3>MQTT settings deleted.</h3></body></html>"));
      return;
    }

    // default = index page
    send200(indexPage(&cfgCache));
  }
}
