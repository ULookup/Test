import { Api } from "./core/api.js";
import { State } from "./core/state.js";
import { UserCache } from "./core/userCache.js";

const postList = document.getElementById("post-list");
const loadMore = document.getElementById("discover-load-more");

let page = 1;
let loading = false;
let finished = false;


async function loadPosts() {
    if (loading || finished) return;
    loading = true;

    const res = await Api.get(`/recommend/posts?page=${page}&page_size=10`);

    if (res.code !== 0) {
        console.log("加载失败");
        return;
    }

    const list = res.data;

    // ★★★ 如果本次返回数量 < page_size → 已经没有更多 ★★★
    if (Array.isArray(list) && list.length < 10) {
        for (const p of list) createPostCard(p);

        loadMore.classList.add("done");
        loadMore.innerText = "";
        finished = true;
        return;
    }

    // 空列表（第一页）
    if (!Array.isArray(list) || list.length === 0) {
        if (page === 1) {
            showEmpty();
        } else {
            loadMore.classList.add("done");
            loadMore.innerText = "";
        }
        finished = true;
        return;
    }

    for (const p of list) {
        createPostCard(p);
    }

    page++;
    loading = false;
}

/* 创建卡片 */
function createPostCard(p) {
    const div = document.createElement("div");
    div.className = "post-card";
    div.onclick = () => (location.href = `/post.html?id=${p.id}`);

    div.innerHTML = `
        <img src="${p.images?.[0] || '/static/noimg.png'}" class="post-cover">
        <div class="post-title">${p.title}</div>
        <div class="post-content">${p.content.slice(0, 60)}...</div>

        <div class="post-meta">
            <img src="${p.author_avatar}" class="meta-avatar">
            <span>${p.author_name}</span>
        </div>
    `;

    postList.appendChild(div);
}

/* 空提示 */
function showEmpty() {
    postList.innerHTML = `
        <div id="discover-empty">（＞﹏＜）暂时还没有推荐内容…</div>
    `;
}

/* ========== 热门话题 ========== */
/* ========== 热门话题（适配后端完整 Topic 结构） ========== */
async function loadTopics() {
    const res = await Api.get(`/topics/hot`);
    if (res.code !== 0) return;

    // ★ 自动检测可用字段
    const raw = res.data;
    const topics =
        Array.isArray(raw) ? raw :
        Array.isArray(raw?.list) ? raw.list :
        Array.isArray(raw?.topics) ? raw.topics :
        [];

    const box = document.getElementById("hot-topics");

    if (topics.length === 0) {
        box.innerHTML = `<div class="topic-empty">（＞﹏＜）暂时没有热门话题</div>`;
        return;
    }

    box.innerHTML = topics.map(t => `
        <div class="topic-item"
            onclick="location.href='/search.html?q=${encodeURIComponent(t.name)}'">
            
            <div class="topic-name">#${t.name}</div>

            <div class="topic-heat-bar">
                <div class="topic-heat-fill"
                     style="width:${Math.min(t.heat, 100)}%;"></div>
            </div>

            <div class="topic-heat-text">${t.heat}</div>
        </div>
    `).join("");
}



/* ========== 推荐创作者 ========== */
async function loadCreators() {
    const res = await Api.get(`/recommend/creators`);
    if (res.code !== 0) return;

    const box = document.getElementById("creators");
    box.innerHTML = res.data
        .map(u => `
            <div class="creator-item" onclick="location.href='/user.html?id=${u.id}'">
                <img src="${u.avatar}">
                <div>
                    <div class="creator-name">${u.username}</div>
                    <div class="creator-fans">粉丝 ${u.fans}</div>
                </div>
            </div>
        `)
        .join("");
}

/* 无限滚动 */
window.addEventListener("scroll", () => {
    const nearBottom =
        window.innerHeight + window.scrollY >= document.body.scrollHeight - 300;
    if (nearBottom) loadPosts();
});

/* 初始化 */
loadPosts();
loadTopics();
loadCreators();   // ★★ 你漏掉的关键行
