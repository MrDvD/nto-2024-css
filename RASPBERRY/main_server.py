import asyncio, yaml, json, socket
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
        self.ax1.set_title("Signal")
        self.ax1.set_xlabel("Time [ms]")
        self.ax1.set_ylabel("Quants")
        self.ax2.set_title("Amplitude Spectrum")
        self.ax2.set_xlabel("Frequency [Hz]")
        self.ax2.set_ylabel("Amplitude, 10^3")
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

    def plot_graphs(self, mic, Y, freq, mode):
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
        elif mode == 'rfft':
            Y = 2 * np.fft.rfft(Y)
            t = np.linspace(0, self.secs, len(Y) - 1)
            if mic:
                self.ax4.clear()
                self.ax4.plot(t, abs(Y)[1:])
                self.ax4.grid()
            else:
                self.ax2.clear()
                self.ax2.plot(t, abs(Y)[1:])
                self.ax2.grid()
    
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
    
    async def get(self, reader, size=10000):
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
    
    def append_wav(self, mic, count, idx, freq, packet):
        curr = ''
        if idx > 1:
            with open(f'mic{mic}.data') as f:
                curr = f.read()
        with open(f'mic{mic}.data', 'w') as f:
            f.write(curr + packet)
        if idx == count:
            with open(f'mic{mic}.data') as f:
                data = f.read()
            data = list(map(lambda x: abs(4095 - int(x)), data.split()))
            self.plot.plot_graphs(mic, data, freq, 'signal')
            self.plot.plot_graphs(mic, data, freq, 'rfft')
            self.update_db(mic, data)
            self.plot.export_figs()
            if not self.poll.done():
                self.poll.set_result('POL')
            # plt.pause(0.00001)

    async def on_connect(self, reader, writer):
        message = await self.get(reader)
        print(message)
        if 'WAV' in message:
            # message = await self.get(reader)
            # header, other = message.split('\n')
            if message[-1] != 'S':
                print('MSG:', message[:50])
                header, packet = message[:50].split('S')
                mic, count, idx, freq = map(int, header[4:].split())
            else:
                message = message[:-1]
                mic, count, idx, freq = map(int, message[4:].split())
                packet = await self.get(reader)
            self.append_wav(mic, count, idx, freq, packet)
            # mic, count, buff_size, freq = map(int, header.split())
            # packet = other
            # for _ in range(count):
            #     packet += await self.get(reader)
            # packet = list(map(lambda x: abs(4095 - int(x)), packet.split()))
            while True:
                message = await self.get(reader)
                print(message)
                if message[-1] != 'S':
                    print('MSG:', message[:50] + 'end')
                    header, packet = message[:50].split('S')
                    mic, count, idx, freq = map(int, header[4:].split())
                else:
                    message = message[:-1]
                    mic, count, idx, freq = map(int, message[4:].split())
                    packet = await self.get(reader)
                self.append_wav(mic, count, idx, freq, packet)
                if idx == count:
                    break
        elif message == 'POL':
            await self.send(await self.poll, writer)
            self.poll = asyncio.Future()
        elif message == 'REC':
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
                sock.connect((cfg['mic0_ip'], cfg['back_port']))
                sock.sendall('REC'.encode())
            print('rec_sent')
        print('close')
        await self.close(writer)

server = Server(cfg['server_ip'], cfg['server_port'])

obj = Activator(server)
obj.elevate()