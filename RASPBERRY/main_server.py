import asyncio, yaml
import scipy.io.wavfile as wf
import numpy as np
import matplotlib.pyplot as plt

plt.ion()

with open('config.yaml') as f:
    cfg = yaml.load(f, Loader=yaml.FullLoader)

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
        # packet = ''
        # while True:
        #     message = await self.get(reader)
        #     if message == '\n':
        #         await self.close()
        #         break
        #     else:
        #         packet += message
        # print(packet)
        message = await self.get(reader)
        print(message)
        cmd, idx, count, sample_rate = message.split()
        idx, count, sample_rate = int(idx), int(count), int(sample_rate)
        packet = ''
        for i in range(count):
            packet += await self.get(reader)
        packet = list(map(lambda x: abs(4095 - int(x)), packet.split()))
        print(len(packet))
        t = np.linspace(0, 10, sample_rate)
        plt.clf()
        plt.plot(t, packet)
        plt.show()
        plt.pause(0.001)
        packet = np.array(packet, dtype=np.int16)
        wf.write('curr_sample.wav', sample_rate, packet)
        await self.close(writer)

server = Server(cfg['server_ip'], cfg['server_port'])

obj = Activator(server)
obj.elevate()