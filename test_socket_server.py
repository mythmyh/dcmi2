import sys
import time
from ctypes import *
import hashlib
import struct
import copy
# mss大小
import time
per_mss = 1400
pure_data = 1400
# 分辨率 总buf大小
total_size = 640 * 480*2

circle_times = int(total_size/ per_mss)
last_circle_bytes = total_size % per_mss
buf = [0]*total_size


class tagBITMAPINFOHEADER(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ('biSize', c_uint32),
        ('biWidth', c_long),
        ('biHeight', c_long),
        ('biPlanes', c_uint16),
        ('biBitCount', c_uint16),
        ('biCompression', c_uint32),
        ('biSizeImage',c_uint32),
        ('biXPelsPerMeter',c_long),
        ('biYPelsPerMeter', c_long),
        ('biClrUsed',c_uint32),
        ('biClrImportant',c_uint32)
    ]

    def encode(self):
        return string_at(addressof(self),sizeof(self))

    def decode(self, data):
        memmove(addressof(self), data, sizeof(self))
        return len(data)


class tagRGBQUAD(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ('rgbBlue', c_byte),
        ('rgbGreen', c_byte),
        ('rgbRed', c_byte),
        ('rgbReserved', c_byte)

    ]

    def encode(self):
        return string_at(addressof(self),sizeof(self))

    def decode(self, data):
        memmove(addressof(self), data, sizeof(self))
        return len(data)


class tagBITMAPFILEHEADER(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ('bfType', c_uint16),
        ('bfSize', c_uint32),
        ('bfReserved1', c_uint16),
        ('bfReserved2', c_uint16),
        ('bfOffBits', c_uint32)
    ]

    def encode(self):
        return string_at(addressof(self),sizeof(self))

    def decode(self, data):
        memmove(addressof(self), data, sizeof(self))
        return len(data)


def Rgb565ConvertBmp(buf2, width, height, filename):
    bmiHdr = tagBITMAPINFOHEADER()
    bmfHdr = tagBITMAPFILEHEADER()
    bmiClr = [tagRGBQUAD(), tagRGBQUAD(), tagRGBQUAD()]
    bmiHdr.biSize = sizeof(tagBITMAPINFOHEADER)
    bmiHdr.biWidth = width
    bmiHdr.biHeight = height
    bmiHdr.biPlanes = 1
    bmiHdr.biBitCount = 16
    bmiHdr.biCompression = 3
    bmiHdr.biSizeImage = (width * height * 2)
    bmiHdr.biXPelsPerMeter = 0
    bmiHdr.biYPelsPerMeter = 0
    bmiHdr.biClrUsed = 0
    bmiHdr.biClrImportant = 0

    bmiClr[0].rgbBlue = 0
    bmiClr[0].rgbGreen = 0xF8
    bmiClr[0].rgbRed = 0
    bmiClr[0].rgbReserved = 0

    bmiClr[1].rgbBlue = 0xE0
    bmiClr[1].rgbGreen = 0x07
    bmiClr[1].rgbRed = 0
    bmiClr[1].rgbReserved = 0

    bmiClr[2].rgbBlue = 0x1F
    bmiClr[2].rgbGreen = 0
    bmiClr[2].rgbRed = 0
    bmiClr[2].rgbReserved = 0
    bmfHdr.bfType = 0x4D42
    bmfHdr.bfSize = sizeof(tagBITMAPFILEHEADER) + sizeof(tagBITMAPINFOHEADER) + sizeof(tagRGBQUAD) * 3 + bmiHdr.biSizeImage
    bmfHdr.bfReserved1 = 0
    bmfHdr.bfReserved2 = 0
    bmfHdr.bfOffBits = sizeof(tagBITMAPFILEHEADER) + sizeof(tagBITMAPINFOHEADER) + sizeof(tagRGBQUAD) * 3
    with open(filename, "wb") as f:
        f.write(bmfHdr.encode())
        f.write(bmiHdr.encode())
        f.write(bmiClr[0].encode())
        f.write(bmiClr[1].encode())
        f.write(bmiClr[2].encode())
        for i in range(height):
            f.write(bytes(buf2[(height-i-1)*width*2:(height-i-1)*width*2+2*width]))
        f.close()


counter_set = set()


def receive_period(_clientsocket, _counter_set, _start, _end):
    global per_mss,buf
    _counter = _start
    _clientsocket.send(str(_start+circle_times+1).encode('utf-8'))
    while _counter < _end:
        if _counter == circle_times - 1:
            per_mss = last_circle_bytes
        msg2 = _clientsocket.recv(per_mss)

        buf[_counter * pure_data:_counter* pure_data + per_mss] = msg2
        _counter += 1


if __name__ == '__main__':
    import socket    # 创建 socket 对象
    serversocket = socket.socket(
        socket.AF_INET, socket.SOCK_STREAM)
    serversocket.setsockopt(socket.SOL_SOCKET,socket.SO_KEEPALIVE,1)
    serversocket.setsockopt(socket.IPPROTO_TCP,socket.TCP_KEEPIDLE,600)

    # 获取本地主机名
    host = socket.gethostname()

    port = 12345

    # 绑定端口号
    serversocket.bind(("192.168.1.7", port))

    # 设置最大连接数，超过后排队
    serversocket.listen(5)
    recv_data = 0
    counter = 0

    if last_circle_bytes != 0:
        circle_times += 1
    clientsocket, addr = serversocket.accept()
    print(clientsocket)
    print("circle time %d "%circle_times)

    #clientsocket.send("helloworld".encode('utf-8'))
    bmp_prefix=1
    md51=set()
    md52=set()
    md53=set()

    total_len=0
    first_run= True
    same_array = 0
    server_circle = 0
    step = 3

    while True:

        filename = "./ov2640/"+str(bmp_prefix)+'.bmp'
        time_start = time.time()
        for start in range(0, circle_times, step):
            end = start + step
            if end >= circle_times:
                end = circle_times
            counter_set.clear()
            #print("---start %d,%d"%(start,end))
            receive_period(clientsocket, counter_set,start, end)
            #print("---end %d,%d"%(start,end))
        time_end=time.time()
        print(time_end-time_start)

        Rgb565ConvertBmp(buf, 640, 480, filename)

        print(filename, " right", str(len(buf)))
            #time.sleep(10000)
        print("end----------------------------")
        buf = [0]*total_size
        counter = 0
        per_mss = pure_data
        counter_set.clear()
        bmp_prefix += 1
        #clientsocket.send("666".encode('utf-8'))




