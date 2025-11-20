export const Api = {
    
    async get(url) {
        const res = await fetch(url, {
            headers: {
                "Authorization": State.getToken() ? `Bearer ${State.getToken()}` : ""
            }
        });
        return await res.json();
    },

    async post(url, data = {}) {
        const res = await fetch(url, {
            method: "POST",
            headers: {
                "Content-Type": "application/json",
                "Authorization": State.getToken() ? `Bearer ${State.getToken()}` : ""
            },
            body: JSON.stringify(data)
        });
        return await res.json();
    },

    async upload(url, file) {
        const form = new FormData();
        form.append("file", file);

        const res = await fetch(url, {
            method: "POST",
            headers: {
                "Authorization": State.getToken() ? `Bearer ${State.getToken()}` : ""
            },
            body: form
        });

        return await res.json();
    }
};
