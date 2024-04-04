import flask, socket, yaml, json

with open('../config.yaml') as f:
    cfg = yaml.load(f, Loader=yaml.FullLoader)

app = flask.Flask('Web', template_folder='')

@app.route('/')
def main():
    return flask.send_file('website/main.html')

@app.route('/result')
def result():
    with open('../database.json') as f:
        db = json.load(f)
    return flask.render_template('website/result.html', context=db)

@app.route('/<js>.js')
def scripts(js):
    conx = {'ip': cfg['server_ip'], 'web_port': cfg['web_port']}
    return flask.render_template(f'website/{js}.js', context=conx)

@app.route('/<css>.css')
def styles(css):
    return flask.send_file(f'website/{css}.css')

@app.route('/media/<img>')
def image(img):
    return flask.send_file(f'website/media/{img}')

@app.route('/fonts/<font>')
def fonts(font):
    return flask.send_file(f'website/fonts/{font}')

@app.route('/record')
def record():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        sock.connect((cfg['server_ip'], cfg['server_port']))
        sock.sendall('REC'.encode())

@app.route('/poll')
def poll():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        sock.connect((cfg['server_ip'], cfg['server_port']))
        sock.sendall('POL'.encode())
        if sock.recv(100).decode() == 'POL':
            return 'POL'

app.run(host=cfg['server_ip'], port=cfg['web_port'])