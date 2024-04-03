let server_ip = '{{ context.ip }}', web_port = {{ context.web_port }};

async function poll_result() {
    let url = `http://${server_ip}:${web_port}/poll`;
    let response = await fetch(url, {
        method: 'GET'
    });
    if (response.status == 200) {
        window.location.reload();
    }
    await poll_result();
}

window.onload = function() {
    document.getElementById('back_button').addEventListener('click', function() {
        window.location.href = `http://${server_ip}:${web_port}/`;
    });
    // poll_result();
}