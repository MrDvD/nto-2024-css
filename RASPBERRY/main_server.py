import asyncio, yaml, json, socket, re
import numpy as np
import matplotlib.pyplot as plt

with open('../config.yaml') as f:
    cfg = yaml.load(f, Loader=yaml.FullLoader)

def beautify(num):
    return f'{round(num, 3):.3f}'

class Plot:
    def __init__(self):
        self.fig, ((self.ax1, self.ax3), (self.ax2, self.ax4)) = plt.subplots(2, 2)
        self.fig.set_size_inches(12, 5)
        for a in [self.ax1, self.ax2, self.ax3, self.ax4]:
            a.locator_params(axis='x', nbins=20)
            a.locator_params(axis='y', nbins=20)
        # self.ax1.set_title("Signal")
        # self.ax1.set_xlabel("Time [ms]")
        # self.ax1.set_ylabel("Quants")
        # self.ax2.set_title("Amplitude Spectrum")
        # self.ax2.set_xlabel("Frequency [Hz]")
        # self.ax2.set_ylabel("Amplitude, 10^3")
        self.texts = list()
        self.secs = 30
    
    def calc_vals(self, Y):
        return list(map(beautify, [np.mean(Y), np.median(Y), np.max(Y), np.min(Y), np.std(Y)]))

    def draw_text(self, fig, Y):
        arr = list()
        mean, median, mx, mn, std = self.calc_vals(Y)
        arr.append(fig.text(0.005, 0.15, f"MEAN: {mean}"))
        arr.append(fig.text(0.005, 0.12, f"MEDIAN: {median}"))
        arr.append(fig.text(0.005, 0.09, f"MAX: {mx}"))
        arr.append(fig.text(0.005, 0.06, f"MIN: {mn}"))
        arr.append(fig.text(0.005, 0.03, f"STD: {std}"))
        return arr

    def remove_text(self, arr):
        for t in arr:
            t.set_visible(False)

    def plot_graphs(self, mic, freq, Y, mode):
        if mode == 'signal':
            # self.remove_text(self.texts)
            # self.texts = self.draw_text(self.fig, Y)
            t = np.linspace(0, self.secs, len(Y))
            if mic:
                self.ax3.clear()
                self.ax3.plot(t, Y)
                self.ax3.grid()
            else:
                self.ax1.clear()
                self.ax1.plot(t, Y)
                self.ax1.grid()
            self.fig.savefig('../FLASK/website/media/mic1.jpg', dpi=150)
        elif mode == 'rfft':
            Y = 2 * np.fft.rfft(Y)
            t = np.arange(1, len(Y)) * freq / len(Y)
            if mic:
                self.ax4.clear()
                self.ax4.plot(t, abs(Y)[1:] / 1000)
                self.ax4.grid()
            else:
                self.ax2.clear()
                self.ax2.plot(t, abs(Y)[1:] / 1000)
                self.ax2.grid()
            self.fig.savefig('../FLASK/website/media/mic2.jpg', dpi=150)
    
    def export_figs(self):
        self.fig.savefig('../FLASK/website/media/wav.jpg', dpi=150)

class Activator:
    def __init__(self, *servers):
        self.servers = list(servers)
    
    async def gather(self):
        start = list(map(lambda x: x.start(), self.servers))
        return await asyncio.gather(*start)

    def elevate(self):
        asyncio.run(self.gather())

class Server:
    def __init__(self, ip, port):
        self.ip, self.port = ip, port
        self.plot = Plot()
        self.init_db()
    
    async def start(self):
        server = await asyncio.start_server(self.on_connect, self.ip, self.port)
        self.poll = asyncio.Future()
        async with server:
            print(f'Listening on [{self.port}]')
            await server.serve_forever()
    
    async def get(self, reader, size=8000):
        data = await reader.read(size)
        return data.decode()
    
    async def send(self, message, writer):
        writer.write(message.encode())
        await writer.drain()
    
    async def close(self, obj):
        obj.close()
        await obj.wait_closed()
    
    def init_db(self):
        db = {0: dict(), 1: dict()}
        for i in [0, 1]:
            db[i]['mean'] = '???'
            db[i]['median'] = '???'
            db[i]['mx'] = '???'
            db[i]['mn'] = '???'
            db[i]['std'] = '???'
        with open('../database.json', 'w') as f:
            json.dump(db, f)
        with open('mic0.data', 'w') as f:
            f.write('')
        with open('mic1.data', 'w') as f:
            f.write('')
    
    def update_db(self, idx, Y):
        idx = str(idx)
        mean, median, mx, mn, std = self.plot.calc_vals(Y)
        with open('../database.json') as f:
            db = json.load(f)
        db[idx]['mean'] = mean
        db[idx]['median'] = median
        db[idx]['mx'] = mx
        db[idx]['mn'] = mn
        db[idx]['std'] = std
        with open('../database.json', 'w') as f:
            json.dump(db, f)
    
    def result(self, freq):
        for mic in [0, 1]:
            with open(f'mic{mic}.data') as f:
                data = f.read()
            data = list(map(lambda x: abs(4095 - int(x)), data.split()))
            print(mic, len(data))
            if data:
                self.plot.plot_graphs(mic, freq, data, 'signal')
                self.plot.plot_graphs(mic, freq, data, 'rfft')
                self.update_db(mic, data)
        self.plot.export_figs()
        if not self.poll.done():
            self.poll.set_result('POL')
    
    def append_wav(self, mic, packet):
        packet = re.sub(r'WAV.*S', '', packet)
        with open(f'mic{mic}.data', 'a') as f:
            f.write(packet)
    
    async def on_connect(self, reader, writer):
        message = await self.get(reader)
        if 'WAV' in message:
            try:
                print('CMD:', message)
                cmd, mic = message.split()
                packet = ''
                while True:
                    msg = await self.get(reader)
                    print(f'MIC {mic}:', len(msg))
                    packet += msg
                    if packet[-1] == 'S':
                        packet = packet[:-1]
                        break
                with open(f'mic{mic}.data', 'a') as f:
                    f.write(packet)
            except Exception as e:
                print(e)
            self.result(1000)
        elif message == 'POL':
            await self.send(await self.poll, writer)
            self.poll = asyncio.Future()
        elif message == 'REC':
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
                sock.connect((cfg['mic0_ip'], cfg['back_port']))
                sock.sendall('REC'.encode())
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
                sock.connect((cfg['mic1_ip'], cfg['back_port']))
                sock.sendall('REC'.encode())
            print('rec_sent')
        print('close')
        await self.close(writer)

    # async def on_connect(self, reader, writer):
    #     message = await self.get(reader)
    #     print(message, '|')
    #     freq = 1000 # for safety
    #     # res = re.search(r'WAV.*S', message)
    #     if 'WAV' in message:
    #         if message[0:3] != 'WAV':
    #             packet = message
    #         else:
    #             if message[-1] == 'S':
    #                 mic, count, idx, freq = map(int, message[4:-1].split())
    #                 packet = await self.get(reader)
    #             else:
    #                 header, packet = message.split('S')
    #                 mic, count, idx, freq = map(int, header[4:].split())
    #         self.append_wav(mic, packet)
    #         while True:
    #             try:
    #                 message = await self.get(reader)
    #                 print('MSG:', message[:50])
    #                 if message[0:3] != 'WAV':
    #                     packet = message
    #                 else:
    #                     if message[-1] == 'S':
    #                         mic, count, idx, freq = map(int, message[4:-1].split())
    #                         packet = await self.get(reader)
    #                     else:
    #                         header, packet = message.split('S')
                            
    #                         mic, count, idx, freq = map(int, header[4:].split())
    #                 self.append_wav(mic, packet)
    #                 print(idx, count)
    #                 if idx == count:
    #                     print('RESULT')
    #                     self.result(freq)
    #                     break
    #             except:
    #                 print('EXCEPT_RESULT')
    #                 self.result(freq)
    #                 break
    #     elif message == 'POL':
    #         await self.send(await self.poll, writer)
    #         self.poll = asyncio.Future()
    #     elif message == 'REC':
    #         # with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
    #         #     sock.connect((cfg['mic0_ip'], cfg['back_port']))
    #         #     sock.sendall('REC'.encode())
    #         with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
    #             sock.connect((cfg['mic1_ip'], cfg['back_port']))
    #             sock.sendall('REC'.encode())
    #         print('rec_sent')
    #     print('close')
    #     await self.close(writer)

server = Server(cfg['server_ip'], cfg['server_port'])

# with open('mic0.data') as f:
#     data = f.read()
#     data = list(map(lambda x: 4095 - int(x), data.split()))
#     server.plot.plot_graphs(0, 1000, data, 'signal')
#     server.plot.plot_graphs(0, 1000, data, 'rfft')
#     server.plot.export_figs()

obj = Activator(server)
obj.elevate()