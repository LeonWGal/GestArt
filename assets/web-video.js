// Expand the existing player without reloading the page or replacing the video.
(function () {
    let player = null;
    let ancestors = [];
    const style = document.createElement('style');
    style.textContent = `
        html[data-gestart-video] { overflow:hidden!important; }
        html[data-gestart-video] body * { visibility:hidden!important; }
        [data-gestart-video-ancestor] { transform:none!important; filter:none!important;
            perspective:none!important; contain:none!important; overflow:visible!important;
            position:static!important; z-index:auto!important; }
        html[data-gestart-video] [data-gestart-video-player], html[data-gestart-video] [data-gestart-video-player] * { visibility:visible!important; }
        [data-gestart-video-player] { position:fixed!important; inset:0!important;
            width:100vw!important; height:100vh!important; max-width:none!important;
            max-height:none!important; margin:0!important; z-index:2147483647!important;
            background:#000!important; }
        [data-gestart-video-player] .html5-video-container { position:absolute!important; inset:0!important; }
        [data-gestart-video-player] video, video[data-gestart-video-player] {
            width:100%!important; height:100%!important; left:0!important; top:0!important; object-fit:contain!important; }
        [data-gestart-video-player] .ytp-chrome-bottom { width:calc(100% - 24px)!important; left:12px!important; }
    `;
    document.documentElement.appendChild(style);
    function restore() {
        if (player) player.removeAttribute('data-gestart-video-player');
        ancestors.forEach(el => el.removeAttribute('data-gestart-video-ancestor'));
        ancestors = [];
        player = null;
        document.documentElement.removeAttribute('data-gestart-video');
        return false;
    }
    window.gestartToggleVideo = function () {
        if (player) return restore();
        const video = Array.from(document.querySelectorAll('video')).find(v => {
            const r = v.getBoundingClientRect();
            return r.width > 0 && r.height > 0;
        });
        if (!video) return false;
        player = video.closest('.html5-video-player') || video;
        for (let el = player.parentElement; el; el = el.parentElement) {
            el.setAttribute('data-gestart-video-ancestor', '');
            ancestors.push(el);
        }
        player.setAttribute('data-gestart-video-player', '');
        document.documentElement.setAttribute('data-gestart-video', '');
        return true;
    };
    document.addEventListener('yt-navigate-start', restore);
    document.addEventListener('keydown', event => {
        if (event.key === 'Escape' && player) { restore(); event.preventDefault(); }
    }, true);
    window.gestartVideoExpanded = () => !!player;
}());
