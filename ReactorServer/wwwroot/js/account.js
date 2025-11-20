// UI tabs
const tabs = document.querySelectorAll(".side-item");
const panels = document.querySelectorAll(".tab-panel");

tabs.forEach(t => {
    t.onclick = () => {
        tabs.forEach(x => x.classList.remove("active"));
        t.classList.add("active");

        const tab = t.dataset.tab;
        panels.forEach(p => p.style.display = "none");
        document.getElementById(`tab-${tab}`).style.display = "block";
    };
});

// 加载用户信息
async function loadUser() {
    const uid = localStorage.getItem("user_id");
    if (!uid) return location.href = "/login.html";

    const res = await fetch(`/api/user/${uid}`);
    const json = await res.json();

    if (json.code !== 0) return;

    const u = json.data;

    document.getElementById("avatar").src = u.avatar;
    document.getElementById("nickname").value = u.nickname;
    document.getElementById("signature").value = u.signature || "";
}

loadUser();


// 修改头像
document.getElementById("change-avatar-btn").onclick = () => {
    document.getElementById("avatar-input").click();
};

document.getElementById("avatar-input").onchange = async () => {
    const file = document.getElementById("avatar-input").files[0];
    const form = new FormData();
    form.append("file", file);

    const upload = await fetch("/api/user/avatar", {
        method: "POST",
        body: form
    });

    const res = await upload.json();
    if (res.code !== 0) {
        alert("头像上传失败");
        return;
    }

    document.getElementById("avatar").src = res.data.url;
};


// 保存资料
document.getElementById("save-profile-btn").onclick = async () => {
    const nickname = document.getElementById("nickname").value.trim();
    const signature = document.getElementById("signature").value.trim();

    const res = await fetch("/api/user/update", {
        method: "POST",
        headers: {"Content-Type": "application/json"},
        body: JSON.stringify({ nickname, signature })
    });

    const json = await res.json();

    if (json.code === 0) {
        alert("资料已更新");
        loadUser();
    } else {
        alert(json.msg);
    }
};


// 修改密码
document.getElementById("change-pwd-btn").onclick = async () => {
    const old_pwd = document.getElementById("old-pwd").value.trim();
    const new_pwd = document.getElementById("new-pwd").value.trim();

    const res = await fetch("/api/auth/change_password", {
        method: "POST",
        headers: {"Content-Type": "application/json"},
        body: JSON.stringify({ old_pwd, new_pwd })
    });

    const json = await res.json();
    if (json.code === 0) alert("密码修改成功");
    else alert(json.msg);
};


// 退出登录
document.getElementById("logout-btn").onclick = async () => {
    await fetch("/api/auth/logout", { method: "POST" });

    localStorage.removeItem("token");
    localStorage.removeItem("user_id");

    location.href = "/login.html";
};
