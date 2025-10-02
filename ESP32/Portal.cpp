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

  static String indexPage(const Config* cfg = nullptr) {
    String host = (cfg && cfg->mhost[0]) ? String(cfg->mhost) : "campbell.lmq.cloudamqp.com";
    String user = (cfg && cfg->muser[0]) ? String(cfg->muser) : "";
    uint16_t port = (cfg && cfg->mport) ? cfg->mport : 1883;

    String html; html.reserve(9000);
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
      "justify-content:flex-end;margin-top:16px;gap:12px;align-items:center}button{background:#54be7f;color:#141414;border:none;"
      "padding:10px 14px;border-radius:6px;cursor:pointer;font-weight:600}@media(max-width:560px){.form-grid{grid-template-columns:1fr}"
      ".actions{justify-content:stretch;flex-direction:column;align-items:stretch}.actions button{width:100%}}</style>"
      "</head><body>"
      "<div class=\"page-header\"><h2>LavinMQ setup</h2></div>"
      "<div class=\"page-intro\"><p>You're connected to the Beetle ESP32-C6. Configure your LavinMQ instance & Wi-Fi below.</p></div>"

      "<form class=\"form-box\" action=\"/savemqtt\" method=\"GET\"><h1>Step 1: Connect to your LavinMQ instance</h1>"
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
      "</div><div class=\"actions\"><button type=\"submit\">Save MQTT</button></div></form>"

      "<form class=\"form-box\" action=\"/savewifi\" method=\"GET\"><h1>Step 2: Connect to your Wi-Fi network</h1><hr />"
      "<div class=\"form-grid\">"
      "<label for=\"ssid\">Network (SSID):</label>"
      "<select id=\"ssid\" name=\"ssid\"><option>Scanning…</option></select>"
      "<label for=\"wifipass\">Password:</label><input id=\"wifipass\" type=\"password\" name=\"pass\" />"
      "</div>"
      "<div class=\"actions\">"
      "<span id=\"scanStatus\" style=\"margin-right:auto;opacity:.85\"></span>"
      "<button type=\"button\" id=\"refreshBtn\">Refresh list</button>"
      "<button type=\"submit\">Save Wi-Fi</button>"
      "</div></form>"

      "<form class=\"form-box\" action=\"/deletewifi\" method=\"GET\"><div class=\"actions\"><button>Delete Wi-Fi</button></div></form>"
      "<form class=\"form-box\" action=\"/deletemqtt\" method=\"GET\"><div class=\"actions\"><button>Delete MQTT</button></div></form>"
      "<form class=\"form-box\" action=\"/erase\" method=\"GET\"><div class=\"actions\"><button>Erase ALL saved settings</button></div></form>"

      "<script>"
      "const ssidSel=document.getElementById('ssid');"
      "const scanStatus=document.getElementById('scanStatus');"
      "const refreshBtn=document.getElementById('refreshBtn');"

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
      "    if(j.status==='started'||j.status==='scanning'){"
      "      await new Promise(res=>setTimeout(res,800));"
      "      continue;"
      "    }"
      "    if(j.status!=='done'){"
      "      scanStatus.textContent='Scan error';"
      "      return;"
      "    }"
      "    const list=(j.networks||[]).filter(n=>n.ssid&&n.ssid.length>0);"
      "    if(!list.length){scanStatus.textContent='No networks found'; ssidSel.innerHTML='<option>(none)</option>'; return;}"
      "    list.sort((a,b)=>b.rssi-a.rssi);"
      "    ssidSel.innerHTML='';"
      "    for(const ap of list){"
      "      const o=document.createElement('option');"
      "      o.value=ap.ssid;"
      "      o.textContent=`${ap.ssid} (${ap.rssi} dBm) ${ap.secure?'🔒':''}`;"
      "      ssidSel.appendChild(o);"
      "    }"
      "    scanStatus.textContent='';"
      "    return;"
      "  }"
      "}"

      "document.addEventListener('DOMContentLoaded',()=>{startScan();});"
      "refreshBtn.addEventListener('click',()=>startScan());"
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
      WiFi.mode(WIFI_AP_STA);                 // AP + STA so we can scan while AP is up
      bool ok = WiFi.softAP(AP_SSID);         // open AP; add a password if you prefer
      Serial.printf("[PORTAL] SoftAP '%s' %s\n", AP_SSID, ok ? "started" : "FAILED");
      if (!ok) { delay(800); ok = WiFi.softAP(AP_SSID); Serial.println(ok ? "[PORTAL] Retry OK" : "[PORTAL] Retry FAILED"); }
      web.begin();
      portalRunning = true;
    }
    portalUntil = millis() + AP_WINDOW_MS;
    IPAddress ip = WiFi.softAPIP();
    Serial.print("[PORTAL] Visit http://"); Serial.println(ip);
    Serial.println("[PORTAL] Tip: Save Wi-Fi to stop AP and join your network.");
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
        if (!ssid.length()) continue; // skip hidden/empty
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
      if (ssid.length() == 0) return send400("Missing SSID");
      ConfigStore::saveWifiOnly(ssid.c_str(), pass.c_str(), &cfgCache);
      send200(F("<!doctype html><html><body><h3>Wi-Fi saved. Connecting…</h3></body></html>"));
      stopAP(); // allow STA to connect immediately
      return;
    }

    // ---- Save MQTT ----
    if (route == "/savemqtt") {
      String host = getParam("host"); host.trim();
      String port = getParam("port"); port.trim();
      String user = getParam("user"); user.trim();
      String pass = getParam("pass"); pass.trim();
      if (host.length() == 0) return send400("Missing host");
      uint16_t p = (uint16_t)(port.length() ? port.toInt() : (cfgCache.mport ? cfgCache.mport : 1883));
      ConfigStore::saveMqttOnly(host.c_str(), p, (user.length()?user.c_str():""), (pass.length()?pass.c_str():nullptr), &cfgCache);
      String body = String(F("<!doctype html><html><body><h3>MQTT saved.</h3><p>Broker: "))
                  + htmlEscape(host) + ":" + String(p) + F("</p></body></html>");
      send200(body);
      return;
    }

    // ---- Maintenance ----
    if (route == "/deletewifi") {
      ConfigStore::clearWifi();
      send200(F("<!doctype html><html><body><h3>Wi-Fi settings deleted.</h3></body></html>"));
      return;
    }

    if (route == "/deletemqtt") {
      ConfigStore::clearMqtt();
      send200(F("<!doctype html><html><body><h3>MQTT settings deleted.</h3></body></html>"));
      return;
    }

    if (route == "/erase") {
      ConfigStore::clearAll();
      send200(F("<!doctype html><html><body><h3>All settings erased.</h3></body></html>"));
      return;
    }

    // default = index page
    send200(indexPage(&cfgCache));
  }
}
