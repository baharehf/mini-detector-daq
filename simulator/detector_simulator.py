import socket
import struct
import time

HOST = "127.0.0.1"
PORT = 5005

WIDTH = 64
HEIGHT = 64
PACKET_PIXELS = 512

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

for frame_id in range(10):
    # Create a simulated 64x64 detector image.
    pixels = [
        (x + y + frame_id) % 65536
        for y in range(HEIGHT)
        for x in range(WIDTH)
    ]

    # 16-bit unsigned pixels, little-endian.
    image_data = struct.pack(
        f"<{len(pixels)}H", *pixels
    )

    packet_size = PACKET_PIXELS * 2
    chunks = [
        image_data[i:i + packet_size]
        for i in range(0, len(image_data), packet_size)
    ]


    for packet_id, chunk in enumerate(chunks):

        # Simulate packet loss:
        # Drop packet index 3 (the fourth packet)
        # from frame 1 only.
#        if frame_id == 1 and packet_id == 3:
#            print("SIMULATED LOSS: Frame 1, packet 4")
#            continue

        header = struct.pack(
            "!IHH", frame_id, packet_id, len(chunks)
        )

        sock.sendto(header + chunk, (HOST, PORT))

    print(f"Finished sending frame {frame_id}")
    time.sleep(0.01)

sock.close()