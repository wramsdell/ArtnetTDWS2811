import time
from pyartnet import ArtNetNode
import asyncio
import socket

a=[]
for i in range(256):
    a.append(i)
for i in range(256):
    a.append(i)

zeros=[]
for i in range(512):
    zeros.append(0)

async def main():
    result=socket.getaddrinfo("Artnet5DD9",0,0,0,0)
    ip=result[0][-1][0]
    print("Connecting to IP address {}".format(ip))
    node=ArtNetNode(ip)

    await node.start()
    universe=node.add_universe(0)
    universe.add_channel(start=1, width=512)

    tree=[]
    tree.append(list(range(24)))
    tree.append(list(reversed(range(24,48))))
    tree.append(list(range(50,74)))
    tree.append(list(reversed(range(74,98))))
    tree.append(list(range(100,124)))
    tree.append(list(reversed(range(124,148))))

    treeX=len(tree)
    treeY=len(tree[0])

    for x in range(treeX):
        for y in range(treeY):
            universe.data[3*(tree[x][y])]=0
            universe.data[3*(tree[x][y])+1]=255
            universe.data[3*(tree[x][y])+2]=0
    node.update()

    # i=0    
    # while True:

    #     for j in range(150):
    #         universe.data[3*j:(3*j+2)]={0,255,0}
    #     universe.data[3*i:(3*i+2)]={255,255,255}
    #     i=i+1
    #     if (i>149):
    #         i=0
    #     node.update()
    #     await asyncio.sleep(.02)

asyncio.run(main())

