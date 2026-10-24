// Keep the live site's image and timer running; restore its DOM unchanged on return.
(function () {
    let hidden = false;
    const frame = document.createElement('div');
    frame.id = 'gestart-focus';
    frame.style.cssText = 'position:fixed;inset:0;z-index:2147483647;background:#111;display:none;pointer-events:none;align-items:center;justify-content:center;';
    const style = document.createElement('style');
    style.textContent = 'html[data-gestart-hidden] :is(button,input,select,textarea,nav,header,footer,[role=toolbar]){opacity:0!important;pointer-events:none!important}';
    document.documentElement.appendChild(style);
    const image = document.createElement('img');
    image.style.cssText = 'max-width:100%;max-height:100%;object-fit:contain;';
    frame.appendChild(image);
    document.documentElement.appendChild(frame);
    function update() {
        if (!hidden) { frame.style.display = 'none'; return; }
        const candidates = Array.from(document.images).filter(i => {
            if (frame.contains(i) || !i.complete || i.naturalWidth < 180 || i.naturalHeight < 180) return false;
            const r = i.getBoundingClientRect(), s = getComputedStyle(i);
            return r.width > 160 && r.height > 160 && r.right > 0 && r.bottom > 0 && r.left < innerWidth && r.top < innerHeight && s.visibility !== 'hidden' && s.display !== 'none' && Number(s.opacity) > 0;
        });
        candidates.sort((a,b) => b.getBoundingClientRect().width*b.getBoundingClientRect().height-a.getBoundingClientRect().width*a.getBoundingClientRect().height);
        const target = candidates[0];
        if (!target) { frame.style.display = 'none'; return; }
        const url = target.currentSrc || target.src;
        if (image.src !== url) image.src = url;
        frame.style.display = 'flex';
    }
    window.gestartFocus = function (enabled) { hidden = enabled; document.documentElement.toggleAttribute('data-gestart-hidden', enabled); update(); };
    // Site timers can replace an image without navigating the page.
    setInterval(update, 250);
}());
