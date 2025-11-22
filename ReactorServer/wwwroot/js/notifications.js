import { Api } from "./core/api.js";
import { State } from "./core/state.js";
import { UserCache } from "./core/userCache.js";

// ===== 导航栏 =====
async function renderNavbar() {
    const nav = document.getElementById("nav-right");
    if (!nav) return;

    if (!State.isLogin()) {
        nav.innerHTML = `
            <a href="/login.html" class="nav-btn">登录</a>
            <a href="/register.html" class="nav-btn">注册</a>
        `;
        return;
    }

    const uid = State.getUserId();
    const user = await UserCache.getUser(uid);

    nav.innerHTML = `
        <div class="nav-user" id="nav-user">
            <img src="${user.avatar}" class="nav-avatar">
        </div>
    `;

    document.getElementById("nav-user").onclick = () => {
        location.href = `/user.html?id=${uid}`;
    };
}

// ===== 分页状态 =====
let page = 1;
let loading = false;
let finished = false;

const listBox = document.getElementById("noti-list");
const loadMore = document.getElementById("load-more");

// 时间格式化
function timeFormat(ts) {
    const diff = (Date.now() - ts * 1000) / 1000;
    if (diff < 60) return "刚刚";
    if (diff < 3600) return `${Math.floor(diff / 60)}分钟前`;
    if (diff < 86400) return `${Math.floor(diff / 3600)}小时前`;
    return `${Math.floor(diff / 86400)}天前`;
}

// ===== 加载通知 =====
async function loadNotifications() {
    if (loading || finished) return;
    loading = true;

    loadMore.innerText = "加载中...";

    const res = await Api.get(`/notifications?page=${page}&page_size=10`);

    if (res.code !== 0) {
        loadMore.innerText = "加载失败";
        return;
    }

    const list = res.data.list;
    if (!list || list.length === 0) {
        finished = true;
        loadMore.innerText = "没有更多通知了";
        return;
    }

    for (const n of list) {
        renderNotification(n);
    }

    page++;
    loading = false;
}

// ===== 渲染单条通知 =====
function renderNotification(n) {
    const div = document.createElement("div");
    div.className = `noti-item ${n.read ? "" : "unread"}`;

    div.innerHTML = `
        <div class="noti-title">${n.title}</div>
        <div class="noti-content">${n.content}</div>
        <div class="noti-time">${timeFormat(n.time)}</div>
    `;

    div.onclick = async () => {
        if (!n.read) {
            await Api.post(`/notifications/${n.id}/read`);
            div.classList.remove("unread");
        }

        if (n.link) location.href = n.link;
    };

    listBox.appendChild(div);
}

// ===== 全部标为已读 =====
document.getElementById("read-all-btn").onclick = async () => {
    const res = await Api.post(`/notifications/read_all`);

    if (res.code === 0) {
        document.querySelectorAll(".noti-item").forEach(el => {
            el.classList.remove("unread");
        });
        alert("已全部标记为已读");
    } else {
        alert(res.msg);
    }
};

// ===== 无限滚动 =====
window.addEventListener("scroll", () => {
    const bottom = window.innerHeight + window.scrollY >= document.body.offsetHeight - 200;
    if (bottom) loadNotifications();
});

// ===== 初始化 =====
renderNavbar();
loadNotifications();
