document.getElementById("login-btn").onclick = async () => {

    const username = document.getElementById("username").value.trim();
    const password = document.getElementById("password").value.trim();

    if (!username || !password) {
        alert("请输入用户名和密码");
        return;
    }

    const res = await fetch("/api/auth/login", {
        method: "POST",
        headers: {"Content-Type": "application/json"},
        body: JSON.stringify({ username, password })
    });

    const json = await res.json();

    if (json.code !== 0) {
        alert(json.msg || "登录失败");
        return;
    }

    // 保存 token & user_id
    localStorage.setItem("token", json.data.token);
    localStorage.setItem("user_id", json.data.user.id);

    alert("登录成功！");
    location.href = "/index.html";
};
