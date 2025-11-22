export const State = {

    saveLogin(token, userId) {
        localStorage.setItem("token", token);
        localStorage.setItem("userId", userId);
    },

    isLogin() {
        return !!localStorage.getItem("token");
    },

    getToken() {
        return localStorage.getItem("token") || "";
    },

    getUserId() {
        return parseInt(localStorage.getItem("userId") || "0");
    },

    logout() {
        localStorage.removeItem("token");
        localStorage.removeItem("userId");
    }
};
