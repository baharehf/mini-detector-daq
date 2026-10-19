# Mini X-ray Detector Data Acquisition System

**A simulated X-ray detector data acquisition pipeline developed using Python, C++17, and UDP networking.**

This project explores fundamental concepts in scientific detector control and data acquisition, including binary data transmission, packet-based communication, image reconstruction, data integrity, visualization, and performance measurement.

The system runs entirely on a local computer without requiring physical detector hardware.

## 1. Project Overview

Modern scientific imaging detectors can generate large volumes of measurement data. Data acquisition software must receive this data, identify individual measurements, reconstruct complete images, and detect transmission problems.

This educational project implements a simplified version of that workflow.

A Python application simulates an X-ray detector by generating synthetic image frames and transmitting them as UDP packets. A C++ receiver collects the packets, reconstructs complete frames, validates packet completeness, and saves the resulting images as binary files.

A separate Python application visualizes the reconstructed detector images.

### Key Features

- Synthetic 64 × 64 pixel detector image generation
- 16-bit unsigned pixel representation
- UDP communication over localhost
- Binary packet headers containing frame and packet identifiers
- Frame reconstruction from multiple network packets
- Duplicate-packet and incomplete-frame detection
- Binary image storage
- Image visualization using NumPy and Matplotlib
- Acquisition performance measurements (FPS and MB/s)
- Simulated packet-loss experiments

## 2. System Architecture

```text
              PYTHON DETECTOR SIMULATOR
              -------------------------
              Generate synthetic images
              64 x 64 pixels, 16-bit
                         |
                         v
                 Packetization
               8 packets per frame
                         |
                         v
                 UDP Transmission
                 127.0.0.1:5005
                         |
                         v
                  C++17 RECEIVER
                  --------------
                 Receive UDP packets
                         |
                         v
                  Parse packet header
                         |
                         v
                Identify frame and index
                         |
                         v
                  Reassemble image
                         |
                  +------+------+
                  |             |
             Complete       Incomplete
                  |             |
                  v             v
             Save .raw     Report missing
                  |          packets
                  v
              PYTHON VISUALIZER
                  |
                  v
              Detector image
```

The implementation uses a single computer, but the design introduces concepts applicable to distributed scientific instrumentation.

## 3. Technologies

| Technology | Purpose |
|---|---|
| C++17 | Data acquisition, packet parsing, image reconstruction |
| Python 3 | Detector simulation and image visualization |
| UDP sockets | Transmission of binary detector data |
| Linux / WSL2 | Development and runtime environment |
| NumPy | Loading and reshaping binary image data |
| Matplotlib | Image visualization |
| Git / GitHub | Version control and project documentation |
| GNU g++ | C++ compilation |

## 4. Detector Data Format

Each simulated frame contains:

| Parameter | Value |
|---|---|
| Image dimensions | 64 × 64 pixels |
| Number of pixels | 4,096 |
| Pixel representation | Unsigned 16-bit integer |
| Image size | 8,192 bytes |
| Packet payload | 1,024 bytes |
| Packets per frame | 8 |
| Application packet header | 8 bytes |

### Packet Structure

Each UDP datagram contains a custom binary header followed by image data.

```text
+----------------------------+
| Frame ID        (4 bytes)  |
+----------------------------+
| Packet Index    (2 bytes)  |
+----------------------------+
| Total Packets   (2 bytes)  |
+----------------------------+
| Pixel Payload (1024 bytes) |
+----------------------------+
```

The packet header is encoded in network byte order.

The image pixel data is encoded as little-endian 16-bit unsigned integers.

Packet identifiers allow the receiver to place incoming data in the correct order during frame reconstruction.

## 5. Project Structure

```text
mini-detector-daq/
|
|-- simulator/
|   |-- detector_simulator.py
|
|-- receiver/
|   |-- receiver.cpp
|
|-- visualization/
|   |-- show_image.py
|
|-- .gitignore
|-- README.md
```

The receiver generates binary image files named `frame_0.raw`, `frame_1.raw`, and so on.

These generated files are excluded from version control.

## 6. Installation

### Requirements

- Ubuntu Linux or Ubuntu under Windows Subsystem for Linux (WSL2)
- Python 3
- g++ with C++17 support
- NumPy
- Matplotlib

### Install Dependencies

On Ubuntu:

```bash
sudo apt update
sudo apt install -y \
    python3 \
    g++ \
    python3-numpy \
    python3-matplotlib \
    git
```

### Clone the Repository

```bash
git clone https://github.com/YOUR_USERNAME/mini-detector-daq.git
cd mini-detector-daq
```

Replace `YOUR_USERNAME` with the repository owner's GitHub username.

## 7. Build and Run

### Step 1: Compile the C++ Receiver

From the project root:

```bash
g++ -std=c++17 -O2 -Wall -Wextra \
    receiver/receiver.cpp -o receiver_app
```

### Step 2: Start the Receiver

Open Terminal 1:

```bash
./receiver_app
```

The receiver listens for UDP packets on port `5005`.

Expected initial output:

```text
Waiting for detector frames...
```

### Step 3: Run the Detector Simulator

Open Terminal 2:

```bash
python3 simulator/detector_simulator.py
```

The Python application generates and transmits synthetic detector images.

### Step 4: Inspect the Output

For a successful 10-frame acquisition:

```text
Saved frame_0.raw (8192 bytes)
...
Saved frame_9.raw (8192 bytes)

Complete frames saved: 10/10
```

The generated binary frames are saved in the current working directory.

**Note:** The sender and receiver must be configured for the same number of frames. The current implementation uses a compile-time frame count in C++.

## 8. Image Visualization

Run:

```bash
python3 visualization/show_image.py
```

The script reads `frame_0.raw`, interprets its contents as little-endian unsigned 16-bit values, and reshapes the data into a 64 × 64 image.

The visualization uses Matplotlib's `viridis` colormap.

### Synthetic Image Generation

Pixel values are generated using:

```python
pixel_value = (x + y + frame_id) % 65536
```

This creates a smooth intensity gradient.

The output is synthetic test data, not a real X-ray measurement.

## 9. Packet-Loss Detection

Reliable scientific acquisition requires identifying incomplete measurements.

The receiver tracks received packet indices for each frame and reconstructs the image only when all expected packets have arrived.

A controlled packet-loss experiment was implemented by intentionally skipping one packet from Frame 1.

### Experiment

```python
if frame_id == 1 and packet_id == 3:
    continue
```

The receiver detected the incomplete frame:

```text
Incomplete frame 1: received 7/8 packets
Complete frames saved: 2/3
```

The incomplete frame was not saved as a valid image.

This experiment demonstrates application-level data integrity checking over UDP.

## 10. Performance Measurements

The receiver measures:

- Elapsed acquisition time
- Complete frames per second (FPS)
- Completed image throughput (MB/s)
- Number of successfully reconstructed frames

### Experimental Results

A 10-frame localhost test produced:

| Metric | Measured result |
|---|---:|
| Complete frames | 10/10 |
| Elapsed acquisition time | 0.0985 seconds |
| Average frame rate | 101.55 FPS |
| Completed image throughput | 0.832 MB/s |
| Frame size | 8,192 bytes |
| Simulated inter-frame delay | 10 ms |

### Benchmark Conditions

- Environment: Ubuntu under WSL2
- Communication: UDP over localhost
- Image size: 64 × 64 pixels
- Pixel format: 16-bit unsigned integer
- Receiver: C++17
- Simulator: Python 3
- Acquisition includes per-packet logging and per-frame file writing

**Important:** These measurements describe one small-scale local experiment. They are not maximum network throughput or optimized detector acquisition performance.

The timing begins when the receiver observes its first valid packet and ends after the acquisition loop completes.

For meaningful high-throughput benchmarking, future tests should use larger frame counts, repeated measurements, reduced logging, and controlled sending rates.

## 11. Current Limitations

This is an educational proof of concept, not a production-ready detector acquisition system.

Current limitations include:

- Fixed image dimensions and packet payload size
- Configured number of expected frames
- Single local UDP receiver
- No detector hardware integration
- No hardware timestamps or synchronization
- No general packet retransmission mechanism
- Limited handling of delayed packets after frame completion
- Individual binary output files instead of a scientific data container
- Performance measurements affected by simulator pacing, logging, and disk I/O

The receiver detects incomplete frames but does not recover missing payload data.

## 12. Planned Improvements

Future development will focus on:

- [ ] Configurable acquisition settings through command-line arguments
- [ ] Larger-scale benchmarking with 100–1,000+ frames
- [ ] Reduced logging during performance tests
- [ ] Improved packet-loss and duplicate-packet statistics
- [ ] Explicit reporting of missing packet indices
- [ ] Better timeout and socket-error handling
- [ ] Automated correctness and regression tests
- [ ] Multithreaded packet reception and processing
- [ ] Producer-consumer queues and buffer management
- [ ] Improved memory allocation efficiency
- [ ] HDF5-based scientific image storage
- [ ] Simulated multi-detector acquisition and control interfaces

## 13. Motivation and Learning Objectives

This project was developed as a practical exercise in embedded and systems software engineering, with a particular interest in high-speed scientific detector data acquisition.

It was motivated by the engineering challenges involved in modern experimental research facilities, including DESY's PETRA IV project.

The main learning objectives are:

1. Strengthen C++ and Python programming skills.
2. Understand Linux socket programming and UDP transmission.
3. Gain practical experience with binary data handling.
4. Implement frame reconstruction and data integrity checks.
5. Investigate performance measurement and optimization.
6. Develop experience documenting and testing scientific software.

The project is independently developed and is not affiliated with or endorsed by DESY.

## 14. References

- [DESY PETRA IV](https://desy.de/microsites/petra-4/content/index_eng.html)
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)
- [C++ Reference](https://en.cppreference.com/w/)
- [Python Documentation](https://docs.python.org/3/)
- [NumPy Documentation](https://numpy.org/doc/)
- [Matplotlib Documentation](https://matplotlib.org/stable/)

---

**Project status:** Educational prototype — ongoing development.

**Development environment:** Windows + WSL2 Ubuntu.

**Primary focus:** Scientific detector simulation, UDP data acquisition, frame reconstruction, data integrity, and performance measurement.
