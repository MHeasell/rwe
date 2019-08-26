export function getIpv4Address(): Promise<string> {
    return fetch("https://ipv4.icanhazip.com")
    .then(response => {
        if (response.status !== 200) {
            throw new Error("Failed to look up IPv4 address");
        }
        return response.text();
    })
    .then(text => {
        return text.replace(/\n$/g, "");
    });
}
