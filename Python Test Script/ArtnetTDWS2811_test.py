import time
from pyartnet import ArtNetNode
import asyncio

a=[]
for i in range(256):
    a.append(i)
for i in range(256):
    a.append(i)

zeros=[]
for i in range(512):
    zeros.append(0)

async def main():

    node=ArtNetNode('192.168.1.92')
#    node=ArtNetNode('Artnet02A3')
    await node.start()
    universes=[]
    for i in range(64):
        print("Adding universe {}".format(i))
        universes.append(node.add_universe(i))
        universes[i].add_channel(start=1, width=512)

    i=0
    universes[0].data[2]=128
    universes[0].data[5]=128

    node.update()
    while(True):
#        print("Update")
#        await node.update()
        await asyncio.sleep(.1)

    # while True:

    #     for j in range(511): 
    #         universes[0].data[j]=0
    #     universes[3].data[i]=255
    #     i=i+1
    #     if (i>511):
    #         i=0
    #     node.update()
    #     await asyncio.sleep(.02)

asyncio.run(main())