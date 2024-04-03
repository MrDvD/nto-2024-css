import asyncio, yaml, json
import scipy.io.wavfile as wf
import numpy as np
import matplotlib.pyplot as plt

with open('../config.yaml') as f:
    cfg = yaml.load(f, Loader=yaml.FullLoader)

def beautify(num):
    return f'{round(num, 3):.3f}'

def update_db(self):
    pass

class Plot:
    def __init__(self):
        self.fig, (self.ax1, self.ax2) = plt.subplots(2, 1)
        self.fig.set_size_inches(15, 5)
        for a in [self.ax1, self.ax2]:
            a.locator_params(axis='x', nbins=20)
            a.locator_params(axis='y', nbins=20)
        self.ax1.set_title("Signal")
        self.ax1.set_xlabel("Time [ms]")
        self.ax1.set_ylabel("Quants")
        self.ax2.set_title("Amplitude Spectrum")
        self.ax2.set_xlabel("Frequency [Hz]")
        self.ax2.set_ylabel("Amplitude, 10^3")
        self.texts = list()
    
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

    def draw_graphs(self, Y, freq, mode):
        if mode == 'signal':
            self.remove_text(self.texts)
            self.texts = self.draw_text(self.fig, Y)
            t = np.linspace(0, self.secs, self.secs * freq)
            self.ax1.clear()
            self.ax1.plot(t, Y)
            self.ax1.grid()
        elif mode == 'rfft':
            Y = 2 * np.fft.rfft(Y)
            t = np.linspace(0, self.secs, self.secs * freq // 2)
            self.ax2.clear()
            self.ax2.plot(t, Y)
            self.ax2.grid()
        plt.pause(0.00001)
    
    def export_fig(self):
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
        self.secs = 30
    
    async def start(self):
        server = await asyncio.start_server(self.on_connect, self.ip, self.port)
        self.poll = asyncio.Future()
        async with server:
            print(f'Listening on [{self.port}]')
            await server.serve_forever()
    
    async def get(self, reader, size=50000):
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
    
    def update_db(self, idx, Y):
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

    async def on_connect(self, reader, writer):
        message = await self.get(reader)
        print(message)
        if message == 'WAV':
            mic, count, buff_size, freq = map(int, (await self.get(reader)).split())
            packet = ''
            for _ in range(count):
                packet += await self.get(reader)
            packet = list(map(lambda x: abs(4095 - int(x)), packet.split()))
            # print(len(packet))
            self.plot.draw_graphs(packet, freq, 'signal')
            self.plot.draw_graphs(packet, freq, 'rfft')
            self.update_db(mic, packet)
            self.plot.export_fig()
            if not self.poll.done():
                self.poll.set_result('POL')
            # packet = np.array(packet, dtype=np.int16)
            # wf.write('curr_sample.wav', freq, packet)
        elif message == 'POL':
            await self.send(await self.poll, writer)
            self.poll = asyncio.Future()
        elif message == 'REC':
            await self.send('REC', writer)
        await self.close(writer)

server = Server(cfg['server_ip'], cfg['server_port'])

obj = Activator(server)
obj.elevate()