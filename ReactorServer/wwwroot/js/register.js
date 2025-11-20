document.getElementById("reg-btn").onclick = async () => {

    const username = document.getElementById("username").value.trim();
    const email = document.getElementById("email").value.trim();
    const password = document.getElementById("password").value.trim();

    if (!username || !email || !password) {
        alert("请填写所有必填项");
        return;
    }

    const res = await fetch("/api/auth/register", {
        method: "POST",
        headers: {"Content-Type": "application/json"},
        body: JSON.stringify({ username, email, password })
    });

    const json = await res.json();

    if (json.code !== 0) {
        alert(json.msg || "注册失败");
        return;
    }

    alert("注册成功！请登录");
    location.href = "/login.html";
};
