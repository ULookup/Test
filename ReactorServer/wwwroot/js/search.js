// 获取搜索关键词
const qs = new URLSearchParams(location.search);
const keyword = qs.get("q") || "";

// DOM
const postBox = document.getElementById("post-results");
const userBox = document.getElementById("user-results");

// 输入框自动填入当前关键词
document.getElementById("search-input").value = keyword;

// 搜索按钮重定向
document.getElementById("search-btn").onclick = () => {
    const q = document.getElementById("search-input").value.trim();
    if (!q) return;
    location.href = `/search.html?q=${encodeURIComponent(q)}`;
};


// 加载帖子搜索
async function loadPosts() {
    if (!keyword) return;

    const res = await fetch(`/api/search/posts?q=${encodeURIComponent(keyword)}`);
    const json = await res.json();
    if (json.code !== 0) return;

    postBox.innerHTML = json.data.map(p => `
        <div class="post-card" onclick="location.href='/post.html?id=${p.id}'">
            <div class="post-title">${p.title}</div>
            <div class="post-content">${p.content.slice(0, 80)}...</div>

            <div class="tags">
                ${p.tags.map(t => `<div class="tag">${t}</div>`).join("")}
            </div>
        </div>
    `).join("");
}


// 加载用户搜索
async function loadUsers() {
    if (!keyword) return;

    const res = await fetch(`/api/search/users?q=${encodeURIComponent(keyword)}`);
    const json = await res.json();
    if (json.code !== 0) return;

    userBox.innerHTML = json.data.map(u => `
        <div class="user-card" onclick="location.href='/user.html?id=${u.id}'">
            <img src="${u.avatar}" class="user-avatar">

            <div class="user-body">
                <div class="user-name">${u.nickname}</div>
                <div class="user-signature">${u.signature || "这个人很神秘..."}</div>
            </div>
        </div>
    `).join("");
}

// 初始化
loadPosts();
loadUsers();
