// wwwroot/js/core/userCache.js
import { Api } from "./api.js";
import { State } from "./state.js";

export const UserCache = {

    // 用户信息内存缓存
    map: {},

    // 头像 base64 缓存 localStorage key 前缀
    avatarPrefix: "avatar-cache-",
    logoKey: "logo-cache",

    clear() {
        this.map = {};
        // 可以选择清理 localStorage
        // localStorage.clear();
    },

    invalidate(id) {
        delete this.map[id];
    },

    /*********************************************************
     * convertImageToBase64(url)
     * 下载图片 → 转 base64 → 返回
     *********************************************************/
    async convertImageToBase64(url) {
        const blob = await fetch(url).then(r => r.blob());

        return await new Promise(resolve => {
            const reader = new FileReader();
            reader.onload = () => resolve(reader.result);
            reader.readAsDataURL(blob);
        });
    },

    /*********************************************************
     * getAvatar(url)
     * 头像图片本地缓存：第一次下载 → base64 → localStorage
     *********************************************************/
    /*********************************************************
 * getAvatar(url)
 * 安全头像缓存：不会爆 localStorage
 *********************************************************/
    async getAvatar(url) {
        if (!url) return "";

        if (!url.startsWith("/")) url = "/" + url;

        const key = this.avatarPrefix + url;

        // 本地已有缓存 → 直接使用
        const cached = localStorage.getItem(key);
        if (cached) return cached;

        // 下载并转 base64
        let base64;
        try {
            base64 = await this.convertImageToBase64(url);
        } catch (err) {
            console.warn("头像下载失败:", err);
            return url;
        }

        // 限制头像缓存数量（最多 30 个）
        const avatarKeys = Object.keys(localStorage)
            .filter(k => k.startsWith(this.avatarPrefix));
        if (avatarKeys.length >= 30) {
            // 删除最老的缓存
            const toDelete = avatarKeys[0];
            localStorage.removeItem(toDelete);
        }

        // 保存缓存（安全写入）
        try {
            localStorage.setItem(key, base64);
        } catch (err) {
            console.warn("头像缓存失败（空间不足）:", err);

            // 清理所有旧头像缓存
            for (const k of Object.keys(localStorage)) {
                if (k.startsWith(this.avatarPrefix)) {
                    localStorage.removeItem(k);
                }
            }

            // fallback 使用原图
            return url;
        }

        return base64;
    },


    /*********************************************************
     * getLogo()
     * logo 图片缓存（与头像使用相同机制）
     *********************************************************/
    async getLogo() {
        const url = "/static/logo.png";
        const cached = localStorage.getItem(this.logoKey);

        if (cached) return cached;

        const base64 = await this.convertImageToBase64(url);
        localStorage.setItem(this.logoKey, base64);
        return base64;
    },

    /*********************************************************
     * 获取用户信息
     *********************************************************/
    async getUser(id) {
        id = String(id);
        if (!id) return null;

        if (this.map[id]) {
            return this.map[id];
        }

        const res = await Api.get(`/user/${id}`);

        if (res.code === 0 && res.data) {
            const user = res.data;

            // 修复头像路径
            if (user.avatar && !user.avatar.startsWith("/")) {
                user.avatar = "/" + user.avatar;
            }

            // 🔥 头像改为本地缓存后的 base64
            user.avatar_cached = await this.getAvatar(user.avatar);

            this.map[id] = user;
            return user;
        }

        return null;
    }
};

/*********************************************************
 * AssetCache — 本地静态资源缓存 (空状态图 / 图标 / banner /素材)
 *********************************************************/

export const AssetCache = {

    prefix: "asset-cache-",   // localStorage key 前缀

    // 将图片下载为 base64
    async fetchBase64(url) {
        const blob = await fetch(url).then(r => r.blob());

        return await new Promise(resolve => {
            const reader = new FileReader();
            reader.onload = () => resolve(reader.result);
            reader.readAsDataURL(blob);
        });
    },

    // 获取缓存图片
    async get(url) {
        if (!url) return "";

        // 自动补全路径（你的接口图路径都不带 /）
        if (!url.startsWith("/")) url = "/" + url;

        const key = this.prefix + url;

        // 本地已有 → 直接返回
        const cached = localStorage.getItem(key);
        if (cached) return cached;

        // 下载并缓存
        try {
            const base64 = await this.fetchBase64(url);
            localStorage.setItem(key, base64);
            return base64;
        } catch (err) {
            console.warn("AssetCache 加载失败:", err);
            return url; // fallback
        }
    }
};
