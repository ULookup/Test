import { Api } from "./core/api.js";
import { State } from "./core/state.js";

const btn = document.getElementById("login-btn");
const errMsg = document.getElementById("error-msg");

btn.onclick = async () => {

    const account = document.getElementById("account").value.trim();
    const password = document.getElementById("password").value;

    if (!account || !password) {
        showError("请输入完整信息");
        return;
    }

    const res = await Api.post("/user/login", {
        account,
        password
    });

    // 登录失败
    if (res.code !== 0) {
        let msg = "未知错误";

        if (res.code === 2001) msg = "用户不存在";
        if (res.code === 2002) msg = "密码错误";
        if (res.msg) msg = res.msg;

        showError(msg);
        return;
    }

    // 登录成功
    const token = res.data.token;
    const uid = res.data.user.id;

    State.saveLogin(token, uid);

    // 返回首页
    location.href = "/index.html";
};

function showError(msg) {
    errMsg.innerText = msg;
    errMsg.style.display = "block";
}
