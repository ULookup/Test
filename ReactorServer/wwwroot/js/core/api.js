import { State } from "./state.js";

export const Api = {

    base: "",

    async get(url) {
        return await this.request("GET", url);
    },

    async post(url, data) {
        return await this.request("POST", url, data);
    },

    async postFile(url, formData) {
        const token = State.getToken();
        const fullUrl = this.base + url;

        let resp = null;
        try {
            resp = await fetch(fullUrl, {
                method: "POST",
                headers: {
                    "Authorization": "Bearer " + token
                },
                body: formData
            });
        } catch (e) {
            return { code: 5000, msg: "network_error" };
        }

        let json = null;
        try {
            json = await resp.json();
        } catch (_) {
            return { code: 5001, msg: "invalid_json" };
        }

        return json;
    },

    async request(method, url, data = null) {
        const token = State.getToken();
        const fullUrl = this.base + url;

        const opt = {
            method,
            headers: {
                "Content-Type": "application/json",
                "Authorization": "Bearer " + token
            }
        };

        if (method === "POST") {
            opt.body = JSON.stringify(data || {});
        }

        let resp = null;
        try {
            resp = await fetch(fullUrl, opt);
        } catch (e) {
            return { code: 5000, msg: "network_error" };
        }

        let json = null;
        try {
            json = await resp.json();
        } catch (_) {
            return { code: 5001, msg: "invalid_json" };
        }

        return json;
    }
};
