/* =====================================================
 * IceACG 通用 UI 模块 — 点赞 / 收藏 / 评论 统一逻辑
 * 适用于所有漫画风页面
 * ===================================================== */
export const UI = {

    /* ===========================
       点赞按钮更新
       =========================== */
    updateLike(btn, count, liked) {
        const icon = btn.querySelector(".like-icon");
        const num  = btn.querySelector(".like-number");

        num.textContent = count;

        if (liked) {
            icon.classList.add("liked");
            btn.classList.add("active");
        } else {
            icon.classList.remove("liked");
            btn.classList.remove("active");
        }
    },

    /* ===========================
       收藏按钮更新
       =========================== */
    updateFav(btn, count, faved) {
        const icon = btn.querySelector(".fav-icon");
        const num  = btn.querySelector(".fav-number");

        num.textContent = count;

        if (faved) {
            icon.classList.add("faved");
            btn.classList.add("active");
        } else {
            icon.classList.remove("faved");
            btn.classList.remove("active");
        }
    },

    /* ===========================
       评论数量更新
       =========================== */
    updateCommentCount(elem, count) {
        elem.textContent = count;
    },

    /* ===========================
       简单的漫画风按钮点击动效
       =========================== */
    pop(btn) {
        btn.classList.add("pop");
        setTimeout(() => btn.classList.remove("pop"), 180);
    }
};
