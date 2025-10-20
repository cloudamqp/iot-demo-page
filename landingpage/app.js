// ============ Device Readings over MQTT (real data only) ============

// WebSocket endpoint for LavinMQ/RabbitMQ MQTT-over-WebSocket
const MQTT_URL = "wss://<hostname>/mqtt";

// Topics must match what your device publishes
const TOPIC_TEMP = "lavinmq/home/temperature";
const TOPIC_HUM  = "lavinmq/home/humidity";

// IMPORTANT: RabbitMQ/LavinMQ use user@vhost for MQTT auth (not user:vhost)
const MQTT_OPTIONS = {
  username: "<username>",
  password: "<password>",
  clientId: "webclient-" + Math.random().toString(16).slice(2),
  clean: true,
  keepalive: 60,
  reconnectPeriod: 2000,
  protocolVersion: 4,
  protocolId: "MQTT"
};

// DOM
const el = {
  tempCur:  document.getElementById("temp-current"),
  tempChg:  document.getElementById("temp-change"),
  humCur:   document.getElementById("hum-current"),
  humChg:   document.getElementById("hum-change"),
  tempSVG:  document.getElementById("temp-spark"),
  humSVG:   document.getElementById("hum-spark"),
};

// Series
const MAX_POINTS = 60;
const series = { temp: [], hum: [] };

// Helpers
const lines = t => String(t).split(/\r?\n/).map(s=>s.trim()).filter(Boolean);

// Parse "current temperature: 23.4" / "current humidity: 51"
const numFrom = (line, key) => {
  const rx = new RegExp(`^\\s*current\\s*${key}\\s*:\\s*([0-9]+(?:\\.[0-9]+)?)\\s*$`, "i");
  const m = String(line).match(rx);
  return m ? parseFloat(m[1]) : null;
};

const accent = () =>
  getComputedStyle(document.documentElement).getPropertyValue("--accent").trim() || "#54be7e";

function drawSparkline(svg, data, color, h=svg.clientHeight||180){
  const w = svg.clientWidth || 600;
  const pad = 12;
  svg.setAttribute("viewBox", `0 0 ${w} ${h}`);
  svg.innerHTML = "";
  if(!data.length) return;
  let min = Math.min(...data), max = Math.max(...data);
  if(min===max){min-=1;max+=1;}
  const innerW=w-pad*2, innerH=h-pad*2, stepX=data.length>1?innerW/(data.length-1):innerW, baseY=pad+innerH;
  const x=i=>pad+i*stepX, y=v=>pad+(max-v)*innerH/(max-min);
  let d=`M ${x(0)} ${y(data[0])}`; for(let i=1;i<data.length;i++) d+=` L ${x(i)} ${y(data[i])}`;
  const area=document.createElementNS("http://www.w3.org/2000/svg","path");
  area.setAttribute("d",`${d} L ${x(data.length-1)} ${baseY} L ${x(0)} ${baseY} Z`);
  area.setAttribute("fill",color); area.setAttribute("opacity",".12"); svg.appendChild(area);
  const path=document.createElementNS("http://www.w3.org/2000/svg","path");
  path.setAttribute("d",d); path.setAttribute("fill","none"); path.setAttribute("stroke",color); path.setAttribute("stroke-width","2.5"); path.setAttribute("stroke-linecap","round"); path.setAttribute("stroke-linejoin","round"); svg.appendChild(path);
  for(let i=0;i<data.length;i++){
    const c=document.createElementNS("http://www.w3.org/2000/svg","circle");
    c.setAttribute("cx",x(i)); c.setAttribute("cy",y(data[i])); c.setAttribute("r","3");
    c.setAttribute("fill",color); c.setAttribute("stroke","#fff"); c.setAttribute("stroke-width","1");
    svg.appendChild(c);
  }
}

// Draw only when the SVG has a real size
function drawWhenReady(svg, series, color) {
  function go(){
    if (svg.clientWidth && svg.clientHeight) return drawSparkline(svg, series, color);
    requestAnimationFrame(go);
  }
  go();
}

// Reveal trend card if hidden
function revealTrend(id){
  const col = document.getElementById(id);
  if (col && col.classList.contains("d-none")) col.classList.remove("d-none");
}

function titleizeCurrent(l){
  return l.replace(/^current\s+([a-z])/i,(m,g1)=>`Current ${g1.toUpperCase()}`);
}

function updateTempBlock(message){
  const ls = lines(message);
  const current = ls.find(l=>l.toLowerCase().startsWith("current temperature")) || ls[0] || "";
  const change  = ls.find(l=>l.toLowerCase().startsWith("temperature changed")) || "";
  el.tempCur.textContent = titleizeCurrent(current);
  el.tempChg.textContent = change.charAt(0).toUpperCase()+change.slice(1);
  const v = numFrom(current, "temperature");
  if(v!==null){
    series.temp.push(v);
    if(series.temp.length>MAX_POINTS) series.temp.shift();
    revealTrend("temp-trend-col");
    if (series.temp.length === 1) drawWhenReady(el.tempSVG, series.temp, accent());
    else                          drawSparkline(el.tempSVG, series.temp, accent());
  }
}

function updateHumBlock(message){
  const ls = lines(message);
  const current = ls.find(l=>l.toLowerCase().startsWith("current humidity")) || ls[0] || "";
  const change  = ls.find(l=>l.toLowerCase().startsWith("humidity changed")) || "";
  el.humCur.textContent = titleizeCurrent(current);
  el.humChg.textContent = change.charAt(0).toUpperCase()+change.slice(1);
  const v = numFrom(current, "humidity");
  if(v!==null){
    series.hum.push(v);
    if(series.hum.length>MAX_POINTS) series.hum.shift();
    revealTrend("hum-trend-col");
    if (series.hum.length === 1) drawWhenReady(el.humSVG, series.hum, accent());
    else                          drawSparkline(el.humSVG, series.hum, accent());
  }
}

// Keep lines crisp on resize
window.addEventListener("resize", () => {
  if (series.temp.length) drawSparkline(el.tempSVG, series.temp, accent());
  if (series.hum.length)  drawSparkline(el.humSVG,  series.hum,  accent());
});

// === MQTT real-time only (no demo fallback) ===
document.addEventListener("DOMContentLoaded", ()=>{
  const client = mqtt.connect(MQTT_URL, MQTT_OPTIONS);

  client.on("connect", () => {
    console.log("MQTT connected");
    client.subscribe([TOPIC_TEMP, TOPIC_HUM], (err) => {
      if (err) console.error("Subscribe error:", err);
    });
  });

  client.on("message", (topic, payload) => {
    const msg = payload.toString();
    if (topic === TOPIC_TEMP || topic.endsWith("/temperature")) {
      updateTempBlock(msg);
    } else if (topic === TOPIC_HUM || topic.endsWith("/humidity")) {
      updateHumBlock(msg);
    } else {
      console.log("Other topic:", topic, msg);
    }
  });

  client.on("error",   (e)=>console.error("MQTT error:", e));
  client.on("close",   ()=>console.warn("MQTT closed"));
  client.on("offline", ()=>console.warn("MQTT offline"));
});
