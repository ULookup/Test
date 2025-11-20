// 获取用户 ID
const qs = new URLSearchParams(location.search);
const uid = qs.get("id");

// DOM
const userBox = document.getElementById("user-info");
const postList = document.getElementById("post-list");
const loadMore = document.getElementById("load-more");

// 登录用户
const login_uid = localStorage.getItem("user_id");

// 时间格式化
function format(ts) {
    const diff = (Date.now() - ts * 1000) / 1000;
    if (diff < 60) return "刚刚";
    if (diff < 3600) return Math.floor(diff / 60) + "分钟前";
    if (diff < 86400) return Math.floor(diff / 3600) + "小时前";
    return Math.floor(diff / 86400) + "天前";
}

// 渲染用户信息
async function loadUser() {
    const res = await fetch(`/api/user/${uid}`);
    const j = await res.json();
    if (j.code !== 0) return;

    const u = j.data;

    userBox.innerHTML = `
        <img src="${u.avatar}" class="avatar">
        <div class="nickname">${u.nickname}</div>
        <div class="signature">${u.signature || "这个人很神秘……"}</div>

        <div class="stats">
            <div class="stat">
                <div class="stat-num">${u.fans}</div>
                <div class="stat-title">粉丝</div>
            </div>
            <div class="stat">
                <div class="stat-num">${u.following}</div>
                <div class="stat-title">关注</div>
            </div>
            <div class="stat">
                <div class="stat-num">${u.post_count}</div>
                <div class="stat-title">帖子</div>
            </div>
        </div>

        ${
            login_uid && login_uid != uid
            ? u.is_follow
                ? `<button id="follow-btn" class="btn-unfollow">取消关注</button>`
                : `<button id="follow-btn" class="btn-follow">关注</button>`
            : ""
        }
    `;

    const btn = document.getElementById("follow-btn");
    if (btn) {
        btn.onclick = async () => {
            if (u.is_follow) {
                await fetch(`/api/user/${uid}/unfollow`, { method: "POST" });
            } else {
                await fetch(`/api/user/${uid}/follow`, { method: "POST" });
            }
            loadUser(); // 重新渲染
        };
    }
}

// 加载帖子
let page = 1;
let finished = false;
let loading = false;

async function loadPosts() {
    if (loading || finished) return;

    loading = true;
    loadMore.innerText = "加载中...";

    const res = await fetch(`/api/user/${uid}/posts?page=${page}&page_size=10`);
    const j = await res.json();

    if (j.code !== 0) return;

    const list = j.data.list;
    if (list.length === 0) {
        finished = true;
        loadMore.innerText = "没有更多帖子了";
        return;
    }

    for (let p of list) {
        const div = document.createElement("div");
        div.className = "post-card";
        div.onclick = () => location.href = `/post.html?id=${p.id}`;

        div.innerHTML = `
            <div class="post-title">${p.title}</div>
            <div class="post-content">${p.content.slice(0, 80)}...</div>
        `;

        postList.appendChild(div);
    }

    page++;
    loading = false;
}

// 无限滚动
window.addEventListener("scroll", () => {
    const nearBottom = window.innerHeight + window.scrollY >= document.body.offsetHeight - 200;
    if (nearBottom) loadPosts();
});

// 登录状态（右上角）
const navRight = document.getElementById("nav-right");
if (!login_uid) {
    navRight.innerHTML = `<button class="btn-blue" onclick="location.href='/login.html'">登录</button>`;
} else {
    fetch(`/api/user/${login_uid}`)
        .then(r => r.json())
        .then(j => {
            navRight.innerHTML = `
                <img src="${j.data.avatar}" 
                     style="width:36px;height:36px;border-radius:50%;cursor:pointer;"
                     onclick="location.href='/account.html'">
            `;
        });
}

// 初始化
loadUser();
loadPosts();
