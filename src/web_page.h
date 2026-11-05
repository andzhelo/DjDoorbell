// The remote pad page the outside unit serves at GET /. One file, no
// external assets, so it works from any phone on the home network:
// open http://djdoorbell-outside.local/ and "Add to Home Screen".
#pragma once

static const char kWebPage[] PROGMEM = R"HTML(<!doctype html>
<html lang="en"><head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1,user-scalable=no">
<meta name="apple-mobile-web-app-capable" content="yes">
<meta name="apple-mobile-web-app-status-bar-style" content="black-translucent">
<meta name="theme-color" content="#111">
<title>DJ Doorbell</title>
<style>
  html,body{margin:0;height:100%;background:#111;color:#eee;font-family:-apple-system,system-ui,sans-serif;
    -webkit-user-select:none;user-select:none;-webkit-touch-callout:none;overscroll-behavior:none}
  main{display:flex;flex-direction:column;align-items:center;justify-content:center;min-height:100%;gap:18px;padding:16px;box-sizing:border-box}
  h1{font-size:18px;font-weight:600;margin:0;letter-spacing:.08em;opacity:.7}
  .pad{display:grid;grid-template-columns:repeat(3,1fr);gap:14px;width:min(92vw,380px)}
  button{appearance:none;border:0;border-radius:50%;aspect-ratio:1;width:100%;font-size:28px;font-weight:700;color:#111;
    touch-action:manipulation;cursor:pointer;box-shadow:0 6px 0 rgba(0,0,0,.45);transition:transform .05s,box-shadow .05s,filter .05s}
  button:active,button.on{transform:translateY(5px);box-shadow:0 1px 0 rgba(0,0,0,.45);filter:brightness(1.35)}
  .r1{background:#ffb000}.r2{background:#00d8ff}.r3{background:#ff38c8}
  .chime{border-radius:40px;aspect-ratio:auto;height:64px;width:min(92vw,380px);background:#eee;font-size:20px}
  #st{font-size:13px;opacity:.5;min-height:1.2em}
  details{width:min(92vw,380px);background:#1c1c1c;border-radius:16px;padding:0 16px;box-sizing:border-box}
  summary{cursor:pointer;padding:14px 0;font-size:15px;font-weight:600;list-style:none;display:flex;justify-content:space-between;align-items:center}
  summary::-webkit-details-marker{display:none}
  summary span{font-weight:400;opacity:.6;font-size:13px}
  .row{display:flex;align-items:center;justify-content:space-between;gap:12px;padding:10px 0;border-top:1px solid #2a2a2a;font-size:15px}
  .row label{flex:1}
  .row input[type=time]{font:inherit;font-size:16px;color:#eee;background:#2a2a2a;border:0;border-radius:8px;padding:6px 8px;color-scheme:dark}
  .row input[type=range]{flex:1;accent-color:#ffb000;min-width:0}
  .row output{width:42px;text-align:right;font-variant-numeric:tabular-nums;opacity:.8}
  .sw{position:relative;width:52px;height:30px;flex:none}
  .sw input{opacity:0;width:0;height:0}
  .sw i{position:absolute;inset:0;background:#444;border-radius:30px;transition:.15s}
  .sw i:before{content:"";position:absolute;width:24px;height:24px;left:3px;top:3px;background:#fff;border-radius:50%;transition:.15s}
  .sw input:checked+i{background:#ffb000}
  .sw input:checked+i:before{transform:translateX(22px)}
  .lv{flex-direction:column;align-items:stretch;gap:6px}
  .lv div{display:flex;align-items:center;gap:10px}
  .note{font-size:12px;opacity:.5;padding:4px 0 14px;border-top:1px solid #2a2a2a}
</style></head><body><main>
<h1>DJ DOORBELL</h1>
<div class="pad">
  <button class="r1" data-p="1">1</button><button class="r1" data-p="2">2</button><button class="r1" data-p="3">3</button>
  <button class="r2" data-p="4">4</button><button class="r2" data-p="5">5</button><button class="r2" data-p="6">6</button>
  <button class="r3" data-p="7">7</button><button class="r3" data-p="8">8</button><button class="r3" data-p="9">9</button>
</div>
<button class="chime" data-c="1">chime</button>
<div id="st"></div>
<details id="q">
  <summary>Quiet hours <span id="qs">…</span></summary>
  <div class="row"><label for="qon">Turn down at night</label>
    <span class="sw"><input type="checkbox" id="qon"><i></i></span></div>
  <div class="row"><label for="qfrom">From</label><input type="time" id="qfrom"></div>
  <div class="row"><label for="qto">Until</label><input type="time" id="qto"></div>
  <div class="row lv"><label for="qout">Door speaker during quiet hours</label>
    <div><input type="range" id="qout" min="0" max="100" step="5"><output for="qout" id="qouto"></output></div></div>
  <div class="row lv"><label for="qin">Inside chime during quiet hours</label>
    <div><input type="range" id="qin" min="0" max="100" step="5"><output for="qin" id="qino"></output></div></div>
  <div class="note" id="qn">0 % is silent, 100 % is normal. The pads still light up.</div>
</details>
<script>
  const st=document.getElementById('st');
  function hit(b){
    const url=b.dataset.c?'/chime':'/pad/'+b.dataset.p;
    b.classList.add('on');setTimeout(()=>b.classList.remove('on'),120);
    fetch(url,{cache:'no-store'}).then(r=>{st.textContent=r.ok?'':'error '+r.status;})
      .catch(()=>{st.textContent='no connection';});
  }
  document.querySelectorAll('button').forEach(b=>{
    b.addEventListener('pointerdown',e=>{e.preventDefault();hit(b);});
  });

  // Quiet hours: settings live on the unit; every change is saved at once.
  const $=id=>document.getElementById(id);
  const hm=m=>String(Math.floor(m/60)).padStart(2,'0')+':'+String(m%60).padStart(2,'0');
  const mins=t=>{const [h,m]=t.split(':').map(Number);return h*60+m;};
  function show(q){
    $('qon').checked=q.on;$('qfrom').value=hm(q.from);$('qto').value=hm(q.to);
    $('qout').value=q.out;$('qin').value=q.in;$('qouto').value=q.out+' %';$('qino').value=q.in+' %';
    const clock=q.now<0?'Clock not set yet.':'It is '+hm(q.now)+(q.active?', quiet now.':'.');
    $('qs').textContent=!q.on?'off':q.active?'ON until '+hm(q.to):hm(q.from)+'–'+hm(q.to);
    $('qn').textContent=clock+' 0 % is silent, 100 % is normal; the pads still light up.';
  }
  function load(){fetch('/quiet',{cache:'no-store'}).then(r=>r.json()).then(show).catch(()=>{$('qs').textContent='?';});}
  let timer;
  function save(){
    clearTimeout(timer);
    timer=setTimeout(()=>{
      const u='/quiet/set?on='+($('qon').checked?1:0)+'&from='+mins($('qfrom').value)+'&to='+mins($('qto').value)
        +'&out='+$('qout').value+'&in='+$('qin').value;
      fetch(u,{cache:'no-store'}).then(r=>r.json()).then(show).catch(()=>{$('qs').textContent='not saved';});
    },300);
  }
  ['qon','qfrom','qto','qout','qin'].forEach(id=>$(id).addEventListener('change',save));
  $('qout').addEventListener('input',()=>{$('qouto').value=$('qout').value+' %';});
  $('qin').addEventListener('input',()=>{$('qino').value=$('qin').value+' %';});
  load();
  setInterval(()=>{if(!document.hidden)load();},60000);
</script>
</main></body></html>
)HTML";
