import { Api } from "./core/api.js";
import { State } from "./core/state.js";

// =============== 登录强制判断 START ===============
if (!State.isLogin()) {
    location.href = "/login.html";
}
// =============== 登录强制判断 END ===============


// 登录 UI
async function initLoginUI() {
    const nav = document.getElementById("nav-right");

    if (!State.isLogin()) {
        nav.innerHTML = `<button onclick="location.href='/login.html'">登录</button>`;
        return;
    }

    const uid = State.getUserId();
    const res = await Api.get(`/user/${uid}`);

    nav.innerHTML = `
        <img src="${res.data.avatar}" class="nav-avatar" onclick="location.href='/account.html'">
    `;
}

// 图片相关
const imgListBox = document.getElementById("img-list");
const imgFileInput = document.getElementById("img-file");
const addImgBtn = document.getElementById("add-img-btn");

let images = [];

// 点击按钮 → 打开文件选择
addImgBtn.onclick = () => imgFileInput.click();


// 选择图片 → 上传
imgFileInput.onchange = async () => {
    const file = imgFileInput.files[0];
    if (!file) return;

    if (images.length >= 6) {
        alert("最多上传 6 张图片");
        return;
    }

    const form = new FormData();
    form.append("file", file);

    let res;
    try {
        res = await fetch("/upload/image", {
            method: "POST",
            headers: {
                // 和 Api.js 保持完全一致！
                "Authorization": State.getToken()
            },
            body: form
        });
    } catch (e) {
        alert("上传失败（网络错误）");
        console.error(e);
        return;
    }

    let json;
    try {
        json = await res.json();
    } catch (e) {
        alert("上传失败（服务器未返回 JSON）");
        console.error(e);
        return;
    }

    if (json.code !== 0) {
        alert(json.msg || "上传失败");
        return;
    }

    const url = json.data.url;
    images.push(url);

    renderImages();
};


// 渲染预览图
function renderImages() {
    imgListBox.innerHTML = images.map(url => `
        <img src="${url}" onclick="this.remove();">
    `).join("");
}


// 发布帖子
document.getElementById("publish-btn").onclick = async () => {
    if (!State.isLogin()) {
        alert("请先登录");
        return;
    }

    const title = document.getElementById("title").value.trim();
    const content = document.getElementById("content").value.trim();
    const tags = document.getElementById("tags").value.trim().split(",")
                   .map(t => t.trim()).filter(t => t.length > 0);

    if (!title || !content) {
        alert("标题和内容不能为空");
        return;
    }

    const res = await Api.post("/post/create", {
        title,
        content,
        images,
        tags
    });

    if (res.code === 0) {
        alert("发布成功！");
        location.href = `/post.html?id=${res.data.id}`;
    } else {
        alert(res.msg);
    }
};

// 初始化页面
initLoginUI();
