import { Api } from "./core/api.js";
import { State } from "./core/state.js";
import "./core/navbar.js";     // 导航栏自己管理自己的事件
import { AssetCache } from "./core/userCache.js";

/* ============ 搜索输入初始化 ============ */

const qs = new URLSearchParams(location.search);
let keyword = qs.get("q") || "";
document.getElementById("search-input").value = keyword;

let type = "posts";
let page = 1;
let loading = false;
let finished = false;

const resultBox = document.getElementById("result-box");
const loadMore = document.getElementById("load-more");


/* ============ 搜索按钮（仅处理页面内部输入框）============ */
document.getElementById("search-btn").onclick = () => {
    const q = document.getElementById("search-input").value.trim();
    if (!q) return;
    location.href = `/search.html?q=${encodeURIComponent(q)}`;
};


/* ============ Tab 切换 ============ */
/* 已加载记录，避免重复请求 */

/* ============ Tab 切换 ============ */
document.querySelectorAll(".tab").forEach(tab => {
    tab.onclick = () => {

        // UI 切换
        document.querySelectorAll(".tab").forEach(x => x.classList.remove("active"));
        tab.classList.add("active");

        type = tab.dataset.type;

        // ========== 重置分页、状态 ==========
        page = 1;
        finished = false;
        loading = false;       // ★ 修复：必须重置 loading
        resultBox.innerHTML = "";

        // ========== 始终同步搜索词 ==========
        const currentKeyword = document.getElementById("search-input").value.trim();
        keyword = currentKeyword;   // ★ 修复：必须更新 keyword

        console.log(`[Tab 切换] 类型=${type}, keyword="${keyword}"`);

        // ========== 首次加载 or 继续加载 ==========
        loadResults();
    };
});




/* ============ 渲染帖子 ============ */
function renderPost(p) {
    const div = document.createElement("div");
    div.className = "post-card";
    div.onclick = () => location.href = `/post.html?id=${p.id}`;

    const cover = p.images && p.images.length > 0 ? p.images[0] : null;

    div.innerHTML = `
        <div class="post-row">
            ${cover ? 
                `<img src="${cover}" class="post-cover">` :
                `<div class="post-cover placeholder">漫画</div>`
            }

            <div class="post-info">
                <div class="post-title">${p.title}</div>
                <div class="post-content">${p.content.slice(0, 80)}...</div>
            </div>
        </div>
    `;
    resultBox.appendChild(div);
}


/* ============ 渲染用户 ============ */
function renderUser(u) {
    const div = document.createElement("div");
    div.className = "user-card";
    div.onclick = () => location.href = `/user.html?id=${u.id}`;

    div.innerHTML = `
        <div class="user-row">
            <img src="${u.avatar}" class="user-avatar">
            <div class="user-name">${u.username}</div>
        </div>
    `;
    resultBox.appendChild(div);
}


/* ============ 加载结果 ============ */
async function loadResults() {
    if (loading || finished || !keyword) return;

    loading = true;
    loadMore.innerText = "加载中...";

    const res = await Api.get(
        `/search/${type}?q=${encodeURIComponent(keyword)}&page=${page}&page_size=10`
    );

    if (res.code !== 0) {
        loadMore.innerText = "加载失败";
        loading = false;
        return;
    }

    // ===============================
    // 兼容两种后端格式
    // ===============================
    const list = Array.isArray(res.data?.list)
        ? res.data.list
        : Array.isArray(res.data)
            ? res.data
            : [];

    console.log(`[搜索结果] type=${type}`, list);

    // 空结果（第一页）
    if (page === 1 && list.length === 0) {
        await showEmpty();
        finished = true;
        return;
    }

    // 没有更多
    if (list.length === 0) {
        loadMore.innerText = "没有更多内容";
        finished = true;
        return;
    }

    // 渲染
    for (const item of list) {
        type === "posts" ? renderPost(item) : renderUser(item);
    }

    page++;
    loading = false;
}



async function showEmpty() {
    const emptyImg = await AssetCache.get("/static/empty_q.png");

    resultBox.innerHTML = `
        <div class="empty-box">
            <div class="empty-bubble">
                <div class="empty-sfx">ドーン!</div>
                <img src="${emptyImg}" class="empty-img">
                <div class="empty-text">什么都没有找到…</div>
            </div>
        </div>
    `;
    loadMore.style.display = "none";
}


/* 无限滚动 */
window.addEventListener("scroll", () => {
    const nearBottom =
        window.innerHeight + window.scrollY >= document.body.offsetHeight - 200;

    if (nearBottom) loadResults();
});


/* 初始化 */
loadResults();
