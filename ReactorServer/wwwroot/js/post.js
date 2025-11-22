import { Api } from "./core/api.js";
import { State } from "./core/state.js";
import { UI } from "./core/ui.js";

/* =============================
   工具函数：解析 URL 参数
============================= */
function getQueryParam(name) {
    return new URL(location.href).searchParams.get(name);
}

const postId = parseInt(getQueryParam("id"));
if (!postId) {
    alert("帖子 ID 不合法");
    location.href = "/index.html";
}


/* =============================
   加载帖子数据
============================= */
let currentPost = null;
let authorInfo = null;

async function loadPost() {
    const box = document.getElementById("post-box");
    box.innerHTML = `<div class="loading">加载帖子中...</div>`;

    const res = await Api.get(`/post/${postId}`);
    if (res.code !== 0) {
        box.innerHTML = `<div class="error">帖子不存在或已被删除</div>`;
        return;
    }

    currentPost = res.data;

    // 加载作者信息
    const author = await Api.get(`/user/${currentPost.author_id}`);
    authorInfo = author.data;

    renderPost();
}

/* =============================
   渲染帖子内容
============================= */
function renderPost() {
    const box = document.getElementById("post-box");

    const imgs = currentPost.images
        .map(url => `<img src="${url}" class="post-image">`)
        .join("");

    box.innerHTML = `
        <h1 class="post-title">${currentPost.title}</h1>

        <div class="post-author">
            <img src="${authorInfo.avatar}" class="author-avatar"
                 onclick="location.href='/user.html?id=${authorInfo.id}'">

            <div>
                <div class="author-name"
                     onclick="location.href='/user.html?id=${authorInfo.id}'">
                    ${authorInfo.username}
                </div>
                <div class="post-time">${formatTime(currentPost.create_time)}</div>
            </div>
        </div>

        <div class="post-content">${currentPost.content}</div>

        <div class="post-images">${imgs}</div>

        <!-- 点赞 / 收藏 -->
        <div class="post-like-box">
            <div id="like-btn" class="like-btn">
                <span class="like-icon">${currentPost.liked ? "❤" : "♡"}</span>
                <span class="like-number">${currentPost.likes}</span>
            </div>

            <div id="fav-btn" class="fav-btn">
                <span class="fav-icon">${currentPost.faved ? "★" : "☆"}</span>
                <span class="fav-number">${currentPost.favs}</span>
            </div>
        </div>
    `;

    bindImageViewer();
    initLikeState();
    initFavState();
}

/* =============================
   图片大图 Lightbox
============================= */
function bindImageViewer() {
    const viewer = document.getElementById("img-viewer");
    const viewerImg = document.getElementById("img-viewer-img");

    document.querySelectorAll(".post-image").forEach(img => {
        img.style.cursor = "zoom-in";
        img.onclick = () => {
            viewerImg.src = img.src;
            viewer.classList.add("show");
        };
    });

    viewer.onclick = () => viewer.classList.remove("show");
}

/* =============================
   点赞初始化 & 事件
============================= */
function initLikeState() {
    const btn = document.getElementById("like-btn");
    const count = btn.querySelector(".like-number");

    UI.updateLike(btn, currentPost.likes, currentPost.liked);

    btn.onclick = async () => {
        if (!State.isLogin()) return alert("请先登录");

        const liked = btn.classList.contains("active");
        const url = liked ? `/post/${postId}/unlike` : `/post/${postId}/like`;

        const res = await Api.post(url);
        if (res.code !== 0) return;

        const newLiked = !liked;
        const newCount = parseInt(count.textContent) + (newLiked ? 1 : -1);

        UI.updateLike(btn, newCount, newLiked);
        UI.pop(btn);
    };
}

/* =============================
   收藏初始化 & 事件
============================= */
function initFavState() {
    const btn = document.getElementById("fav-btn");
    const count = btn.querySelector(".fav-number");

    UI.updateFav(btn, currentPost.favs, currentPost.faved);

    btn.onclick = async () => {
        if (!State.isLogin()) return alert("请先登录");

        const faved = btn.classList.contains("active");
        const url = faved ? `/post/${postId}/unfav` : `/post/${postId}/fav`;

        const res = await Api.post(url);
        if (res.code !== 0) return;

        const newFaved = !faved;
        const newCount = parseInt(count.textContent) + (newFaved ? 1 : -1);

        UI.updateFav(btn, newCount, newFaved);
        UI.pop(btn);
    };
}

/* =============================
   评论区加载
============================= */
async function loadComments() {
    const list = document.getElementById("comment-list");
    list.innerHTML = `<div class="loading">加载评论...</div>`;

    const res = await Api.get(`/post/${postId}/comments`);

    if (res.code !== 0) {
        list.innerHTML = `<div class="error">评论加载失败</div>`;
        return;
    }

    const comments = res.data;

    if (comments.length === 0) {
        list.innerHTML = `<div class="empty">还没有评论~</div>`;
        return;
    }

    list.innerHTML = comments.map(c => `
        <div class="comment-item">
            <img src="${c.avatar}" class="comment-avatar"
                 onclick="location.href='/user.html?id=${c.uid}'">

            <div class="comment-body">
                <div class="comment-user"
                     onclick="location.href='/user.html?id=${c.uid}'">
                    ${c.username}
                </div>

                <div class="comment-text">${c.content}</div>
                <div class="comment-time">${formatTime(c.create_time)}</div>

                <div class="comment-like-box">
                    <span class="comment-like-btn ${c.liked ? "liked" : ""}" data-id="${c.id}">
                        👍
                    </span>
                    <span class="comment-like-count">${c.likes}</span>
                </div>
            </div>
        </div>
    `).join("");

    bindCommentLike();
}

/* =============================
   评论点赞
============================= */
function bindCommentLike() {
    document.querySelectorAll(".comment-like-btn").forEach(btn => {
        btn.onclick = async () => {
            if (!State.isLogin()) return alert("请先登录");

            const cid = btn.dataset.id;
            const countEl = btn.nextElementSibling;
            const liked = btn.classList.contains("liked");

            const url = liked
                ? `/comment/${cid}/unlike`
                : `/comment/${cid}/like`;

            const res = await Api.post(url);
            if (res.code !== 0) return;

            const newLiked = !liked;
            const newCount =
                parseInt(countEl.textContent) + (newLiked ? 1 : -1);

            // 更新前端
            if (newLiked) btn.classList.add("liked");
            else btn.classList.remove("liked");

            countEl.textContent = newCount;

            UI.pop(btn);
        };
    });
}

/* =============================
   发布评论
============================= */
async function submitComment() {
    if (!State.isLogin()) return alert("请先登录");

    const text = document.getElementById("comment-input").value.trim();
    if (!text) return alert("评论不能为空");

    const res = await Api.post(`/post/${postId}/comment`, { content: text });

    if (res.code === 0) {
        document.getElementById("comment-input").value = "";
        loadComments();
    } else {
        alert(res.msg);
    }
}

/* =============================
   工具函数：时间格式化
============================= */
function formatTime(ts) {
    const d = new Date(ts * 1000);
    return `${d.getFullYear()}-${d.getMonth() + 1}-${d.getDate()} ${
        String(d.getHours()).padStart(2, "0")
    }:${String(d.getMinutes()).padStart(2, "0")}`;
}

/* =============================
   启动流程
============================= */
loadPost();
loadComments();

document.getElementById("comment-btn").onclick = submitComment;
