import socket, yaml
import scipy.io.wavfile as wf
import numpy as np

with open('config.yaml') as f:
    cfg = yaml.load(f, Loader=yaml.FullLoader)

sample_rate = 8000

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
    sock.bind((cfg['server_ip'], cfg['server_port']))
    sock.listen()
    print(f'Listening on [{cfg['server_port']}]')
    conn, addr = sock.accept()
    message = conn.recv(16384)
    message.decode()
    print(message)
data = np.linspace(0., 1., sample_rate)
wf.write('curr_sample.wav', sample_rate, data.astype(np.int16))