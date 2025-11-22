import { Api } from "./core/api.js";
import { State } from "./core/state.js";
import { UserCache } from "./core/userCache.js";

async function init() {

    const uid = State.getUserId();
    if (!uid) {
        alert("请先登录");
        location.href = "/login.html";
        return;
    }

    // 加载用户数据
    const user = await UserCache.getUser(uid);
    if (!user) {
        alert("加载用户数据失败");
        return;
    }

    // 渲染基本资料
    document.getElementById("avatar-preview").src = user.avatar;
    document.getElementById("input-username").value = user.username;
    document.getElementById("input-bio").value = user.bio || "";

    // 自动上传头像
    document.getElementById("avatar-file").onchange = async (ev) => {
        const file = ev.target.files[0];
        if (!file) return;

        const formData = new FormData();
        formData.append("avatar", file);

        const res = await Api.postFile("/user/avatar/upload", formData);

        if (res.code === 0) {
            UserCache.invalidate(uid);
            document.getElementById("avatar-preview").src = file ? URL.createObjectURL(file) : user.avatar;
            alert("头像更新成功！");
        } else {
            alert("上传失败：" + res.msg);
        }
    };

    // 保存基本资料
    document.getElementById("save-btn").onclick = async () => {
        const username = document.getElementById("input-username").value.trim();
        const bio = document.getElementById("input-bio").value.trim();

        const res = await Api.post("/user/profile/update", { username, bio });

        if (res.code === 0) {
            UserCache.invalidate(uid);
            alert("保存成功！");
        } else {
            alert(res.msg || "保存失败");
        }
    };
}

init();
