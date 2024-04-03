import asyncio, yaml
import scipy.io.wavfile as wf
import numpy as np
import matplotlib.pyplot as plt

with open('config.yaml') as f:
    cfg = yaml.load(f, Loader=yaml.FullLoader)

def beautify(num):
    return f'{round(num, 3):.3f}'

class Plot:
    def __init__(self):
        self.fig, (self.ax1, self.ax2) = plt.subplots(2, 1)
        self.fig.set_size_inches(14, 8)
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

    def draw_text(self, fig, Y):
        arr = list()
        arr.append(fig.text(0.005, 0.15, f"MEAN: {beautify(np.mean(Y))}"))
        arr.append(fig.text(0.005, 0.12, f"MEDIAN: {beautify(np.median(Y))}"))
        arr.append(fig.text(0.005, 0.09, f"MAX: {beautify(np.max(Y))}"))
        arr.append(fig.text(0.005, 0.06, f"MIN: {beautify(np.min(Y))}"))
        arr.append(fig.text(0.005, 0.03, f"STD: {beautify(np.std(Y))}"))
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
        self.secs = 30
    
    async def start(self):
        server = await asyncio.start_server(self.on_connect, self.ip, self.port)
        self.future = asyncio.Future()
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

    async def on_connect(self, reader, writer):
        message = await self.get(reader)
        print(message)
        cmd, idx, count, buff_size, freq = message.split()
        idx, count, buff_size, freq = int(idx), int(count), int(buff_size), int(freq)
        packet = ''
        for _ in range(count):
            packet += await self.get(reader)
        packet = list(map(lambda x: abs(4095 - int(x)), packet.split()))
        # print(len(packet))
        self.plot.draw_graphs(packet, freq, 'signal')
        self.plot.draw_graphs(packet, freq, 'rfft')
        # packet = np.array(packet, dtype=np.int16)
        # wf.write('curr_sample.wav', freq, packet)
        await self.close(writer)

server = Server(cfg['server_ip'], cfg['server_port'])

obj = Activator(server)
obj.elevate()