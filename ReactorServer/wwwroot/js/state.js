export const State = {
    getToken() {
        return localStorage.getItem("token") || "";
    },

    getUserId() {
        return localStorage.getItem("user_id") || "";
    },

    isLogin() {
        return !!this.getToken();
    },

    saveLogin(token, uid) {
        localStorage.setItem("token", token);
        localStorage.setItem("user_id", uid);
    },

    logout() {
        localStorage.removeItem("token");
        localStorage.removeItem("user_id");
    }
};
