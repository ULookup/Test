import { Api } from "./core/api.js";
import { State } from "./core/state.js";

// =============================
// 用户缓存模块（避免重复请求）
// =============================
const UserCache = {
    map: new Map(),

    async getUser(id) {
        if (this.map.has(id)) return this.map.get(id);

        const res = await Api.get(`/api/user/${id}`);
        if (res.code !== 0) return null;

        this.map.set(id, res.data);
        return res.data;
    }
};


// =============================
// 登录状态 UI
// =============================
async function initLoginUI() {
    const nav = document.getElementById("nav-right");

    if (!State.isLogin()) {
        nav.innerHTML = `<button class="login-btn" onclick="location.href='/login.html'">登录</button>`;
        return;
    }

    const uid = State.getUserId();
    const u = await UserCache.getUser(uid);

    nav.innerHTML = `
        <img src="${u.avatar}" class="nav-avatar" onclick="location.href='/account.html'">
    `;
}


// =============================
// 搜索跳转
// =============================
document.getElementById("search-btn").onclick = () => {
    const q = document.getElementById("search-input").value.trim();
    if (q) location.href = `/search.html?q=${encodeURIComponent(q)}`;
};


// =============================
// 帖子列表加载（无限滚动）
// =============================
let page = 1;
let loading = false;
let finished = false;

const postList = document.getElementById("post-list");
const loadMore = document.getElementById("load-more");

async function loadPosts() {
    if (loading || finished) return;
    loading = true;
    loadMore.innerText = "加载中...";

    const res = await Api.get(`/api/posts?page=${page}&page_size=10`);
    if (res.code !== 0) return;

    const list = res.data.list;
    if (list.length === 0) {
        finished = true;
        loadMore.innerText = "没有更多帖子了";
        return;
    }

    for (const post of list) {
        await renderPostCard(post);
    }

    page++;
    loading = false;
}

async function renderPostCard(post) {
    const author = await UserCache.getUser(post.author_id);

    const div = document.createElement("div");
    div.className = "post-card";
    div.onclick = () => location.href = `/post.html?id=${post.id}`;

    div.innerHTML = `
        <div class="post-title">${post.title}</div>
        <div class="post-sub">
            <img class="author-avatar" src="${author.avatar}">
            <span class="author-name">${author.nickname}</span>
        </div>
        <div class="post-content">${post.content.slice(0, 80)}...</div>
        ${post.images[0] ? `<img class="post-cover" src="${post.images[0]}">` : ""}
        <div class="tags">${post.tags.map(t => `<div class="tag">${t}</div>`).join("")}</div>
        <div class="post-actions">
            <span>❤️ ${post.like_count}</span>
            <span>⭐ ${post.fav_count}</span>
            <span>💬 ${post.comment_count}</span>
        </div>
    `;

    postList.appendChild(div);
}


// 无限滚动监听
window.addEventListener("scroll", () => {
    if (window.innerHeight + window.scrollY >= document.body.offsetHeight - 200) {
        loadPosts();
    }
});


// =============================
// 右侧栏加载
// =============================
async function loadHotTopics() {
    const res = await Api.get("/api/topics/hot");
    if (res.code !== 0) return;

    const box = document.getElementById("hot-topics");
    box.innerHTML = res.data.map(t => `<div class="side-item"># ${t}</div>`).join("");
}

async function loadRecommendPosts() {
    const res = await Api.get("/api/recommend/posts");
    if (res.code !== 0) return;

    const box = document.getElementById("recommend-posts");
    box.innerHTML = res.data.map(p => `
        <div class="side-item" onclick="location.href='/post.html?id=${p.id}'">${p.title}</div>
    `).join("");
}

async function loadRecommendCreators() {
    const res = await Api.get("/api/recommend/creators");
    if (res.code !== 0) return;

    const box = document.getElementById("recommend-creators");

    box.innerHTML = res.data.map(u => `
        <div class="creator-item" onclick="location.href='/user.html?id=${u.id}'">
            <img src="${u.avatar}">
            <div class="creator-info">
                <div class="creator-name">${u.nickname}</div>
                <div class="creator-fans">粉丝 ${u.fans}</div>
            </div>
        </div>
    `).join("");
}


// =============================
// 初始化
// =============================
initLoginUI();
loadPosts();
loadHotTopics();
loadRecommendPosts();
loadRecommendCreators();
