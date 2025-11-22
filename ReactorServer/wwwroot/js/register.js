import { Api } from "./core/api.js";
import { State } from "./core/state.js";

const btn = document.getElementById("reg-btn");
const errMsg = document.getElementById("error-msg");

btn.onclick = async () => {

    const username = document.getElementById("username").value.trim();
    const email = document.getElementById("email").value.trim();
    const password = document.getElementById("password").value;

    // 基础校验
    if (!username || !email || !password) {
        return showError("请填写完整信息");
    }
    if (!email.includes("@")) {
        return showError("邮箱格式不正确");
    }
    if (password.length < 3) {
        return showError("密码太短");
    }

    // 调用注册接口（⚠ 无 /api 前缀）
    const res = await Api.post("/user/register", {
        username,
        email,
        password
    });

    // 注册失败
    if (res.code !== 0) {
        let msg = "注册失败";

        if (res.code === 2003) msg = "用户名已存在";
        if (res.code === 2004) msg = "邮箱已被使用";
        if (res.msg) msg = res.msg;

        return showError(msg);
    }

    // 注册成功 → 自动跳转到登录页
    alert("注册成功，请登录！");
    location.href = "/login.html";
};

function showError(msg) {
    errMsg.innerText = msg;
    errMsg.style.display = "block";
}
