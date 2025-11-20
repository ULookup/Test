import { Api } from "./api.js";
import { State } from "./state.js";

// 获取帖子 id
const qs = new URLSearchParams(location.search);
const postId = qs.get("id");

// DOM
const postBox = document.getElementById("post-detail");
const commentList = document.getElementById("comment-list");

// 时间格式化
function format(ts) {
  const diff = (Date.now() - ts * 1000) / 1000;
  if (diff < 60) return "刚刚";
  if (diff < 3600) return Math.floor(diff / 60) + "分钟前";
  if (diff < 86400) return Math.floor(diff / 3600) + "小时前";
  return Math.floor(diff / 86400) + "天前";
}

// 加载帖子详情
async function loadPost() {
  const res = await fetch(`/api/post/${postId}`);
  const json = await res.json();
  if (json.code !== 0) return;

  const p = json.data;

  // 加载作者信息
  const u = await (await fetch(`/api/user/${p.author_id}`)).json();
  const author = u.data;

  postBox.innerHTML = `
    <h1 class="post-title">${p.title}</h1>

    <div class="post-info">
      <img src="${author.avatar}" class="author-avatar">
      <span class="author-name">${author.nickname}</span>
      <span class="post-time">${format(p.create_time)}</span>
    </div>

    <div class="post-content">${p.content}</div>

    <div class="image-list">
      ${p.images.map(url => `<img src="${url}">`).join("")}
    </div>

    <div class="tags">
      ${p.tags.map(t => `<div class="tag">${t}</div>`).join("")}
    </div>

    <div class="actions">
      <div id="like-btn" class="action">❤️ ${p.like_count}</div>
      <div id="fav-btn" class="action">⭐ ${p.fav_count}</div>
      <div class="action">💬 ${p.comment_count}</div>
    </div>
  `;

  initActions(p);
}

// 点赞收藏逻辑
function initActions(p) {
  document.getElementById("like-btn").onclick = async () => {
    await fetch(`/api/post/${postId}/like`, { method: "POST" });
    location.reload();
  };

  document.getElementById("fav-btn").onclick = async () => {
    await fetch(`/api/post/${postId}/fav`, { method: "POST" });
    location.reload();
  };
}

// 加载评论
async function loadComments() {
  const res = await fetch(`/api/post/${postId}/comments`);
  const json = await res.json();
  if (json.code !== 0) return;

  commentList.innerHTML = json.data.map(c => `
    <div class="comment">
      <img src="${c.author_avatar}" class="comment-avatar">
      <div class="comment-body">
        <div>
          <span class="comment-name">${c.author_nickname}</span>
          <span class="comment-time">${format(c.time)}</span>
        </div>
        <div class="comment-text">${c.content}</div>
      </div>
    </div>
  `).join("");
}

// 发表评论
document.getElementById("comment-btn").onclick = async () => {
  const text = document.getElementById("comment-input").value.trim();
  if (!text) return alert("不能发送空评论");

  await fetch(`/api/post/${postId}/comment`, {
    method: "POST",
    headers: {"Content-Type": "application/json"},
    body: JSON.stringify({ content: text })
  });

  document.getElementById("comment-input").value = "";
  loadComments();
};

// 初始化
loadPost();
loadComments();
