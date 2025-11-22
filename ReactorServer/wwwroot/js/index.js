/*************************************************
 * IceACG 首页 index.js — 完全漫画化（分镜主题）
 *************************************************/

import { Api } from "./core/api.js";
import { State } from "./core/state.js";
import { UserCache } from "./core/userCache.js";


/*************************************************
 * 工具：格式化时间
 *************************************************/
function formatTime(ts) {
    const date = new Date(ts * 1000);
    const now = new Date();
    const diff = (now - date) / 1000;

    if (diff < 60) return "刚刚";
    if (diff < 3600) return Math.floor(diff / 60) + " 分钟前";
    if (diff < 86400) return Math.floor(diff / 3600) + " 小时前";

    return `${date.getFullYear()}-${date.getMonth()+1}-${date.getDate()}`;
}


/*************************************************
 * 速度线动画
 *************************************************/
let lastY = window.scrollY;
const speedlines = document.createElement("div");
speedlines.id = "speedlines";
document.body.appendChild(speedlines);

setInterval(() => {
    const newY = window.scrollY;
    speedlines.style.opacity = Math.abs(newY - lastY) > 18 ? 0.25 : 0;
    lastY = newY;
}, 80);


/*************************************************
 * 随机拟声词
 *************************************************/
const SFX_WORDS = ["ドン!", "バン!", "パァン!", "ズキューン!", "ゴゴゴ…", "カッ!", "ドドドド!", "ピカッ!", "バサッ!", "シュッ!"];
function randomSFX() { return SFX_WORDS[Math.floor(Math.random() * SFX_WORDS.length)]; }


/*************************************************
 * 骨架屏
 *************************************************/
function showSkeleton(count = 4) {
    const box = document.getElementById("post-list");
    for (let i = 0; i < count; i++) {
        let sk = document.createElement("div");
        sk.className = "post-skeleton";
        box.appendChild(sk);
    }
}
function clearSkeleton() {
    document.querySelectorAll(".post-skeleton").forEach(e => e.remove());
}


/*************************************************
 * 推荐帖子（无限流）
 *************************************************/
let page = 1, loading = false, finished = false;
let feedMode = "recommend";

async function loadPosts() {
    if (loading || finished) return;
    loading = true;

    const box = document.getElementById("post-list");
    showSkeleton(page === 1 ? 4 : 2);

    const url = 
        feedMode === "recommend"
        ? `/recommend/posts?page=${page}&page_size=10`
        : `/follow/feed?page=${page}&page_size=10`;

    const res = await Api.get(url);

    clearSkeleton();

    // ❌ 后端异常 —— 当成“已经到底”
    if (!res || res.code !== 0 || !Array.isArray(res.data)) {
        showLoadMore();
        return;
    }

    const list = res.data;

    /* ================================================
       ① 第一页为空：展示 empty-box，但不要 finished=true
       ================================================ */
    if (page === 1 && list.length === 0) {

        let html = "";

        if (feedMode === "recommend") {
            html = `
                <div class="empty-box">
                    <img src="/static/empty-recommend.png" class="empty-img">
                    <p>这里还没有帖子 ~</p>
                    <a class="btn-post" href="/publish.html">去发布第一条帖子！</a>
                </div>
            `;
        }
        else if (feedMode === "follow") {
            html = `
                <div class="empty-box">
                    <img src="/static/empty-follow.png" class="empty-img">
                    <p>你还没有关注任何人</p>
                    <a class="btn-post" href="/discover.html">去发现创作者</a>
                </div>
            `;
        }

        box.innerHTML = html;

        // ❗注意：这里不设 finished，以便 load-more 有机会出现
        loading = false;
        return;
    }

    /* ================================================
       ② 翻页加载时为空：正常的“没有更多内容啦”
       ================================================ */
    if (list.length === 0) {
        showLoadMore();
        return;
    }

    /* ================================================
       ③ 正常渲染列表
       ================================================ */
    for (const p of list) {
        const div = document.createElement("div");
        div.className = "manga-card";

        const cover = p.images?.[0] ?? null;
        const authorAvatar = await UserCache.getAvatar(p.author_avatar);

        div.innerHTML = `
            ${cover ? `
            <div class="manga-cover-box">
                <img src="${cover}" class="manga-cover">
            </div>` : ``}

            <div class="manga-info">
                <div class="manga-author">
                    <img src="${authorAvatar}" class="manga-avatar">
                    <div class="manga-author-info">
                        <span class="manga-author-name">${p.author_name}</span>
                        <span class="manga-time">${formatTime(p.create_time)}</span>
                    </div>
                </div>

                <div class="manga-title">${p.title}</div>
                <div class="manga-content">${p.content.slice(0, 60)}...</div>

                <div class="manga-bottom">
                    <span>👍 ${p.like_count}</span>
                    <span>💬 ${p.comment_count}</span>
                </div>
            </div>
        `;

        div.style.opacity = "0";
        div.style.transform = "translateY(20px)";
        setTimeout(() => {
            div.style.transition = "0.45s cubic-bezier(.33,1.02,.52,1.08)";
            div.style.opacity = "1";
            div.style.transform = "translateY(0)";
        }, 20);

        div.onclick = () => location.href = `/post.html?id=${p.id}`;
        box.appendChild(div);
    }

    page++;
    loading = false;
}


/* ================================================
   统一的 load-more 显示函数
   ================================================ */
function showLoadMore() {
    finished = true;
    loading = false;

    const loadMore = document.getElementById("load-more");
    loadMore.innerHTML = `
        <div>（ ゝ∀・）☆ 没有更多内容啦！</div>
        <div style="font-size:14px;margin-top:6px;color:#555;">已经看到世界尽头了</div>
    `;
    loadMore.classList.add("show");
}


function switchMode(mode) {
    if (feedMode === mode) return;

    feedMode = mode;

    // 激活样式
    document.querySelectorAll(".tab").forEach(t => t.classList.remove("tab-active"));
    document.getElementById(`tab-${mode}`).classList.add("tab-active");

    // 重置流
    page = 1;
    finished = false;
    loading = false;

    const box = document.getElementById("post-list");
    box.innerHTML = "";

    loadPosts();
}

/*************************************************
 * 热门话题
 *************************************************/
/* ======================
 * 首页热门话题（自适配后端）
 * ====================== */
async function loadTopics() {
    const res = await Api.get(`/topics/hot`);
    if (res.code !== 0) return;

    const box = document.getElementById("topics");

    // ★ 自动适配后端字段名
    const raw = res.data;
    const topics =
        Array.isArray(raw) ? raw :
        Array.isArray(raw?.list) ? raw.list :
        Array.isArray(raw?.topics) ? raw.topics :
        [];

    // ★ 空数据处理
    if (topics.length === 0) {
        box.innerHTML = `
            <div class="topic-item">（＞﹏＜）暂无热门话题</div>
        `;
        return;
    }

    // ★ 正常渲染
    box.innerHTML = topics.map(t => `
        <div class="topic-item"
            onclick="location.href='/search.html?q=${encodeURIComponent(t.name)}'">
            # ${t.name}
        </div>
    `).join("");
}



/*************************************************
 * 推荐创作者（本地缓存头像）
 *************************************************/
async function loadCreators() {
    const res = await Api.get(`/recommend/creators`);
    if (res.code !== 0) return;

    const box = document.getElementById("creators");

    let html = "";

    for (const c of res.data) {
        const avatar = await UserCache.getAvatar(c.avatar);

        html += `
            <div class="creator-item" onclick="location.href='/user.html?id=${c.id}'">
                <img src="${avatar}">
                <div class="creator-info">
                    <span>${c.username}</span>
                    <span>粉丝 ${c.followers}</span>
                </div>
            </div>
        `;
    }

    box.innerHTML = html;
}

/*************************************************
 * 搜索
 *************************************************/
document.getElementById("nav-search-btn").onclick = () => {
    const q = document.getElementById("nav-search-input").value.trim();
    if (q) location.href = `/search.html?q=${encodeURIComponent(q)}`;
};


/*************************************************
 * 无限滚动
 *************************************************/
window.addEventListener("scroll", () => {
    if (!loading && !finished) {
        const nearBottom = window.innerHeight + window.scrollY >= document.body.offsetHeight - 280;
        if (nearBottom) loadPosts();
    }
});

document.getElementById("tab-recommend").onclick = () => switchMode("recommend");
document.getElementById("tab-follow").onclick = () => switchMode("follow");

/*************************************************
 * 初始化
 *************************************************/
loadPosts();
loadTopics();
loadCreators();

/*************************************************
 * A. 随机拟声词飞入特效（100% 纯前端）
 *************************************************/
(function setupFloatingSFX() {
    const WORDS = ["ドン!", "バン!", "パァン!", "ズキューン!", "ピカッ!", "バサッ!", "ゴゴゴ…", "キラッ☆", "ドドドド!", "パッ!"];

    function spawnSFX() {
        const el = document.createElement("div");
        el.className = "floating-sfx";
        el.innerText = WORDS[Math.floor(Math.random() * WORDS.length)];

        // 随机从左侧或右侧飞入
        const fromLeft = Math.random() < 0.5;
        el.style.left = fromLeft ? "-60px" : "auto";
        el.style.right = fromLeft ? "auto" : "-60px";

        // 随机高度
        el.style.top = (Math.random() * window.innerHeight * 0.6 + 80) + "px";

        // 初始旋转
        el.style.transform = `rotate(${(Math.random() * 20 - 10)}deg)`;

        document.body.appendChild(el);

        // 强制刷新避免动画失效
        void el.offsetWidth;

        // 飞入动画
        el.classList.add("show");

        // 移除
        setTimeout(() => el.remove(), 2200);
    }

    // 每 3~6 秒随机出现一个
    setInterval(spawnSFX, 3000 + Math.random() * 3000);
})();


/*************************************************
 * B. 鼠标速度线拖尾（Canvas）
 *************************************************/
(function setupMouseTrail() {
    const canvas = document.createElement("canvas");
    canvas.id = "mouse-trail";
    canvas.style.position = "fixed";
    canvas.style.left = "0";
    canvas.style.top = "0";
    canvas.style.pointerEvents = "none";
    canvas.style.zIndex = "2";  // 在 speedlines 上层，但不挡元素
    document.body.appendChild(canvas);

    const ctx = canvas.getContext("2d");
    let w = canvas.width = window.innerWidth;
    let h = canvas.height = window.innerHeight;

    window.addEventListener("resize", () => {
        w = canvas.width = window.innerWidth;
        h = canvas.height = window.innerHeight;
    });

    let lastX = 0, lastY = 0;

    window.addEventListener("mousemove", e => {
        const dx = e.clientX - lastX;
        const dy = e.clientY - lastY;
        const dist = Math.sqrt(dx*dx + dy*dy);

        // 移动太小则不画
        if (dist > 6) {
            ctx.strokeStyle = "rgba(0,0,0,0.25)";
            ctx.lineWidth = 2;
            ctx.beginPath();
            ctx.moveTo(e.clientX, e.clientY);
            ctx.lineTo(e.clientX - dx * 0.4, e.clientY - dy * 0.4);
            ctx.stroke();
        }

        lastX = e.clientX;
        lastY = e.clientY;
    });

    // 动态淡出
    function fade() {
        ctx.fillStyle = "rgba(255,255,255,0.08)";
        ctx.fillRect(0, 0, w, h);
        requestAnimationFrame(fade);
    }
    fade();
})();

/*************************************************
 * D. 卡片 hover 拟声词喷出 + 漫画震动
 *************************************************/
(function setupCardHoverSFX() {
    const MINI_SFX = ["パッ!", "ピョン!", "キラ!", "バッ!", "ドン!"];

    function createMiniSFX(card) {
        const span = document.createElement("span");
        span.className = "mini-sfx-burst";
        span.innerText = MINI_SFX[Math.floor(Math.random() * MINI_SFX.length)];

        card.appendChild(span);

        // 动画结束清理
        setTimeout(() => span.remove(), 600);
    }

    // 每次新加载的卡片也会自动绑定
    const observer = new MutationObserver(mutations => {
        mutations.forEach(m => {
            m.addedNodes.forEach(node => {
                if (node.classList && node.classList.contains("manga-card")) {
                    node.addEventListener("mouseenter", () => {
                        node.classList.add("card-shake");
                        createMiniSFX(node);

                        setTimeout(() => node.classList.remove("card-shake"), 250);
                    });
                }
            });
        });
    });

    observer.observe(document.getElementById("post-list"), { childList: true });
})();

/*************************************************
 * E. Banner 光效扫描
 *************************************************/
(function setupBannerLight() {
    const banner = document.querySelector(".index-banner");
    if (!banner) return;

    const light = document.createElement("div");
    light.className = "banner-light-scan";
    banner.appendChild(light);

    function triggerScan() {
        light.classList.remove("run");
        void light.offsetWidth; // 强制刷新
        light.classList.add("run");
    }

    // 每 4–7 秒扫描一次
    setInterval(triggerScan, 4000 + Math.random() * 3000);
})();

function burst(x, y) {
    const star = document.createElement("div");
    star.className = "like-burst";
    star.style.left = x + "px";
    star.style.top = y + "px";
    star.innerText = "★";
    document.body.appendChild(star);
    setTimeout(() => star.remove(), 450);
}
const io = new IntersectionObserver(entries => {
    entries.forEach(e => {
        if (e.isIntersecting) {
            const img = e.target;
            img.src = img.dataset.src;
            io.unobserve(img);
        }
    })
});
