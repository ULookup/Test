import { Api } from "./core/api.js";
import { State } from "./core/state.js";
import { UserCache } from "./core/userCache.js";

let uid = null;

async function init() {

    uid = parseInt(new URLSearchParams(location.search).get("id"));
    if (!uid) {
        document.getElementById("content").innerHTML = "无效用户";
        return;
    }

    const user = await UserCache.getUser(uid);
    if (!user) {
        document.getElementById("content").innerHTML = "用户不存在";
        return;
    }

    // 个人主页：隐藏关注按钮
    if (State.getUserId() == uid) {
        document.getElementById("follow-btn").style.display = "none";
        const editBtn = document.getElementById("edit-btn");
        editBtn.style.display = "inline-block";
        editBtn.onclick = () => (location.href = "/account.html");
    }

    // 渲染基本信息
    document.getElementById("user-avatar").src = user.avatar;
    document.getElementById("user-name").innerText = user.username;
    document.getElementById("user-bio").innerText = user.bio || "这个人很安静，没有写简介";

    document.getElementById("post-count").innerText = user.post_count;
    document.getElementById("follower-count").innerText = user.follower_count;
    document.getElementById("following-count").innerText = user.following_count;

    initFollowButton(user);
    initTabs();
    bindUserActionButtons();  // ★★★★★ 关键！
    loadPosts();              // 默认加载帖子
}



/* ------------------------ 关注 ------------------------ */
function initFollowButton(user) {
    const btn = document.getElementById("follow-btn");

    if (State.getUserId() == uid) {
        btn.style.display = "none";
        return;
    }

    btn.onclick = async () => {
        const res = await Api.post(`/user/${uid}/follow`);
        if (res.code === 0) btn.innerText = "已关注";
    };
}


/* ------------------------ Tabs 切换 ------------------------ */
function initTabs() {
    const tabs = document.querySelectorAll(".tab");

    tabs.forEach(tab => {
        tab.addEventListener("click", () => {
            // 切换 tab 的选中状态
            tabs.forEach(t => t.classList.remove("active"));
            tab.classList.add("active");

            // 清空内容区域
            const content = document.getElementById("content");
            content.innerHTML = "";

            // 根据 data-tab 加载对应内容
            const tag = tab.dataset.tab;
            if (tag === "posts") loadPosts();
            if (tag === "likes") loadLikes();
            if (tag === "favs") loadFavs();
            if (tag === "followers") loadFollowers();
            if (tag === "following") loadFollowing();
        });
    });
}


/* ------------------------ 用户帖子 ------------------------ */
async function loadPosts() {
    const res = await Api.get(`/posts?user_id=${uid}&page=1&size=20`);
    if (res.code !== 0) return;

    const list = res.data.list;
    const content = document.getElementById("content");

    list.forEach(p => {
        const div = document.createElement("div");
        div.className = "post-item";
        div.innerHTML = `
            <h3>${p.title}</h3>
            <p>${p.content.substring(0, 80)}</p>
        `;
        div.onclick = () => (location.href = `/post.html?id=${p.id}`);
        content.appendChild(div);
    });
}


/* ------------------------ 用户喜欢的帖子 ------------------------ */
async function loadLikes() {
    const res = await Api.get(`/user/${uid}/likes`);
    if (res.code !== 0) return;

    const list = res.data;
    const content = document.getElementById("content");

    if (list.length === 0) {
        content.innerHTML = `<div class="empty">还没有喜欢任何帖子~</div>`;
        return;
    }

    list.forEach(p => {
        const div = document.createElement("div");
        div.className = "post-item";
        div.innerHTML = `
            <h3>${p.title}</h3>
            <p>${p.content.substring(0, 80)}</p>
        `;
        div.onclick = () => (location.href = `/post.html?id=${p.id}`);
        content.appendChild(div);
    });
}


/* ------------------------ 用户收藏的帖子 ------------------------ */
async function loadFavs() {
    const res = await Api.get(`/user/${uid}/favs`);
    if (res.code !== 0) return;

    const list = res.data;
    const content = document.getElementById("content");

    if (list.length === 0) {
        content.innerHTML = `<div class="empty">还没有收藏任何帖子~</div>`;
        return;
    }

    list.forEach(p => {
        const div = document.createElement("div");
        div.className = "post-item";
        div.innerHTML = `
            <h3>${p.title}</h3>
            <p>${p.content.substring(0, 80)}</p>
        `;
        div.onclick = () => (location.href = `/post.html?id=${p.id}`);
        content.appendChild(div);
    });
}


/* ------------------------ 粉丝 ------------------------ */
async function loadFollowers() {
    const res = await Api.get(`/user/${uid}/followers`);
    if (res.code !== 0) return;
    const content = document.getElementById("content");

    res.data.forEach(u => {
        const div = document.createElement("div");
        div.className = "follow-item";
        div.innerHTML = `
            <img src="${u.avatar}">
            <span>${u.username}</span>
        `;
        div.onclick = () => (location.href = `/user.html?id=${u.id}`);
        content.appendChild(div);
    });
}


/* ------------------------ 关注中 ------------------------ */
async function loadFollowing() {
    const res = await Api.get(`/user/${uid}/following`);
    if (res.code !== 0) return;
    const content = document.getElementById("content");

    res.data.forEach(u => {
        const div = document.createElement("div");
        div.className = "follow-item";
        div.innerHTML = `
            <img src="${u.avatar}">
            <span>${u.username}</span>
        `;
        div.onclick = () => (location.href = `/user.html?id=${u.id}`);
        content.appendChild(div);
    });
}

function bindUserActionButtons() {
    const btns = document.querySelectorAll(".user-actions button");

    btns.forEach(btn => {
        btn.addEventListener("click", () => {
            const tag = btn.dataset.target;

            if (tag === "noti") {
                location.href = "/notifications.html";
                return;
            }

            switchToTab(tag);
        });
    });
}

/* ------------------------ 外部跳转 Tab ------------------------ */
function switchToTab(name) {
    const tab = document.querySelector(`.tab[data-tab="${name}"]`);
    if (tab) tab.click();
}

init();
