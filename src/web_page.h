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
</style></head><body><main>
<h1>DJ DOORBELL</h1>
<div class="pad">
  <button class="r1" data-p="1">1</button><button class="r1" data-p="2">2</button><button class="r1" data-p="3">3</button>
  <button class="r2" data-p="4">4</button><button class="r2" data-p="5">5</button><button class="r2" data-p="6">6</button>
  <button class="r3" data-p="7">7</button><button class="r3" data-p="8">8</button><button class="r3" data-p="9">9</button>
</div>
<button class="chime" data-c="1">chime</button>
<div id="st"></div>
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
</script>
</main></body></html>
)HTML";
