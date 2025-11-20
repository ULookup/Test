// 登录状态检查
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
              <img src="${j.data.avatar}" style="width:36px;height:36px;border-radius:50%;cursor:pointer;"
                   onclick="location.href='/user.html?id=${j.data.id}'">
          `;
      });
}

// 标签逻辑
const tagInput = document.getElementById("tag-input");
const tagList = document.getElementById("tag-list");
let tags = [];

tagInput.addEventListener("keydown", e => {
    if (e.key === "Enter") {
        e.preventDefault();
        let t = tagInput.value.trim();
        if (t && !tags.includes(t)) {
            tags.push(t);
            renderTags();
        }
        tagInput.value = "";
    }
});

function renderTags() {
    tagList.innerHTML = tags.map(t => `
      <div class="tag">${t} <span class="tag-close" onclick="removeTag('${t}')">×</span></div>
    `).join("");
}

window.removeTag = (t) => {
    tags = tags.filter(x => x !== t);
    renderTags();
};

// 图片上传
const addImgBtn = document.getElementById("add-img-btn");
const imgInput = document.getElementById("img-input");
const imgBox = document.getElementById("img-box");

let images = [];

addImgBtn.onclick = () => imgInput.click();

imgInput.onchange = async () => {
    for (let file of imgInput.files) {
        const form = new FormData();
        form.append("file", file);

        const res = await fetch("/api/upload/image", {
            method: "POST",
            body: form
        });

        const json = await res.json();
        if (json.code === 0) {
            images.push(json.data.url);
        }
    }
    renderImages();
};

function renderImages() {
    imgBox.innerHTML = images.map(url => `
      <div class="img-wrap">
        <img src="${url}" class="img-preview">
        <div class="img-delete" onclick="deleteImage('${url}')">×</div>
      </div>
    `).join("");
}

window.deleteImage = (url) => {
    images = images.filter(x => x !== url);
    renderImages();
};

// 发布帖子
document.getElementById("submit-btn").onclick = async () => {

    const title = document.getElementById("post-title").value.trim();
    const content = document.getElementById("post-content").value.trim();

    if (!title || !content) {
        alert("标题和正文不能为空");
        return;
    }

    const body = {
        title,
        content,
        tags,
        images
    };

    const res = await fetch("/api/post/create", {
        method: "POST",
        headers: {"Content-Type": "application/json"},
        body: JSON.stringify(body)
    });

    const json = await res.json();

    if (json.code !== 0) {
        alert(json.msg || "发布失败");
        return;
    }

    // 跳转到帖子详情页
    alert("发布成功！");
    location.href = `/post.html?id=${json.data.id}`;
};
