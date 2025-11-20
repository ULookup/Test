import { Api } from "./api.js";
import { State } from "./state.js";

const loginArea = document.getElementById("login-area");
const topicListEl = document.getElementById("topic-list");
const recommendListEl = document.getElementById("recommend-list");
const creatorListEl = document.getElementById("creator-list");

async function initLogin() {
  if (!State.isLogin()) {
    loginArea.innerHTML = `
      <button class="login-btn" onclick="location.href='/login.html'">登录</button>
    `;
    return;
  }

  const user = await Api.getUserInfo();
  loginArea.innerHTML = `
    <span style="margin-right:10px;">${user.nickname}</span>
    <img src="${user.avatar || '/static/logo.png'}" style="width:32px;height:32px;border-radius:50%;">
  `;
}

/* ========== 加载热门话题 ========== */
async function loadTopics() {
  const res = await fetch("/api/topics/hot");
  const json = await res.json();

  (json.data || []).forEach(t => {
    const div = document.createElement("div");
    div.className = "topic";
    div.textContent = `# ${t.name}`;
    div.onclick = () => location.href = `/index.html?tag=${encodeURIComponent(t.name)}`;
    topicListEl.appendChild(div);
  });
}

/* ========== 加载推荐帖子 ========== */
async function loadRecommend() {
  const res = await fetch("/api/recommend/posts");
  const json = await res.json();

  (json.data || []).forEach(p => {
    const div = document.createElement("div");
    div.className = "recommend-card";
    div.onclick = () => location.href = `/post.html?id=${p.id}`;

    div.innerHTML = `
      <div class="rec-title">${p.title}</div>
      <div class="rec-info">${p.author} · ${p.time}</div>
      <div class="rec-tags">
        ${(p.tags || []).map(t => `<span class="rec-tag">${t}</span>`).join("")}
      </div>
    `;

    recommendListEl.appendChild(div);
  });
}

/* ========== 推荐创作者 ========== */
async function loadCreators() {
  const res = await fetch("/api/recommend/creators");
  const json = await res.json();

  (json.data || []).forEach(c => {
    const div = document.createElement("div");
    div.className = "creator-card";

    div.innerHTML = `
      <img class="creator-avatar" src="${c.avatar || '/static/logo.png'}">
      <div class="creator-info">
        <div class="creator-name">${c.nickname}</div>
        <div class="creator-meta">粉丝 ${c.fans}</div>
      </div>
      <div class="creator-easter">⭐ 程序员推荐</div>
    `;

    creatorListEl.appendChild(div);
  });
}

/* ========== 启动 ========== */
(async function init() {
  initLogin();
  loadTopics();
  loadRecommend();
  loadCreators();
})();
