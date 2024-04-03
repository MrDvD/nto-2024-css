let server_ip = '{{ context.ip }}', web_port = {{ context.web_port }};

async function poll_result() {
    let url = `http://${server_ip}:${web_port}/poll`;
    let response = await fetch(url, {
        method: 'GET'
    });
    if (response.status == 200) {
        window.location.href = `http://${server_ip}:${web_port}/result`;
    } else {
        await poll_result();
    }
}

window.onload = function() {
    let button = document.getElementsByClassName('button')[0];
    button.addEventListener('click', function() {
        button.innerHTML = "Listening...";
        poll_result();
    });
}