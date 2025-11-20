// 时间格式化
function format(ts) {
    const diff = (Date.now() - ts * 1000) / 1000;
    if (diff < 60) return "刚刚";
    if (diff < 3600) return Math.floor(diff / 60) + "分钟前";
    if (diff < 86400) return Math.floor(diff / 3600) + "小时前";
    return Math.floor(diff / 86400) + "天前";
}

// UI 渲染
const listBox = document.getElementById("notif-list");

// 加载通知
async function loadNotifications() {
    const res = await fetch("/api/notifications");
    const json = await res.json();
    if (json.code !== 0) return;

    const list = json.data;

    listBox.innerHTML = "";

    for (let n of list) {
        const user = await (await fetch(`/api/user/${n.from_user_id}`)).json();
        const u = user.data;

        const div = document.createElement("div");
        div.className = "notif-card";

        div.onclick = () => {
            markRead(n.id);

            // 跳转到帖子详情
            if (n.target_post_id) {
                location.href = `/post.html?id=${n.target_post_id}`;
                return;
            }
        };

        div.innerHTML = `
            <img src="${u.avatar}" class="notif-avatar">
            <div class="notif-body">
                <div class="notif-user">${u.nickname}</div>
                <div class="notif-text">${n.content}</div>
                <div class="notif-time">${format(n.time)}</div>
            </div>

            ${n.is_read ? "" : `<div class="unread-dot"></div>`}
        `;

        listBox.appendChild(div);
    }
}

// 标记已读
async function markRead(id) {
    await fetch(`/api/notifications/${id}/read`, {
        method: "POST"
    });
}

// 登录状态（右上角）
const navRight = document.getElementById("nav-right");

const token = localStorage.getItem("token");
const uid = localStorage.getItem("user_id");

if (!token || !uid) {
    navRight.innerHTML = `<button onclick="location.href='/login.html'" class="btn-blue">登录</button>`;
} else {
    fetch(`/api/user/${uid}`)
      .then(r => r.json())
      .then(j => {
          navRight.innerHTML = `
              <img src="${j.data.avatar}"
                   style="width:36px;height:36px;border-radius:50%;cursor:pointer;"
                   onclick="location.href='/user.html?id=${j.data.id}'">
          `;
      });
}

// 初始化
loadNotifications();
