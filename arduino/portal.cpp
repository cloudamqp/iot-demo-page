#include <Arduino.h>
#include "Portal.h"
#include <WiFiS3.h>
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

  // Escape for safe HTML attribute/text rendering
  static String htmlEscape(String s) {
    s.replace("&", "&amp;");
    s.replace("<", "&lt;");
    s.replace(">", "&gt;");
    s.replace("\"", "&quot;");
    return s;
  }

  // === New dark-themed page ===
  static String indexPage(const Config* cfg = nullptr) {
    // Prefill MQTT fields from saved config (fallback to examples)
    String host = (cfg && cfg->mhost[0]) ? String(cfg->mhost) : "campbell.lmq.cloudamqp.com";
    String user = (cfg && cfg->muser[0]) ? String(cfg->muser) : "fiaetnku:fiaetnku";
    uint16_t port = (cfg && cfg->mport) ? cfg->mport : 1883;

    // Scan Wi-Fi while we build the page
    int n = WiFi.scanNetworks();

    String html;
    html.reserve(9000);

    html += F(
      "<!DOCTYPE html>\n<html>\n  <head>\n"
      "    <meta name=\"viewport\" content=\"width=device-width,initial-scale=1\" />\n"
      "    <title>LavinMQ arduino setup</title>\n"
      "    <style>\n"
      "      :root { color-scheme: dark; }\n"
      "      body { background:#181818; color:#fff; font-family:-apple-system, system-ui, Segoe UI, Roboto, Helvetica, Arial, \"Apple Color Emoji\", \"Segoe UI Emoji\"; margin:0; padding:24px; }\n"
      "      .page-header { display:flex; justify-content:center; margin:32px 0; }\n"
      "      .page-intro { max-width:800px; margin:-8px auto 24px; color:#e6e6e6; text-align:center; }\n"
      "      .page-intro ul { list-style:disc inside; margin:8px 0 0; padding:0; color:#d8d8d8; }\n"
      "      .page-intro li { margin:4px 0; }\n"
      "      .form-box { background:#414140; max-width:800px; margin:0 auto 24px; padding:24px; border-radius:8px; box-sizing:border-box; }\n"
      "      .form-box h3 { margin:0 0 16px; font-weight:600; }\n"
      "      .form-box h1 { margin:0 0 8px; font-size:22px; line-height:1.25; font-weight:700; color:#fff; }\n"
      "      .form-box p { margin:0 0 16px; color:#e6e6e6; }\n"
      "      .form-box hr { border:0; border-top:1px dashed #5a5a59; opacity:.6; margin:16px 0; }\n"
      "      .form-box a { color:#54be7f; text-decoration:none; }\n"
      "      .form-box a:hover { text-decoration:underline; }\n"
      "      .form-grid { display:grid; grid-template-columns:1fr 1.5fr; gap:12px 16px; align-items:center; }\n"
      "      .form-grid label { text-align:left; }\n"
      "      .form-grid input, .form-grid select { width:100%; padding:8px 10px; background:#2a2a2a; border:1px solid #5a5a59; color:#fff; border-radius:6px; box-sizing:border-box; }\n"
      "      .actions { display:flex; justify-content:flex-end; margin-top:16px; gap:12px; align-items:center; }\n"
      "      .actions .hint { color:#e6e6e6; font-size:13px; max-width:520px; text-align:right; }\n"
      "      button { background:#54be7f; color:#141414; border:none; padding:10px 14px; border-radius:6px; cursor:pointer; font-weight:600; }\n"
      "      button:hover { background:#4cd37d; }\n"
      "      @media (max-width:560px) {\n"
      "        .form-grid { grid-template-columns:1fr; }\n"
      "        .actions { justify-content:stretch; flex-direction:column; align-items:stretch; }\n"
      "        .actions button { width:100%; }\n"
      "        .actions .hint { order:-1; text-align:left; margin:0 0 8px; }\n"
      "      }\n"
      "    </style>\n"
      "  </head>\n  <body>\n"
      "    <div class=\"page-header\">\n"
      "      <svg width=\"156\" height=\"32\" fill=\"none\" xmlns=\"http://www.w3.org/2000/svg\">\n"
      "        <path d=\"M153.959 23.044l1.782 1.9-2.357 2.108a.791.791 0 01-1.1-.048l-1.381-1.472a9.41 9.41 0 01-4.179.976 9.178 9.178 0 01-5.738-2.06 9.31 9.31 0 01-3.213-5.217 9.37 9.37 0 01.715-6.097 9.262 9.262 0 014.331-4.319 9.158 9.158 0 016.057-.655 9.216 9.216 0 015.141 3.294 9.347 9.347 0 01-.058 11.59zm-5.989-.668l-2.571-2.748 2.361-2.112a.79.79 0 011.091.052l2.33 2.496a5.893 5.893 0 00.738-2.904c0-3.2-2.29-5.34-5.195-5.34-2.905 0-5.199 2.132-5.199 5.34 0 3.208 2.294 5.344 5.199 5.344a6.684 6.684 0 001.246-.128zM135.155 26.152h-4.076V15.696l-4.476 7.432a.396.396 0 01-.35.196.396.396 0 01-.349-.196l-4.508-7.432v9.664a.803.803 0 01-.794.8h-3.294V8.172h4.076l4.842 8 4.842-8h4.075l.012 17.98z\" fill=\"#fff\"/>\n"
      "        <path d=\"M113.506 8.172v17.98h-3.06l-6.862-9.752v8.948a.803.803 0 01-.49.74.786.786 0 01-.304.06h-3.282V8.172h3.056l6.521 9.248a.198.198 0 00.361-.116V8.172h4.06zM95.912 26.152V8.172h-4.08v17.98h4.08zM72.664 8.172h4.457L81.069 21.4l3.97-13.228h4.44L83.847 25.6a.8.8 0 01-.754.552h-4.62l-5.81-17.98zM68.73 23.324h-6.635l-.707 2.276a.8.8 0 01-.758.56h-3.877l6.064-17.98h5.2l6.063 17.972h-4.445l-.904-2.828zm-1.226-3.852l-2.087-6.6-2.009 6.328a.2.2 0 00.191.26l3.905.012zM54.499 22.196v3.956H44.053V8.172h4.076v14.024h6.37z\" fill=\"#fff\"/>\n"
      "        <mask id=\"a\" maskUnits=\"userSpaceOnUse\" x=\"0\" y=\"0\" width=\"32\" height=\"32\">\n"
      "          <path d=\"M0 0h31.75v32H0V0z\" fill=\"#fff\"/>\n"
      "        </mask>\n"
      "        <g mask=\"url(#a)\">\n"
      "          <path fill-rule=\"evenodd\" clip-rule=\"evenodd\" d=\"M28.572 16.464l-1.254-1.315L20.3 8.14a.199.199 0 00-.28 0l-6.967 6.957a.203.203 0 00-.048.21l.24.69-.795-.831-2.799-2.795a.197.197 0 00-.28 0l-5.526 5.52a.08.08 0 01-.135-.045A12.48 12.48 0 013.572 16c0-6.837 5.519-12.4 12.303-12.4 6.433 0 11.727 5.003 12.257 11.351.017.204.06.581.127.774.125.358.318.744.313.739zM16.498 28.953l-1.071-.504c-.218-.074-.84-.121-1.068-.15-4.018-.5-7.441-2.955-9.298-6.394a.161.161 0 01.029-.19l4.29-4.284a.077.077 0 01.132.056v4.457a.2.2 0 00.058.141l5.813 5.806 1.115 1.062zM31.75 16c0-9.212-7.724-16.606-17.001-15.961C6.92.583.579 6.975.039 14.865-.601 24.215 6.735 32 15.875 32H28.824a.201.201 0 00.14-.344L17.534 20.24h-.002l-2.165-2.16a.16.16 0 010-.225l4.66-4.654a.078.078 0 01.133.055v4.49c0 .032.012.062.035.085l11.215 11.2a.2.2 0 00.34-.142V16.333l-.009.008c.003-.114.009-.227.009-.341z\" fill=\"#fff\"/>\n"
      "        </g>\n"
      "      </svg>\n"
      "    </div>\n"
      "    <div class=\"page-intro\">\n"
      "      <p>Nice! You're connected to the Arduino. Next, configure your LavinMQ instance &amp; enter your Wi-Fi credentials.</p>\n"
      "    </div>\n"
    );

    // MQTT form
    html += F(
      "    <form class=\"form-box\" action=\"/savemqtt\" method=\"GET\">\n"
      "      <h1>Step 1: Connect to your LavinMQ instance</h1>\n"
      "      <p>Signup at <a href=\"//customer.cloudamqp.com/instance/create?lavinmq\">CloudAMQP</a> and create a LavinMQ instance.</p>\n"
      "      <hr />\n"
      "      <h3>Copy from your LavinMQ instance - MQTT Connection details</h3>\n"
      "      <div class=\"form-grid\">\n"
      "        <label for=\"host\">Host:</label>\n"
      "        <input id=\"host\" type=\"text\" name=\"host\" value=\""
    );
    html += htmlEscape(host);
    html += F("\" />\n"
      "        <label for=\"port\">Port:</label>\n"
      "        <input id=\"port\" type=\"number\" name=\"port\" value=\"");
    html += String(port);
    html += F("\" />\n"
      "        <label for=\"user\">Username:</label>\n"
      "        <input id=\"user\" type=\"text\" name=\"user\" value=\"");
    html += htmlEscape(user);
    html += F("\" />\n"
      "        <label for=\"mqttpass\">Password:</label>\n"
      "        <input id=\"mqttpass\" type=\"password\" name=\"pass\" placeholder=\"leave blank to keep\" />\n"
      "      </div>\n"
      "      <div class=\"actions\">\n"
      "        <button type=\"submit\">Save MQTT</button>\n"
      "      </div>\n"
      "    </form>\n"
    );

    // Wi-Fi form (dynamic SSIDs + RSSI)
    html += F(
      "    <form class=\"form-box\" action=\"/savewifi\" method=\"GET\">\n"
      "      <h1>Step 2: Connect to your Wi-Fi network</h1>\n"
      "      <p>Enter your Wi-Fi network details below.</p>\n"
      "      <hr />\n"
      "      <div class=\"form-grid\">\n"
      "        <label for=\"ssid\">Network:</label>\n"
      "        <select id=\"ssid\" name=\"ssid\">\n"
    );

    for (int i = 0; i < n; i++) {
      String s = WiFi.SSID(i);
      String esc = htmlEscape(s);
      long rssi = WiFi.RSSI(i);
      html += "          <option value=\"";
      html += esc;
      html += "\">";
      html += esc;
      html += " (";
      html += String(rssi);
      html += " dBm))</option>\n"; // kept the sample's double ')'
    }

    html += F(
      "        </select>\n"
      "        <label for=\"wifipass\">Password:</label>\n"
      "        <input id=\"wifipass\" type=\"password\" name=\"pass\" />\n"
      "      </div>\n"
      "      <div class=\"actions\">\n"
      "        <p class=\"hint\">When you save, the access point shuts down and the Arduino connects to your Wi-Fi. This page won't be available. To return here, power-cycle the Arduino and reconnect to its access point within 60 seconds of startup.</p>\n"
      "        <button type=\"submit\">Save Wi-Fi</button>\n"
      "      </div>\n"
      "    </form>\n"
    );

    // Delete/erase controls
    html += F(
      "    <form class=\"form-box\" action=\"/deletewifi\" method=\"GET\">\n"
      "      <div class=\"actions\"><button>Delete Wi-Fi</button></div>\n"
      "    </form>\n"
      "    <form class=\"form-box\" action=\"/deletemqtt\" method=\"GET\">\n"
      "      <div class=\"actions\"><button>Delete MQTT</button></div>\n"
      "    </form>\n"
      "    <form class=\"form-box\" action=\"/erase\" method=\"GET\">\n"
      "      <div class=\"actions\"><button>Erase ALL saved settings</button></div>\n"
      "    </form>\n"
      "  </body>\n</html>\n"
    );

    return html;
  }
} // anonymous namespace

namespace Portal {
  // Keep these in the namespace so all Portal:: functions can see them
  static WiFiServer web(80);
  static bool portalRunning = false;
  static unsigned long portalUntil = 0;

  void startAPPortalWindow() {
    if (!portalRunning) {
      int ret = WiFi.beginAP(AP_SSID); // open AP; for password: WiFi.beginAP(AP_SSID, "portalPwd");
      if (ret != WL_AP_LISTENING) {
        Serial.println("[Portal] AP start failed, retrying...");
        delay(800);
        WiFi.beginAP(AP_SSID);
      }
      web.begin();
      portalRunning = true;
    }
    portalUntil = millis() + AP_WINDOW_MS;

    IPAddress ip = WiFi.localIP();
    Serial.print("[Portal] AP '"); Serial.print(AP_SSID); Serial.print("' up. Open http://");
    Serial.println(ip[0] ? ip : IPAddress(192,168,4,1));
  }

  void stopAP() {
    if (portalRunning) {
      web.end();
      WiFi.end(); // stops AP
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

    // flush rest of request headers
    while (c.connected() && c.available()) { c.read(); }

    auto send200 = [&](const String& body){
      c.print("HTTP/1.1 200 OK Content-Type: text/html Connection: close Content-Length: ");
      c.print(body.length());
      c.print("\r\n\r\n");
      c.print(body);
      c.stop();
    };
    auto send400 = [&](const char* msg){
      String body = String("<!doctype html><html><body><h3>") + msg + "</h3></body></html>";
      c.print("HTTP/1.1 400 Bad Request Content-Type: text/html Connection: close Content-Length: ");
      c.print(body.length());
      c.print("\r\n\r\n");
      c.print(body);
      c.stop();
    };

    // extract path + query  from "GET /path?query HTTP/1.1"
    int qStart = reqLine.indexOf(' ') + 1;
    int qEnd = reqLine.indexOf(' ', qStart);
    String path = reqLine.substring(qStart, qEnd);
    int qs = path.indexOf('?');
    String route = qs >= 0 ? path.substring(0, qs) : path;
    String query = qs >= 0 ? path.substring(qs + 1) : "";

    // Helper to extract a query param
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

    if (route == "/savewifi") {
      String ssid = getParam("ssid"); ssid.trim();
      String pass = getParam("pass"); pass.trim();
      if (ssid.length() == 0) return send400("Missing SSID");
      ConfigStore::saveWifiOnly(ssid.c_str(), pass.c_str(), &cfgCache);
      send200(F("<!doctype html><html><body><h3>Wi-Fi saved. Connecting...</h3></body></html>"));
      stopAP(); // allow STA to connect immediately
      return;
    }

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
