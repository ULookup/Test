//------------------------------------------------------
// IceACG 全站导航栏（完全漫画风 + 解耦事件系统 + 发现页入口）
//------------------------------------------------------

import { Api } from "./api.js";
import { State } from "./state.js";
import { UserCache } from "./userCache.js";


// =============== 工具：向页面广播统一事件 =================
function emit(eventName, detail = {}) {
    window.dispatchEvent(new CustomEvent(eventName, { detail }));
}


// =========================================================
// 未登录 UI
// =========================================================
function renderGuest(navRight) {
    navRight.innerHTML = `
        <button class="nav-login-btn" id="nav-login-btn">登录</button>
        <button class="nav-login-btn" id="nav-register-btn">注册</button>
    `;

    document.getElementById("nav-login-btn").onclick = () => {
        emit("nav:login");
        location.href = "/login.html";
    };

    document.getElementById("nav-register-btn").onclick = () => {
        emit("nav:register");
        location.href = "/register.html";
    };
}



// =========================================================
// 已登录 UI
// =========================================================
async function renderUser(navRight, uid) {

    const user = await UserCache.getUser(uid);

    if (!user || !user.id) {
        renderGuest(navRight);
        return;
    }

    const avatarCached = user.avatar_cached || await UserCache.getAvatar(user.avatar);

    navRight.innerHTML = `
        <div class="nav-user" id="nav-user">
            <img class="nav-avatar" id="nav-avatar" src="">
            <span class="nav-username">${user.username}</span>

            <div class="nav-menu" id="nav-menu">
                <div class="nav-menu-item" id="menu-home">个人主页</div>
                <div class="nav-menu-item" id="menu-account">账号中心</div>
                <div class="nav-menu-item" id="menu-logout">退出登录</div>
            </div>
        </div>
    `;

    document.getElementById("nav-avatar").src = avatarCached;

    const navUser = document.getElementById("nav-user");
    const navMenu = document.getElementById("nav-menu");

    let visible = false;

    navUser.onclick = (e) => {
        e.stopPropagation();
        visible = !visible;
        navMenu.classList.toggle("show", visible);
    };

    document.addEventListener("click", (e) => {
        if (!navUser.contains(e.target)) {
            visible = false;
            navMenu.classList.remove("show");
        }
    });

    document.getElementById("menu-home").onclick = () => {
        emit("nav:goHome", { uid });
        location.href = `/user.html?id=${uid}`;
    };

    document.getElementById("menu-account").onclick = () => {
        emit("nav:account");
        location.href = "/account.html";
    };

    document.getElementById("menu-logout").onclick = () => {
        emit("nav:logout");
        State.logout();
        UserCache.clear();
        location.reload();
    };
}




// =========================================================
// 主入口
// =========================================================
async function initNavbar() {

    const container = document.querySelector(".nav-container");
    const navLeft = document.querySelector(".nav-left");
    const navRight = document.getElementById("nav-right");


    // =====================================================
    // （1）加入发现按钮
    // =====================================================
    const discoverBtn = document.createElement("a");
    discoverBtn.className = "nav-discover";
    discoverBtn.href = "/discover.html";
    discoverBtn.innerText = "发现";

    discoverBtn.onclick = () => {
        emit("nav:discover");
    };

    if (container && navLeft) {
        container.insertBefore(discoverBtn, navLeft.nextSibling);
    }


    // =====================================================
    // （2）搜索栏：如果当前是搜索页 → 萌系模式
    // =====================================================
    const disableNavSearch = document.body.dataset.disableNavSearch === "true";

    const navSearch = document.querySelector(".nav-search");
    const navInput = document.getElementById("nav-search-input");
    const navBtn = document.getElementById("nav-search-btn");

    if (navSearch && disableNavSearch) {
        navSearch.classList.add("search-page-mode");
    }

    if (!disableNavSearch && navBtn && navInput) {
        navBtn.onclick = () => {
            const q = navInput.value.trim();
            if (q) {
                emit("nav:search", { q });
                location.href = `/search.html?q=${encodeURIComponent(q)}`;
            }
        };
    }


    // =====================================================
    // （3）Logo
    // =====================================================
    const logoElem = document.getElementById("nav-logo");
    if (logoElem) {
        logoElem.src = await UserCache.getLogo();
    }


    // =====================================================
    // （4）用户状态
    // =====================================================
    if (!State.isLogin()) {
        renderGuest(navRight);
        return;
    }

    const uid = State.getUserId();
    if (!uid) {
        renderGuest(navRight);
        return;
    }

    await renderUser(navRight, uid);
}


// 启动
initNavbar();
